#pragma once

#include <vector>

#include <nlohmann/json.hpp>

#include "token.h"
#include "tokentype.h"
#include "astnode.h"

using std::string;
using std::vector;

class ParserState {
public:
  vector<Token> tokens;
  int index;

  ParserState (vector<Token> tokens_, int index_ = 0)
      : tokens{ std::move (tokens_) }, index{ index_ } {}

  bool hasNext ();
  Token currentToken ();
  Token advance ();

  /* If current_token.type == t, return the the Token and advance */
  Token expect (TokenType t);
  bool currentTokenIs (TokenType t);
  bool currentTokenIsNot (TokenType t);
  bool matchTokenType (TokenType t);

  // Token peekToken(int n);
  // Token advance();
  // Token matchKeyword(std::string kwrd);
  // Token matchTokenType(TokenType ttype);
  // Token matchSymbol(std::string smbl);
  // Token matchLiteral();
  void error (string msg);
  void warn (string msg);
};

ASTNode parse_tokens (vector<Token> tokens);
