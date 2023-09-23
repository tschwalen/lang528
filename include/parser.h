#pragma once

#include <vector>

#include <nlohmann/json.hpp>

#include "token.h"

using std::vector;
using std::string;

enum class NodeType {
    TOP_LEVEL,
    BLOCK,
    ASSIGN_OP,
    VAR_DECLARE,
    FUNC_DECLARE,
    IF, // first child -> body, second child -> else (if present)
    WHILE,
    BINARY_OP, // e.g. x + y, 5 * 5, etc.
    UNARY_OP, // e.g. !x, -5, etc.
    FUNC_CALL, 
    INDEX_ACCESS, // e.g. "sub" X[Y]
    VAR_LOOKUP,

    VEC_LITERAL, // e.g. brackets, which can contain sub-expressions
    BASIC_LITERAL // numbers, strings, true/false, etc.
};

class ASTNode {
public:
    NodeType type; 
    vector<ASTNode> children;
    nlohmann::json data;
    // TODO: could stick metadata directly into data json?
    TokenMetadata metadata; 

    // factory methods
    static ASTNode makeFunctionDeclare(string name, vector<string> args, ASTNode body, TokenMetadata metadata);
    static ASTNode makeBlock(vector<ASTNode> statements, TokenMetadata metadata);
    static ASTNode makeVectorLiteral(vector<ASTNode> elements, TokenMetadata metadata);
    static ASTNode makeVarLookup(string identifier, TokenMetadata metadata);
    static ASTNode makeFunctionCall(ASTNode lvalue_expr, vector<ASTNode> arg_exprs, TokenMetadata metadata);
    static ASTNode makeIndexAccess(ASTNode lvalue_expr, ASTNode index_expr, TokenMetadata metadata);
};

class ParserState 
{
public:
    vector<Token> tokens;
    int index;

    ParserState (vector<Token> tokens_, int index_=0)
        : tokens { std::move(tokens_) }, index { index_ } {}

    bool hasNext();
    Token currentToken();
    Token advance();

    /* If current_token.type == t, return the the Token and advance */
    Token expect(TokenType t);
    bool currentTokenIs(TokenType t);
    bool currentTokenIsNot(TokenType t);
    
    // Token peekToken(int n);
    // Token advance();
    // Token matchKeyword(std::string kwrd);
    // Token matchTokenType(TokenType ttype);
    // Token matchSymbol(std::string smbl);
    // Token matchLiteral();
    void error(string msg);
};



ASTNode parse_tokens(vector<Token> tokens);

