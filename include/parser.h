#pragma once

#include <vector>

#include <nlohmann/json.hpp>

#include "astnode.h"
#include "token.h"
#include "tokentype.h"

using std::string;
using std::vector;

class ParserState {
public:
  vector<Token> tokens;
  int index;

  ParserState(vector<Token> tokens_, int index_ = 0)
      : tokens{std::move(tokens_)}, index{index_} {}

  bool hasNext();
  Token currentToken();
  Token advance();

  Token expect(TokenType t);
  bool currentTokenIs(TokenType t);
  bool currentTokenIsNot(TokenType t);
  bool matchTokenType(TokenType t);

  void error(string msg);
  void warn(string msg);
};

ASTNode parse_tokens(vector<Token> tokens);
