#include <string>
#include <variant>
#include <vector>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include "token.h"
#include "tokentype.h"

using json = nlohmann::json;

using std::string;

//////////////////////////////////////////////////////////////////
// json conversion methods
//////////////////////////////////////////////////////////////////

void to_json(json& j, const TokenMetadata& tm) {
  j = json{
      {"line", tm.line},
      {"column", tm.column},
  };
}

void from_json(const json& j, TokenMetadata& tm) {
  j.at("line").get_to(tm.line);
  j.at("column").get_to(tm.column);
}

void to_json(json& j, const Token& t) {
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
    case 0:  // monostate
      j["value"] = nullptr;
      break;
    case 1:  // int
      j["value"] = std::get<int>(value_variant);
      break;
    case 2:  // float
      j["value"] = std::get<float>(value_variant);
      break;
    case 3:  // string
      j["value"] = std::get<string>(value_variant);
      break;
    case 4:  // bool
      j["value"] = std::get<bool>(value_variant);
      break;
  }
}

void from_json(const json& j, Token& t) {
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

//////////////////////////////////////////////////////////////////
// END OF json conversion methods
//////////////////////////////////////////////////////////////////

/*
  Returns true if the passed token type is a binary operator
*/
bool is_binary_op(TokenType tt) {
  return (
      tt == TokenType::GREATER_EQUALS || tt == TokenType::EQUALS_EQUALS ||
      tt == TokenType::LESS_EQUALS || tt == TokenType::NOT_EQUALS ||
      tt == TokenType::GREATER || tt == TokenType::MINUS ||
      tt == TokenType::TIMES || tt == TokenType::LESS ||
      tt == TokenType::PLUS || tt == TokenType::DIV || tt == TokenType::MOD ||
      tt == TokenType::AND ||
      tt == TokenType::OR

      // not infix 'operators' in the traditional sense, but still binary ops
      || tt == TokenType::LBRACKET  // "[]", index/subscript access
      || tt == TokenType::LPAREN    // "()", function call
      || tt == TokenType::DOT       // ".field", struct/class access
  );
}

/*
  Returns true if the passed token type is a right-associative unary operator
*/
bool is_unary_op(TokenType tt) {
  return tt == TokenType::MINUS || tt == TokenType::NOT;
}

bool is_assign_op(TokenType tt) {
  return tt == TokenType::EQUALS || tt == TokenType::PLUS_EQUALS ||
         tt == TokenType::MINUS_EQUALS || tt == TokenType::TIMES_EQUALS ||
         tt == TokenType::DIV_EQUALS || tt == TokenType::MOD_EQUALS;
}

TokenType assign_op_to_binary_op(TokenType tt) {
  switch (tt) {
    case TokenType::PLUS_EQUALS:
      return TokenType::PLUS;
    case TokenType::MINUS_EQUALS:
      return TokenType::MINUS;
    case TokenType::TIMES_EQUALS:
      return TokenType::TIMES;
    case TokenType::DIV_EQUALS:
      return TokenType::DIV;
    case TokenType::MOD_EQUALS:
      return TokenType::MOD;
    default: break;
  }
  
  throw std::runtime_error("argument must be an assign op");
}

bool binary_precedence_test(TokenType op, TokenType lookahead) {
  return is_binary_op(lookahead) &&
         (binary_op_precedence(lookahead) > binary_op_precedence(op));
}

bool unary_precedence_test(TokenType op, TokenType lookahead) {
  return is_unary_op(lookahead) &&
         (unary_op_precedence(lookahead) == unary_op_precedence(op));
}

// TODO: replace these magic numbers with a better system for
//        setting op precedence in one place
// TODO2: consider if unary not needs a different precendence.
int unary_op_precedence(TokenType tt) {
  return is_unary_op(tt) ? 11 : -1;
}

int binary_op_precedence(TokenType tt) {
  switch (tt) {
    case TokenType::LBRACKET:
    case TokenType::LPAREN:
    case TokenType::DOT:
      return 12;

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

    default:
      break;
  }
  return -1;
}
