#include <iostream>
#include <sstream>
#include <string>

#include "parser.h"
#include "token.h"
#include "tokentype.h"

using std::string;

bool ParserState::hasNext() { return this->index < this->tokens.size(); }

Token ParserState::currentToken() { return this->tokens.at(this->index); }

Token ParserState::advance() {
  auto current_token = this->currentToken();
  this->index++;
  return current_token;
}

bool ParserState::currentTokenIs(TokenType t) {
  return this->currentToken().type == t;
}

bool ParserState::currentTokenIsNot(TokenType t) {
  return this->currentToken().type != t;
}

bool ParserState::matchTokenType(TokenType t) {
  auto match = this->currentTokenIs(t);
  if (match) {
    this->advance();
  }
  return match;
}

Token ParserState::expect(TokenType t) {
  /* If current_token.type == t, return the the Token and advance */
  auto current_token = this->currentToken();
  if (current_token.type != t) {

    /* make error message string */
    std::stringstream msg;
    auto expected_tt = token_type_to_string(t);
    auto encountered_tt = token_type_to_string(current_token.type);
    msg << "Expected TokenType \"" << expected_tt << "\", encountered \""
        << encountered_tt << "\"";

    // implicit early return
    this->error(msg.str());
  }
  this->index++;
  return current_token;
}

void ParserState::warn(string msg) {
  auto metadata = this->currentToken().metadata;
  std::cerr << "Error encountered at line: " << metadata.line
            << ", column: " << metadata.column << "\n";

  std::cerr << msg << "\n";
}

void ParserState::error(string msg) {
  this->warn(msg);
  exit(1);
}
