

#include <vector>

#include <nlohmann/json.hpp>
#include "parser.h"
#include "token.h"

using std::vector;
using std::string;
using json = nlohmann::json;


ASTNode const_var_declare(ParserState ps) {}

ASTNode let_var_declare(ParserState ps) {}

ASTNode if_block(ParserState ps);

ASTNode while_loop(ParserState ps);

ASTNode return_statement(ParserState ps);

vector<ASTNode> expr_list(ParserState ps);

ASTNode unary_op(ParserState ps);

ASTNode binary_op(ParserState ps);

ASTNode expr(ParserState ps);

ASTNode basic_literal(ParserState ps);

ASTNode vector_literal(ParserState ps) {
    auto first_token_metadata = ps.currentToken().metadata;
    // '['
    ps.expect(TokenType::LBRACKET);
    
    vector<ASTNode> elements;
    if(ps.currentTokenIsNot(TokenType::RBRACKET)) {
        elements = expr_list(ps);
        ps.expect(TokenType::RBRACKET);
    }
    else {
        ps.advance();
    }
    
    return ASTNode::makeVectorLiteral(elements, first_token_metadata);
}



ASTNode primary_prime(ParserState ps) {
    // primary ::= '(' expression ')' | NUMBER | VARIABLE | '-' primary
    
    
    // primary ::= '(' expression ')' | LITERAL | '-/!' primary
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
    else if (ps.currentTokenIs(TokenType::MINUS) || ps.currentTokenIs(TokenType::NOT)) {
        return unary_op(ps);
    }
    // vector literal
    else if (ps.currentTokenIs(TokenType::LBRACKET)) {
        return vector_literal(ps); 
    }
    else if (ps.currentTokenIs(TokenType::IDENTIFIER)) {
        auto identifier = std::get<string>(current_token.value);
        auto primary = ASTNode::makeVarLookup(identifier, current_token.metadata);
        ps.advance(); 
        return primary;
    }

    // all other literals (number, string, bool)
    return basic_literal(ps);
}

ASTNode expr_helper(ParserState ps, ASTNode lhs, int min_precedence=0) {

    while( is_binary_op(ps.currentToken().type) && 
          (op_precedence(ps.currentToken().type) >= min_precedence)
    ) {
        auto op = ps.advance().type;
        auto rhs = primary_prime(ps);

        /*
        while lookahead is a binary operator whose precedence is greater
                 than op's, or a right-associative operator
                 whose precedence is equal to op's
        */
    }
}

ASTNode expr(ParserState ps) {
    return expr_helper(ps, primary_prime(ps));
}

ASTNode primary_expr(ParserState ps) {
    // TODO:
    // restructure this into proper operator precedence parsing


    auto current_token = ps.currentToken(); 
    auto first_token_metadata = current_token.metadata;
    auto current_token_type = current_token.type;

    // parse vector literals and unary operators
    switch (current_token_type) {
        case TokenType::LBRACKET:
            return vector_literal(ps); 
        case TokenType::NOT:
        case TokenType::MINUS:
            return unary_op(ps);
        default: break;
    }

    // next, try to parse identifiers and parenthesized expressions
    ASTNode primary;
    if ( current_token_type == TokenType::LPAREN ) {
        ps.advance();
        primary = expr(ps);
        ps.expect(TokenType::RPAREN);
    }
    else if ( current_token_type == TokenType::IDENTIFIER ) {
        auto identifier = std::get<string>(current_token.value);
        primary = ASTNode::makeVarLookup(identifier, first_token_metadata);
        ps.advance();
    }
    else {
        return basic_literal(ps);
    }

    // check if identifier or parenthesized expr is follow by function call parens, index brackets, or a dot field access
    if ( ps.currentTokenIs(TokenType::LPAREN) ) {
        ps.advance();

        vector<ASTNode> arg_exprs;
        if (ps.currentTokenIsNot(TokenType::RPAREN)) {
            arg_exprs = expr_list(ps); 
            ps.expect(TokenType::RPAREN); 
        }
        else {
            ps.advance();
        }

        primary = ASTNode::makeFunctionCall(primary, arg_exprs, first_token_metadata);
    }
    else if (ps.currentTokenIs(TokenType::LBRACKET)) {
        ps.advance();
        auto index_expr = expr(ps);
        ps.expect(TokenType::RBRACKET);
        primary = ASTNode::makeIndexAccess(primary, index_expr, first_token_metadata);
    }
    else if (ps.currentTokenIs(TokenType::DOT)) {
        ps.advance();

    }

    return primary;
}

/*
    Parse any statement that could occur in a block:
       - variable declaration
       - assignments
       - conditionals
       - loops
       - standalone expressions with side-effects
*/
ASTNode statement(ParserState ps) {
    // If we encounter an identifier, this might be an assignment or a standalone function call
    /*
        e.g

        just_a_var = 5 + x;

        an_array[x] = 5;

        a_function(x, y, z);

        an_object.something = 5;
        
        etc.

        TODO: handle simple and array/object assignment, as well as standalone function calls
    */

    auto current_token_type = ps.currentToken().type;
    if ( current_token_type == TokenType::IDENTIFIER ) {
        auto lvalue = primary_expr(ps);

        // if next token is a semicolon then we're done...

        // if it's an assignment-like operator, then we do an assignment
    }


    switch(ps.currentToken().type) {
        case TokenType::LET:  
            return let_var_declare(ps);
        case TokenType::CONST:
            return const_var_declare(ps);
        case TokenType::WHILE:
            return while_loop(ps);
        case TokenType::RETURN:
            return return_statement(ps);
        default:
        {
            // TODO: complain 
        }
    }


}

/*
    Parse a block.

    A block is a series of zero or more statements, and then a DOT_DOT (..) to close.
*/
ASTNode block(ParserState ps) {

    // grab first token metadata for debug/error info
    auto first_token_metadata = ps.currentToken().metadata;
    
    vector<ASTNode> statements;
    while ( ps.currentTokenIsNot(TokenType::DOT_DOT) ) {
        auto stmt = statement(ps);
        statements.push_back(stmt);
    }

    return ASTNode::makeBlock(statements, first_token_metadata);
}

ASTNode function_declare(ParserState ps) {
    // 'function' keyword
    auto first_token_metadata = ps.expect(TokenType::FUNCTION).metadata;

    // function name identifier
    auto fn_name = std::get<string>(ps.expect(TokenType::IDENTIFIER).value);

    // '('
    ps.expect(TokenType::LPAREN);

    // parse zero or more arguments 
    // TODO: in the future args could contain type info, default values, etc.
    vector<string> arg_names;
    while(ps.currentTokenIsNot(TokenType::RPAREN)) {

        // get token and advance
        auto arg = std::get<string>(ps.expect(TokenType::IDENTIFIER).value);
        arg_names.push_back(arg);

        // if next token is a comma, advance
        if (ps.currentTokenIs(TokenType::COMMA)) {
            ps.advance();
        }
    }

    // ')' (could just advance since loop above only exits once currentToken is RPAREN)
    ps.expect(TokenType::RPAREN);

    // parse function body
    auto body = block(ps);

    return ASTNode::makeFunctionDeclare(fn_name, arg_names, body, first_token_metadata);
}


/*
* parse the top level of a file (variable and function definition statements)
*/
ASTNode top_level(ParserState ps) {
    vector<ASTNode> children;
    
    while (ps.hasNext() ) {
        ASTNode child;

        auto currentToken= ps.currentToken();
        switch(currentToken.type) {
            case TokenType::CONST:
            {
                child = const_var_declare(ps);
                break;
            }
            case TokenType::LET:
            {
                child = let_var_declare(ps);
                break;
            }
            case TokenType::FUNCTION:
            {
                child = function_declare(ps);
                break;
            }
            default:
            {
                // TODO: complain
            }
        }
        children.push_back(child);
    }


}


/*
* parser entrypoint function
*/
ASTNode parse_tokens(vector<Token> tokens) {
    ParserState ps {tokens};

    return top_level(ps);
}
