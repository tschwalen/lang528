#include <vector>

#include "astnode.h"
#include "parser.h"
#include "token.h"
#include "tokentype.h"

#include <nlohmann/json.hpp>

using std::string;
using std::vector;
using json = nlohmann::json;

ASTNode expr(ParserState &ps);
ASTNode primary(ParserState &ps);
ASTNode block(ParserState &ps);
ASTNode statement(ParserState &ps);
ASTNode expr_helper(ParserState &ps, ASTNode lhs, int min_precedence = 0);

ASTNode var_declare(ParserState &ps, TokenType type) {
  assert(type == TokenType::CONST || type == TokenType::LET);
  auto metadata = ps.expect(type).metadata;
  auto id_token = ps.expect(TokenType::IDENTIFIER);
  auto id_name = std::get<string>(id_token.value);

  ps.expect(TokenType::EQUALS);
  auto rhs = expr(ps);
  ps.expect(TokenType::SEMICOLON);

  return ASTNode::makeVarDeclare(id_name, rhs, type == TokenType::CONST,
                                 metadata);
}

ASTNode if_block(ParserState &ps) {
  assert(ps.currentTokenIs(TokenType::IF) ||
         ps.currentTokenIs(TokenType::ELSEIF));
  // advance over the if/elseif but keep the metadata
  auto metadata = ps.advance().metadata;
  auto condition = expr(ps);

  auto body_metadata = ps.currentToken().metadata;
  vector<ASTNode> statements;
  while (ps.currentTokenIsNot(TokenType::DOT_DOT) &&
         ps.currentTokenIsNot(TokenType::ELSEIF) &&
         ps.currentTokenIsNot(TokenType::ELSE)) {
    auto stmt = statement(ps);
    statements.push_back(stmt);
  }

  auto body = ASTNode::makeBlock(statements, body_metadata);

  if (ps.currentTokenIs(TokenType::DOT_DOT)) {
    ps.advance();
    return ASTNode::makeIf(condition, body, metadata);
  }

  // otherwise, handle and if/elseif block
  ASTNode else_body;
  if (ps.currentTokenIs(TokenType::ELSEIF)) {
    // elseif is basically just another if, but with a different keyword
    else_body = if_block(ps);
  } else {
    ps.advance();
    else_body = block(ps);
  }

  return ASTNode::makeIfElse(condition, body, else_body, metadata);
}

ASTNode while_loop(ParserState &ps) {
  auto metadata = ps.expect(TokenType::WHILE).metadata;
  auto condition = expr(ps);
  auto body = block(ps);
  return ASTNode::makeWhile(condition, body, metadata);
}

ASTNode return_statement(ParserState &ps) {
  auto metadata = ps.expect(TokenType::RETURN).metadata;
  auto value = expr(ps);
  ps.expect(TokenType::SEMICOLON);
  return ASTNode::makeReturn(value, metadata);
}

vector<ASTNode> expr_list(ParserState &ps) {
  vector<ASTNode> exprs;
  do {
    exprs.push_back(expr(ps));
  } while (ps.matchTokenType(TokenType::COMMA));
  return exprs;
}

ASTNode unary_op(ParserState &ps) {
  // current token is a unary op
  assert(is_unary_op(ps.currentToken().type));
  auto op_token = ps.advance();
  // previously:
  // auto rhs = primary(ps);
  auto rhs = expr_helper(ps, primary(ps), unary_op_precedence(op_token.type));

  return ASTNode::makeUnaryOp(op_token.type, rhs, op_token.metadata);
}

ASTNode basic_literal(ParserState &ps) {
  auto current_token = ps.advance();
  switch (current_token.type) {
  case TokenType::BOOL_LITERAL:
    return ASTNode::makeLiteral(std::get<bool>(current_token.value),
                                current_token.metadata);
  case TokenType::FLOAT_LITERAL:
    return ASTNode::makeLiteral(std::get<double>(current_token.value),
                                current_token.metadata);
  case TokenType::INT_LITERAL:
    return ASTNode::makeLiteral(std::get<int>(current_token.value),
                                current_token.metadata);
  case TokenType::STRING_LITERAL:
    return ASTNode::makeLiteral(std::get<string>(current_token.value),
                                current_token.metadata);
  case TokenType::NOTHING_LITERAL:
    return ASTNode::makeNothingLiteral(current_token.metadata);
  default:
    break;
  }
  ps.error("Expected literal, got something else");
  return ASTNode::nothing();
}

ASTNode dict_literal(ParserState &ps) {
  auto first_token_metadata = ps.currentToken().metadata;

  // '{'
  ps.expect(TokenType::LBRACE);

  // we're just gonna store it in one array and handle them two-by-two
  vector<ASTNode> kv_pairs;
  if (ps.currentTokenIsNot(TokenType::RBRACE)) {
    do {
      auto key = expr(ps);
      ps.expect(TokenType::COLON);
      auto value = expr(ps);
      kv_pairs.push_back(key);
      kv_pairs.push_back(value);
    } while (ps.matchTokenType(TokenType::COMMA));
    ps.expect(TokenType::RBRACE);
  } else {
    ps.advance();
  }

  return ASTNode::makeDictLiteral(kv_pairs, first_token_metadata);
}

ASTNode vector_literal(ParserState &ps) {
  auto first_token_metadata = ps.currentToken().metadata;
  // '['
  ps.expect(TokenType::LBRACKET);

  vector<ASTNode> elements;
  if (ps.currentTokenIsNot(TokenType::RBRACKET)) {
    elements = expr_list(ps);
    ps.expect(TokenType::RBRACKET);
  } else {
    ps.advance();
  }

  return ASTNode::makeVectorLiteral(elements, first_token_metadata);
}

ASTNode primary(ParserState &ps) {
  // Tries to use a standard operator precedence parsing algorithm.
  // Not certain if this implementation is correct, needs more testing.
  //
  // primary ::= '(' expression ')' | LITERAL | VARIABLE | '-/!' primary
  // https://en.wikipedia.org/wiki/Operator-precedence_parser
  auto current_token = ps.currentToken();

  // parenthesized expr
  if (ps.currentTokenIs(TokenType::LPAREN)) {
    ps.advance();
    auto primary = expr(ps);
    ps.expect(TokenType::RPAREN);
    return primary;
  }
  // unary - or !
  else if (ps.currentTokenIs(TokenType::MINUS) ||
           ps.currentTokenIs(TokenType::NOT)) {
    return unary_op(ps);
  }
  // vector literal
  else if (ps.currentTokenIs(TokenType::LBRACKET)) {
    return vector_literal(ps);
  }
  // dict literal
  else if (ps.currentTokenIs(TokenType::LBRACE)) {
    return dict_literal(ps);
  } else if (ps.currentTokenIs(TokenType::IDENTIFIER)) {
    auto identifier = std::get<string>(current_token.value);
    auto primary = ASTNode::makeVarLookup(identifier, current_token.metadata);
    ps.advance();
    return primary;
  }

  // all other literals (number, string, bool)
  return basic_literal(ps);
}

ASTNode expr_helper(ParserState &ps, ASTNode lhs, int min_precedence) {

  while (is_binary_op(ps.currentToken().type) &&
         (binary_op_precedence(ps.currentToken().type) >= min_precedence)) {
    auto op_token = ps.advance();
    auto op = op_token.type;

    ASTNode rhs;
    // handle operator special cases
    if (op == TokenType::LPAREN) {
      // function call takes an expression list and a closing paren

      vector<ASTNode> arg_exprs;
      if (ps.currentTokenIsNot(TokenType::RPAREN)) {
        arg_exprs = expr_list(ps);
        ps.expect(TokenType::RPAREN);
      } else {
        ps.advance();
      }
      rhs = ASTNode::makeExprList(arg_exprs, op_token.metadata);
    } else if (op == TokenType::LBRACKET) {
      rhs = expr(ps);
      ps.expect(TokenType::RBRACKET);
    } else {
      rhs = primary(ps);
    }

    auto lookahead = ps.currentToken().type;

    /*
    while lookahead is a binary operator whose precedence is greater than op's,
        or a right-associative operator whose precedence is equal to op's */
    while (binary_precedence_test(op, lookahead)) {
      // rhs := parse_expression_1 (rhs, precedence of op + (1 if lookahead
      // precedence is greater, else 0))
      auto new_precedence = binary_op_precedence(op) + 1;
      rhs = expr_helper(ps, rhs, new_precedence);

      lookahead = ps.currentToken().type;
    }

    // just have "make binary op" handle the function call and index access
    lhs = ASTNode::makeBinaryOp(op, lhs, rhs, op_token.metadata);
  }
  return lhs;
}

ASTNode expr(ParserState &ps) { return expr_helper(ps, primary(ps)); }

ASTNode statement(ParserState &ps) {
  /*
    Parse any statement that could occur in a block:
       - variable declaration
       - assignments
       - conditionals
       - loops
       - standalone expressions with side-effects
  */
  auto current_token_type = ps.currentToken().type;
  switch (current_token_type) {
  case TokenType::IDENTIFIER: {
    // If we encounter an identifier, this might be an assignment or a
    // standalone function call
    auto result = expr(ps);

    // if next token is an assignment-like operator, then we do an assignment
    // (deciding whether the lhs of the assignment is semantically valid wil be
    // done at a later step)
    auto maybe_assign_op = ps.currentToken().type;
    if (is_assign_op(maybe_assign_op)) {
      ps.advance();
      auto rhs = expr(ps);
      result =
          ASTNode::makeAssignOp(maybe_assign_op, result, rhs, result.metadata);
    }

    ps.expect(TokenType::SEMICOLON);
    return result;
  }
  case TokenType::LET:
  case TokenType::CONST:
    return var_declare(ps, current_token_type);
  case TokenType::WHILE:
    return while_loop(ps);
  case TokenType::IF:
    return if_block(ps);
  case TokenType::RETURN:
    return return_statement(ps);
  default:
    break;
  }
  ps.error("Expected start of statement, got something else");
  return ASTNode::nothing();
}

ASTNode block(ParserState &ps) {
  /*
      Parse a block.
      A block is a series of zero or more statements, and then a DOT_DOT (..) to
     close.
  */

  // grab first token metadata for debug/error info
  auto first_token_metadata = ps.currentToken().metadata;

  vector<ASTNode> statements;
  while (ps.currentTokenIsNot(TokenType::DOT_DOT)) {
    auto stmt = statement(ps);
    statements.push_back(stmt);
  }

  ps.expect(TokenType::DOT_DOT);

  return ASTNode::makeBlock(statements, first_token_metadata);
}

ASTNode module_import(ParserState &ps) {
  auto first_token_metadata = ps.expect(TokenType::IMPORT).metadata;

  auto module_path =
      std::get<string>(ps.expect(TokenType::STRING_LITERAL).value);

  ASTNode result;

  // handle optional named import (e.g. import "x" as y; )
  if (ps.matchTokenType(TokenType::AS)) {
    auto module_name = std::get<string>(ps.expect(TokenType::IDENTIFIER).value);
    result = ASTNode::makeModuleImport(module_path, module_name,
                                       first_token_metadata);
  } else {
    result = ASTNode::makeModuleImport(module_path, first_token_metadata);
  }

  ps.expect(TokenType::SEMICOLON);
  return result;
}

ASTNode function_declare(ParserState &ps) {
  // 'function' keyword
  auto first_token_metadata = ps.expect(TokenType::FUNCTION).metadata;

  // function name identifier
  auto fn_name = std::get<string>(ps.expect(TokenType::IDENTIFIER).value);

  // '('
  ps.expect(TokenType::LPAREN);

  // parse zero or more arguments
  // NOTE: in the future args could contain type info, default values, etc.
  vector<string> arg_names;
  while (ps.currentTokenIsNot(TokenType::RPAREN)) {

    // get token and advance
    auto arg = std::get<string>(ps.expect(TokenType::IDENTIFIER).value);
    arg_names.push_back(arg);

    // if next token is a comma, advance
    if (ps.currentTokenIs(TokenType::COMMA)) {
      ps.advance();
    }
  }

  // ')' (could just advance since loop above only exits once currentToken is
  // RPAREN)
  ps.expect(TokenType::RPAREN);

  // parse function body
  auto body = block(ps);

  return ASTNode::makeFunctionDeclare(fn_name, arg_names, body,
                                      first_token_metadata);
}

ASTNode top_level(ParserState &ps) {
  /*
   * parse the top level of a file (variable and function definition statements)
   */
  auto metadata = ps.currentToken().metadata;
  vector<ASTNode> children;

  while (ps.hasNext()) {
    ASTNode child;

    auto currentToken = ps.currentToken();
    switch (currentToken.type) {
    case TokenType::LET:
    case TokenType::CONST: {
      child = var_declare(ps, currentToken.type);
      break;
    }
    case TokenType::FUNCTION: {
      child = function_declare(ps);
      break;
    }
    case TokenType::IMPORT: {
      child = module_import(ps);
      break;
    }
    default: {
      ps.error("Expected top level statement, got something else");
    }
    }
    children.push_back(child);
  }

  return ASTNode::makeTopLevel(children, metadata);
}

ASTNode parse_tokens(vector<Token> tokens) {
  /*
   * parser entrypoint function
   */
  ParserState ps{tokens};
  return top_level(ps);
}
