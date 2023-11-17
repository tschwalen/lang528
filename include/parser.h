#pragma once

#include <vector>

#include <nlohmann/json.hpp>

#include "token.h"

using std::string;
using std::vector;

enum class NodeType {
  TOP_LEVEL,
  BLOCK,
  ASSIGN_OP,
  VAR_DECLARE,
  FUNC_DECLARE,
  IF, // first child -> body, second child -> else (if present)
  RETURN,
  WHILE,
  BINARY_OP, // e.g. x + y, 5 * 5, etc.
  UNARY_OP,  // e.g. !x, -5, etc.
  FUNC_CALL,
  INDEX_ACCESS, // e.g. "sub" X[Y]
  FIELD_ACESS,  // e.g. object.field
  VAR_LOOKUP,
  EXPR_LIST,

  VEC_LITERAL, // e.g. brackets, which can contain sub-expressions
  BOOL_LITERAL,
  INT_LITERAL,
  FLOAT_LITERAL,
  STRING_LITERAL,

  // DELETE IF UNUSED
  BASIC_LITERAL, // numbers, strings, true/false, etc.

};

class ASTNode {
public:
  NodeType type;
  vector<ASTNode> children;
  nlohmann::json data;
  // TODO: could stick metadata directly into data json?
  TokenMetadata metadata;

  // factory methods TODO: implement
  static ASTNode makeTopLevel (vector<ASTNode> statements,
                               TokenMetadata metadata);
  static ASTNode makeFunctionDeclare (string name, vector<string> args,
                                      ASTNode body, TokenMetadata metadata);
  static ASTNode makeLetDeclare (string name, ASTNode rhs,
                                 TokenMetadata metadata);
  static ASTNode makeConstDeclare (string name, ASTNode rhs,
                                   TokenMetadata metadata);
  static ASTNode makeBlock (vector<ASTNode> statements,
                            TokenMetadata metadata);
  static ASTNode makeWhile (ASTNode condition, ASTNode body,
                            TokenMetadata metadata);
  static ASTNode makeIf (ASTNode condition, ASTNode body,
                         TokenMetadata metadata);
  static ASTNode makeIfElse (ASTNode condition, ASTNode body,
                             ASTNode else_body, TokenMetadata metadata);
  static ASTNode makeVectorLiteral (vector<ASTNode> elements,
                                    TokenMetadata metadata);
  static ASTNode makeVarLookup (string identifier, TokenMetadata metadata);
  static ASTNode makeFunctionCall (ASTNode lvalue_expr, ASTNode arg_expr_list,
                                   TokenMetadata metadata);
  static ASTNode makeExprList (vector<ASTNode> arg_exprs,
                               TokenMetadata metadata);
  static ASTNode makeIndexAccess (ASTNode lvalue_expr, ASTNode index_expr,
                                  TokenMetadata metadata);
  static ASTNode makeFieldAccess (ASTNode lvalue_expr, ASTNode field_expr,
                                  TokenMetadata metadata);
  static ASTNode makeBinaryOp (TokenType op, ASTNode lhs_expr,
                               ASTNode rhs_expr, TokenMetadata metadata);
  static ASTNode makeUnaryOp (TokenType op, ASTNode expr,
                              TokenMetadata metadata);
  static ASTNode makeAssignOp (TokenType op, ASTNode lhs_expr,
                               ASTNode rhs_expr, TokenMetadata metadata);
  static ASTNode makeReturn (ASTNode value, TokenMetadata metadata);
  static ASTNode makeLiteral (string value, TokenMetadata metadata);
  static ASTNode makeLiteral (int value, TokenMetadata metadata);
  static ASTNode makeLiteral (float value, TokenMetadata metadata);
  static ASTNode makeLiteral (bool value, TokenMetadata metadata);
  static ASTNode nothing ();
};

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
  bool advanceIfCurrentTokenIs (TokenType t);

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
