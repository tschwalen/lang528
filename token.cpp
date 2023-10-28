#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

#include "token.h"

using json = nlohmann::json;

using std::string;
using std::vector;

// stupid methods I have to write because enum classes aren't that good

string token_type_to_string(TokenType tt) {
  vector<string> entries{"FUNCTION",
                         "LET",
                         "CONST",
                         "IF",
                         "ELSEIF",
                         "ELSE",
                         "WHILE",
                         "RETURN",
                         "IDENTIFIER",
                         "DOT",
                         "DOT_DOT",
                         "EQUALS",
                         "EQUALS_EQUALS",
                         "NOT",
                         "NOT_EQUALS",
                         "LESS",
                         "LESS_EQUALS",
                         "GREATER",
                         "GREATER_EQUALS",
                         "PLUS",
                         "PLUS_EQUALS",
                         "AND",
                         "OR",
                         "MINUS",
                         "MINUS_EQUALS",
                         "TIMES",
                         "TIMES_EQUALS",
                         "DIV",
                         "DIV_EQUALS",
                         "MOD",
                         "MOD_EQUALS",
                         "LPAREN",
                         "RPAREN",
                         "LBRACKET",
                         "RBRACKET",
                         "LBRACE",
                         "RBRACE",
                         "SEMICOLON",
                         "BOOL_LITERAL",
                         "INT_LITERAL",
                         "FLOAT_LITERAL",
                         "STRING_LITERAL",
                         "END_OF_FILE",
                         "NULL_TOKEN"};
  return entries.at((int)tt);
}

TokenType int_to_token_type(int i) {
  vector<TokenType> entries{TokenType::FUNCTION,
                            TokenType::LET,
                            TokenType::CONST,
                            TokenType::IF,
                            TokenType::ELSEIF,
                            TokenType::ELSE,
                            TokenType::WHILE,
                            TokenType::RETURN,
                            TokenType::IDENTIFIER,
                            TokenType::DOT,
                            TokenType::DOT_DOT,
                            TokenType::EQUALS,
                            TokenType::EQUALS_EQUALS,
                            TokenType::NOT,
                            TokenType::NOT_EQUALS,
                            TokenType::LESS,
                            TokenType::LESS_EQUALS,
                            TokenType::GREATER,
                            TokenType::GREATER_EQUALS,
                            TokenType::PLUS,
                            TokenType::PLUS_EQUALS,
                            TokenType::AND,
                            TokenType::OR,
                            TokenType::MINUS,
                            TokenType::MINUS_EQUALS,
                            TokenType::TIMES,
                            TokenType::TIMES_EQUALS,
                            TokenType::DIV,
                            TokenType::DIV_EQUALS,
                            TokenType::MOD,
                            TokenType::MOD_EQUALS,
                            TokenType::LPAREN,
                            TokenType::RPAREN,
                            TokenType::LBRACKET,
                            TokenType::RBRACKET,
                            TokenType::LBRACE,
                            TokenType::RBRACE,
                            TokenType::SEMICOLON,
                            TokenType::BOOL_LITERAL,
                            TokenType::INT_LITERAL,
                            TokenType::FLOAT_LITERAL,
                            TokenType::STRING_LITERAL,
                            TokenType::END_OF_FILE,
                            TokenType::NULL_TOKEN};
  return entries.at(i);
}

/////////////////////////////////////////////////////////////////
// json conversion methods
//////////////////////////////////////////////////////////////////

void to_json(json &j, const TokenMetadata &tm) {
  j = json{
      {"line", tm.line},
      {"column", tm.column},
  };
}

void from_json(const json &j, TokenMetadata &tm) {
  j.at("line").get_to(tm.line);
  j.at("column").get_to(tm.column);
}

void to_json(json &j, const Token &t) {
  auto type_string = token_type_to_string(t.type);
  auto type_int = (int)t.type;

  j = json{
      {"type_string", type_string},
      {"type_int", type_int},
      {"metadata", t.metadata},
  };

  auto value_variant = t.value;
  // <std::monostate, int, float, std::string, bool>
  switch (value_variant.index()) {
  case 0: // monostate
    j["value"] = nullptr;
    break;
  case 1: // int
    j["value"] = std::get<int>(value_variant);
    break;
  case 2: // float
    j["value"] = std::get<float>(value_variant);
    break;
  case 3: // string
    j["value"] = std::get<string>(value_variant);
    break;
  case 4: // bool
    j["value"] = std::get<bool>(value_variant);
    break;
  }
}

void from_json(const json &j, Token &t) {
  // metadata should be simple
  j.at("metadata").get_to(t.metadata);

  // tokentype needs to be extracted as int and then converted to enum (a pain)
  auto ttype = j.at("type_int");
  assert(ttype.is_number_integer());
  auto ttype_int = ttype.get<int>();
  auto ttype_enum = int_to_token_type(ttype_int);

  t.type = ttype_enum;

  // value will be the worst since it can be several different types
  auto value = j.at("value");
  if (value.is_null()) {
    t.value = {};
  } else if (value.is_number_float()) {
    t.value = value.get<float>();
  } else if (value.is_number_integer()) {
    t.value = value.get<int>();
  } else if (value.is_string()) {
    t.value = value.get<string>();
  } else if (value.is_boolean()) {
    t.value = value.get<bool>();
  }
}

/////////////////////////////////////////////////////////////////
// END OF json conversion methods
//////////////////////////////////////////////////////////////////

/*
  Returns true if the passed token type is a binary operator
*/
bool is_binary_op(TokenType tt) {
  return ( tt == TokenType::GREATER_EQUALS
        || tt == TokenType::EQUALS_EQUALS
        || tt == TokenType::LESS_EQUALS
        || tt == TokenType::NOT_EQUALS
        || tt == TokenType::GREATER
        || tt == TokenType::MINUS
        || tt == TokenType::TIMES
        || tt == TokenType::LESS
        || tt == TokenType::PLUS
        || tt == TokenType::DIV
        || tt == TokenType::MOD
        || tt == TokenType::AND
        || tt == TokenType::OR 

        // not infix operators in the traditional sense, but still binary ops
        || tt == TokenType::LBRACKET // "[]", index
        || tt == TokenType::LPAREN // "()", function call 
        || tt == TokenType::DOT   // ".field", struct/class access
  );
}

int op_precedence(TokenType tt) {
  switch (tt) {
    case TokenType::LBRACKET:
    case TokenType::LPAREN:
    case TokenType::DOT:
      return 10;

    case TokenType::TIMES:
    case TokenType::MOD:
    case TokenType::DIV:
      return 9;

    case TokenType::MINUS:
    case TokenType::PLUS:
      return 8;

    case TokenType::GREATER_EQUALS:
    case TokenType::LESS_EQUALS:
    case TokenType::GREATER:
    case TokenType::LESS:
      return 7;

    case TokenType::EQUALS_EQUALS:
    case TokenType::NOT_EQUALS:
      return 6;

    case TokenType::AND:
      return 5;

    case TokenType::OR:
      return 4;

    default: break;
  }
  return -1;
}