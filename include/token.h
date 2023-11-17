#pragma once

#include <string>
#include <variant>

#include <nlohmann/json.hpp>

enum class TokenType {
  // keywords
  FUNCTION,
  LET,
  CONST,
  IF,
  ELSEIF,
  ELSE,
  WHILE,
  RETURN,
  // end of keywords
  IDENTIFIER,
  // symbols and operators
  DOT,            // .
  DOT_DOT,        // ..
  COMMA,          // ,
  EQUALS,         // =
  EQUALS_EQUALS,  // ==
  NOT,            // !
  NOT_EQUALS,     // !=
  LESS,           // <
  LESS_EQUALS,    // <=
  GREATER,        // >
  GREATER_EQUALS, // >=
  PLUS,           // +
  PLUS_EQUALS,    // +=
  AND,            // &
  OR,             // |
  MINUS,          // -
  MINUS_EQUALS,   // -=
  TIMES,          // *
  TIMES_EQUALS,   // *=
  DIV,            // /
  DIV_EQUALS,     // /=
  MOD,            // %
  MOD_EQUALS,     // %=
  LPAREN,         // (
  RPAREN,         // )
  LBRACKET,       // [
  RBRACKET,       // ]
  LBRACE,         // {
  RBRACE,         // }
  SEMICOLON,      // ;
  // literals
  BOOL_LITERAL,
  INT_LITERAL,
  FLOAT_LITERAL,
  STRING_LITERAL,
  // not sure if needed
  END_OF_FILE,
  // satisfies compiler when we throw exceptions
  NULL_TOKEN,
};

struct TokenMetadata {
  int line;
  int column;
};

struct Token {
  TokenType type; 
  std::variant<std::monostate, int, float, std::string, bool> value;
  TokenMetadata metadata;
};

bool is_binary_op(TokenType tt);

int binary_op_precedence(TokenType tt);

bool is_right_assoc_op(TokenType tt);

int unary_op_precedence(TokenType tt);

bool binary_precedence_test(TokenType op, TokenType lookahead);

bool unary_precedence_test(TokenType op, TokenType lookahead);

bool is_assign_op(TokenType tt );

std::string token_type_to_string(TokenType tt);

void to_json(nlohmann::json &j, const TokenMetadata &tm);

void from_json(const nlohmann::json &j, TokenMetadata &tm);

void to_json(nlohmann::json &j, const Token &t);

void from_json(const nlohmann::json &j, Token &t);