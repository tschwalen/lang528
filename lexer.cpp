#include <algorithm>
#include <cctype>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "lexer.h"
#include "token.h"

using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

bool is_id_start_char(char c) { return isalpha(c) || c == '_' || c == '$'; }

bool is_id_char(char c) {
  return isdigit(c) || isalpha(c) || c == '_' || c == '$';
}

bool symbol_pair_comparator(const pair<string, TokenType> &a,
                            const pair<string, TokenType> &b) {
  // sort on string length descending
  return a.first.size() > b.first.size();
}

unordered_map<string, TokenType> KEYWORDS{{"function", TokenType::FUNCTION},
                                          {"let", TokenType::LET},
                                          {"const", TokenType::CONST},
                                          {"if", TokenType::IF},
                                          {"while", TokenType::WHILE}};

unordered_map<char, vector<pair<string, TokenType>>> SYMBOLS{
    {'.', {{"..", TokenType::DOT_DOT}, {".", TokenType::DOT}}},
    {'=', {{"=", TokenType::EQUALS}, {"==", TokenType::EQUALS_EQUALS}}},
    {'!', {{"!", TokenType::NOT}, {"!=", TokenType::NOT_EQUALS}}},
    {'<', {{"<", TokenType::LESS}, {"<=", TokenType::LESS_EQUALS}}},
    {'>', {{">", TokenType::GREATER}, {">=", TokenType::GREATER_EQUALS}}},
    {'+', {{"+", TokenType::PLUS}, {"+=", TokenType::PLUS_EQUALS}}},
    {',', {{",", TokenType::COMMA}}},
    {'&', {{"&", TokenType::AND}}},
    {'|', {{"|", TokenType::OR}}},
    {'-', {{"-", TokenType::MINUS}, {"-=", TokenType::MINUS_EQUALS}}},
    {'*', {{"*", TokenType::TIMES}, {"*=", TokenType::TIMES_EQUALS}}},
    {'/', {{"/", TokenType::DIV}, {"/=", TokenType::DIV_EQUALS}}},
    {'%', {{"%", TokenType::MOD}, {"%=", TokenType::MOD_EQUALS}}},
    {'(', {{"(", TokenType::LPAREN}}},
    {')', {{")", TokenType::RPAREN}}},
    {'[', {{"[", TokenType::LBRACKET}}},
    {']', {{"]", TokenType::RBRACKET}}},
    {'{', {{"{", TokenType::LBRACE}}},
    {'}', {{"}", TokenType::RBRACE}}},
    {';', {{";", TokenType::SEMICOLON}}},
};

/////////////////////////////////////////////////////////////////
// LexerState instance methods
//////////////////////////////////////////////////////////////////

TokenMetadata LexerState::current_metadata() {
  return TokenMetadata{this->line, this->column};
}

string LexerState::report_metadata() {
  // probably compeltely superfluous now that we have the json hooked up
  std::stringstream message;
  message << "{ line: " << this->line << ", column: " << this->column << " }";
  return message.str();
}

bool LexerState::has_next() { return this->index < this->source.size(); }

char LexerState::current_char() { return this->source.at(this->index); }

char LexerState::next_char() {
  // todo: bound check here
  this->advance();
  return this->current_char();
}

void LexerState::advance() {
  // maybe newline check here
  this->index++;
  this->column++;
}

void LexerState::advance(int n) {
  // maybe newline check here
  this->index += n;
  this->column += n;
}

void LexerState::handle_newline() {
  this->line++;
  this->index++;
  this->column = 0;
}

void LexerState::handle_whitespace() {
  if (this->current_char() == '\n') {
    this->handle_newline();
  } else {
    this->advance();
  }
}

void LexerState::handle_comment() {
  // just single-line right now
  while (this->current_char() != '\n') {
    this->index++;
    this->column++;
  }
}

string LexerState::lookahead(int n) {
  //        index=8
  //          v
  // "current input "
  //          01234
  // lookahead(5) = "input"
  return this->source.substr(this->index, n);
}

Token LexerState::handle_wordlike() {
  // capture the line number and index of first character
  TokenMetadata metadata = this->current_metadata();
  string word;

  auto c = this->current_char();
  while (is_id_char(c)) {
    // appending char requires 'n' for number of times to append
    word.append(1, c);
    c = this->next_char();
  }

  // check if it's a keyword, return correct keyword token if so
  if (KEYWORDS.count(word) > 0) {
    auto keyword_token_type = KEYWORDS.at(word);
    return Token{keyword_token_type, {}, metadata};
  }

  // check if it's a bool literal
  if (word == "true" || word == "false") {
    auto bool_value = word == "true"; // bit of a hack, but works
    return Token{TokenType::BOOL_LITERAL, bool_value, metadata};
  }

  // otherwise, it's an identifier
  return Token{TokenType::IDENTIFIER, word, metadata};
}

Token LexerState::handle_numeric() {
  TokenMetadata metadata = this->current_metadata();
  string number;

  auto c = this->current_char();
  while (isdigit(c)) {
    number.append(1, c);
    c = this->next_char();
  }

  // read floating point if '.' is encountered
  if (c == '.') {
    c = this->next_char();
    while (isdigit(c)) {
      number.append(1, c);
      c = this->next_char();
    }
    auto value = std::stof(number);
    return Token{TokenType::FLOAT_LITERAL, value, metadata};
  }

  // otherwise, is int
  auto value = std::stoi(number);
  return Token{TokenType::INT_LITERAL, value, metadata};
}

Token LexerState::handle_string() {
  TokenMetadata metadata = this->current_metadata();
  auto quote = this->current_char();
  string string_contents;

  // build the string until a closing quote is encountered
  // TODO: handle escape characters
  auto curr = this->next_char();
  while (curr != quote) {
    string_contents.append(1, curr);
    curr = this->next_char();
  }

  // advance past the closing quote
  this->advance();

  return Token{TokenType::STRING_LITERAL, string_contents, metadata};
}

Token LexerState::handle_symbols() {
  // handle single and multichar symbols/operators
  TokenMetadata metadata = this->current_metadata();
  auto curr = this->current_char();
  string symbol{curr};

  // could sort all these once before main program runs
  auto candidates = SYMBOLS.at(curr);
  std::sort(candidates.begin(), candidates.end(), symbol_pair_comparator);

  // now that candidates is sorted with the longest candidate first,
  // pick the first that matches
  for (auto &candidate : candidates) {
    auto candidate_string = candidate.first;
    auto length = candidate_string.size();

    auto string_at_current_postition = this->lookahead(length);

    // if we match, then we're done
    if (string_at_current_postition == candidate_string) {
      auto t = Token{candidate.second, {}, metadata};
      this->advance(length);
      return t;
    }
  }

  // TODO: error if we've reached this point
  return Token{TokenType::NULL_TOKEN, {}, metadata};
}

/////////////////////////////////////////////////////////////////
// END OF LexerState instance methods
//////////////////////////////////////////////////////////////////

vector<Token> lex_string(string source) {
  vector<Token> result;
  /// this constructor call is fucked up for some reason
  LexerState lex_state{source};

  while (lex_state.has_next()) {
    auto curr = lex_state.current_char();

    if (isspace(curr)) {
      lex_state.handle_whitespace();
    } else if (curr == '#') {
      lex_state.handle_comment();
    } else if (is_id_start_char(curr)) {
      auto t = lex_state.handle_wordlike();
      result.push_back(t);
    } else if (isdigit(curr)) {
      auto t = lex_state.handle_numeric();
      result.push_back(t);
    } else if (curr == '"' || curr == '\'') {
      auto t = lex_state.handle_string();
      result.push_back(t);
    } else if (SYMBOLS.count(curr) > 0) {
      auto t = lex_state.handle_symbols();
      result.push_back(t);
    } else {
      std::cout << "Invalid character for start of token: '" << curr << "'\n\n"
                << "Encountered at: " << lex_state.report_metadata() << "\n";
      break;
    }
  }
  return result;
}