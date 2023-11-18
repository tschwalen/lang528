#pragma once

#include <string>
#include <vector>

#include "token.h"
#include "tokentype.h"

bool is_id_start_char(char c);

bool is_id_char(char c);

bool symbol_pair_comparator(const std::pair<std::string, TokenType> &a,
                            const std::pair<std::string, TokenType> &b);

class LexerState {
public:
  int line;
  int column;
  int index;
  std::string source;

  LexerState(std::string source) {
    line = 0;
    column = 0;
    index = 0;
    this->source = source;
  }

  TokenMetadata current_metadata();

  std::string report_metadata();

  bool has_next();

  char current_char();

  char next_char();

  void advance();

  void advance(int n);

  void handle_newline();

  void handle_whitespace();

  void handle_comment();

  std::string lookahead(int n);

  Token handle_wordlike();

  Token handle_numeric();

  Token handle_string();

  Token handle_symbols();
};

std::vector<Token> lex_string(std::string source);
