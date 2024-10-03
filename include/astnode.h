#pragma once

#include <vector>
#include <string>

#include <nlohmann/json.hpp>

#include "token.h"
#include "nodetype.h"

using std::vector;
using std::string;

class ASTNode {
public:
  NodeType type;
  vector<ASTNode> children;
  nlohmann::json data;
  // TODO: could stick metadata directly into data json?
  TokenMetadata metadata;

  static ASTNode makeTopLevel (vector<ASTNode> statements,
                               TokenMetadata metadata);
  static ASTNode makeFunctionDeclare (string name, vector<string> args,
                                      ASTNode body, TokenMetadata metadata);
  static ASTNode makeVarDeclare(string name, ASTNode rhs, 
                                bool is_const, TokenMetadata metadata);
  static ASTNode makeModuleImport(string module_path, TokenMetadata metadata);
  static ASTNode makeModuleImport(string module_path, string module_name, TokenMetadata metadata);
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
  static ASTNode makeDictLiteral (vector<ASTNode> kv_pairs,
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
  static ASTNode makeNothingLiteral(TokenMetadata metadata);
  static ASTNode nothing ();
};

void to_json(nlohmann::json &j, const ASTNode &node);

void from_json(const nlohmann::json &j, ASTNode &node);