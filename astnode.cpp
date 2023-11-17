#include <nlohmann/json.hpp>
#include <vector>

#include "parser.h"
#include "token.h"

using std::string;
using std::vector;

ASTNode ASTNode::makeTopLevel(vector<ASTNode> statements,
                              TokenMetadata metadata) {
  return ASTNode{NodeType::TOP_LEVEL, statements, {}, metadata};
}

ASTNode ASTNode::makeFunctionDeclare(string name,
                                     vector<string> args,
                                     ASTNode body,
                                     TokenMetadata metadata) {
  return ASTNode{NodeType::FUNC_DECLARE,
                 {body},
                 {{"function_name", name}, {"args", args}},
                 metadata};
}

ASTNode ASTNode::makeLetDeclare(string name,
                                ASTNode rhs,
                                TokenMetadata metadata) {
  return ASTNode{NodeType::VAR_DECLARE, {rhs}, {"const", false}, metadata};
}

ASTNode ASTNode::makeConstDeclare(string name,
                                  ASTNode rhs,
                                  TokenMetadata metadata) {
  return ASTNode{NodeType::VAR_DECLARE, {rhs}, {"const", true}, metadata};
}

ASTNode ASTNode::makeBlock(vector<ASTNode> statements, TokenMetadata metadata) {
  return ASTNode{NodeType::BLOCK, statements, {}, metadata};
}

ASTNode ASTNode::makeWhile(ASTNode condition,
                           ASTNode body,
                           TokenMetadata metadata) {
  return ASTNode{NodeType::WHILE, {condition, body}, {}, metadata};
}

ASTNode ASTNode::makeIf(ASTNode condition,
                        ASTNode body,
                        TokenMetadata metadata) {
  return ASTNode{NodeType::IF, {condition, body}, {}, metadata};
}

ASTNode ASTNode::makeIfElse(ASTNode condition,
                            ASTNode body,
                            ASTNode else_body,
                            TokenMetadata metadata) {
  return ASTNode{NodeType::IF, {condition, body, else_body}, {}, metadata};
}

ASTNode ASTNode::makeVectorLiteral(vector<ASTNode> elements,
                                   TokenMetadata metadata) {
  return ASTNode{NodeType::VEC_LITERAL, elements, {}, metadata};
}

ASTNode ASTNode::makeVarLookup(string identifier, TokenMetadata metadata) {
  return ASTNode{
      NodeType::VAR_LOOKUP, {}, {{"identifier", identifier}}, metadata};
}

ASTNode ASTNode::makeFunctionCall(ASTNode lvalue_expr,
                                  ASTNode arg_expr_list,
                                  TokenMetadata metadata) {
  return ASTNode{
      NodeType::FUNC_CALL, {lvalue_expr, arg_expr_list}, {}, metadata};
}

ASTNode ASTNode::makeExprList(vector<ASTNode> arg_exprs,
                              TokenMetadata metadata) {
  return ASTNode{NodeType::EXPR_LIST, arg_exprs, {}, metadata};
}

ASTNode ASTNode::makeIndexAccess(ASTNode lvalue_expr,
                                 ASTNode index_expr,
                                 TokenMetadata metadata) {
  return ASTNode{NodeType::EXPR_LIST, {lvalue_expr, index_expr}, {}, metadata};
}

ASTNode ASTNode::makeFieldAccess(ASTNode lvalue_expr,
                                 ASTNode field_expr,
                                 TokenMetadata metadata) {
  return ASTNode{
      NodeType::FIELD_ACESS, {lvalue_expr, field_expr}, {}, metadata};
}

ASTNode ASTNode::makeBinaryOp(TokenType op,
                              ASTNode lhs_expr,
                              ASTNode rhs_expr,
                              TokenMetadata metadata) {
  // function call special case
  if (op == TokenType::LPAREN) {
    return ASTNode::makeFunctionCall(lhs_expr, rhs_expr, metadata);
  }

  // index access special case
  if (op == TokenType::LBRACKET) {
    return ASTNode::makeIndexAccess(lhs_expr, rhs_expr, metadata);
  }

  // field access special case
  if (op == TokenType::DOT) {
    return ASTNode::makeFieldAccess(lhs_expr, rhs_expr, metadata);
  }

  // should the other operators get special cases too? Should any of them get
  // special cases?
  return ASTNode{NodeType::BINARY_OP,
                 {lhs_expr, rhs_expr},
                 {{"op_name", token_type_to_string(op)}, {"op", (int)op}},
                 metadata};
}

ASTNode ASTNode::makeUnaryOp(TokenType op,
                             ASTNode expr,
                             TokenMetadata metadata) {
  return ASTNode{NodeType::UNARY_OP,
                 {expr},
                 {{"op_name", token_type_to_string(op)}, {"op", (int)op}},
                 metadata};
}

ASTNode ASTNode::makeAssignOp(TokenType op,
                              ASTNode lhs_expr,
                              ASTNode rhs_expr,
                              TokenMetadata metadata) {
  // could probably rewrite rule this immediately
  return ASTNode{NodeType::ASSIGN_OP,
                 {lhs_expr, rhs_expr},
                 {{"op_name", token_type_to_string(op)}, {"op", (int)op}},
                 metadata};
}

ASTNode ASTNode::makeReturn(ASTNode value, TokenMetadata metadata) {
  return ASTNode{NodeType::RETURN, {value}, {}, metadata};
}

ASTNode ASTNode::makeLiteral(string value, TokenMetadata metadata) {
  return ASTNode{NodeType::STRING_LITERAL, {}, {{"value", value}}, metadata};
}

ASTNode ASTNode::makeLiteral(int value, TokenMetadata metadata) {
  return ASTNode{NodeType::INT_LITERAL, {}, {{"value", value}}, metadata};
}

ASTNode ASTNode::makeLiteral(float value, TokenMetadata metadata) {
  return ASTNode{NodeType::FLOAT_LITERAL, {}, {{"value", value}}, metadata};
}

ASTNode ASTNode::makeLiteral(bool value, TokenMetadata metadata) {
  return ASTNode{NodeType::BOOL_LITERAL, {}, {{"value", value}}, metadata};
}

ASTNode ASTNode::nothing() {
  return ASTNode{};
}