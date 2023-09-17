#pragma once

#include <vector>

#include <nlohmann/json.hpp>

#include "token.h"

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

struct ASTNode {
    NodeType type; 
    std::vector<ASTNode> children;
    nlohmann::json data;
    TokenMetadata metadata;
};

class ParserState 
{
public:
    std::vector<Token> tokens;
    int index;

    ParserState (std::vector<Token> tokens_, int index_=0)
        : tokens { std::move(tokens_) }, index { index_ } {}

    bool hasNext();
    Token currentToken();
    Token bumpToken();

    /* If current_token.type == t, return the the Token and advance */
    Token expect(TokenType t);
    bool currentTokenIs(TokenType t);
    
    // Token peekToken(int n);
    // Token advance();
    // Token matchKeyword(std::string kwrd);
    // Token matchTokenType(TokenType ttype);
    // Token matchSymbol(std::string smbl);
    // Token matchLiteral();
    void error(std::string msg);
};



ASTNode parse_tokens(std::vector<Token> tokens);

