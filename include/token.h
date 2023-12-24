#pragma once

#include <string>
#include <variant>

#include "tokentype.h"

#include <nlohmann/json.hpp>

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

bool is_unary_op(TokenType tt);

int unary_op_precedence(TokenType tt);

bool binary_precedence_test(TokenType op, TokenType lookahead);

bool unary_precedence_test(TokenType op, TokenType lookahead);

bool is_assign_op(TokenType tt );

TokenType assign_op_to_binary_op(TokenType tt);

void to_json(nlohmann::json &j, const TokenMetadata &tm);

void from_json(const nlohmann::json &j, TokenMetadata &tm);

void to_json(nlohmann::json &j, const Token &t);

void from_json(const nlohmann::json &j, Token &t);