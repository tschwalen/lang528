

#include <vector>

#include <nlohmann/json.hpp>
#include "parser.h"
#include "token.h"

using std::vector;
using std::string;
using json = nlohmann::json;


/* 

Use this space for planning first. Need to decide on data representation of the AST. Using OO kind of sucks with
C++, last time I wrote a parser that used a unique class for each AST node type with a base class to enable polymorphism,
I really wasn't happy with the inflexibility...

So instead I propose something like this: ( I experimented with this approach in flexcomp as well)

Node {
    type: Enum;                 # not sure how fine-grained I want to be with type. Could be like "UnaryOp, BinaryOp, AssignOp, etc."
                                # or could be as specific as ADD, MULTIPLY, INCREMENT, etc.

    children: [Node]            # either a vector or map of nodes (or a pointer to one for better memory usage)
                                # This provides great flexibility, since you can make assumptions about order, number, and so on.
                                # For example, in binary operations, you can say the first arg is the left operand, and the second one
                                # is the right.
                                # 
                                # Also usable for stuff like if/else

    metadata: {string -> ?}     # this gets interesting since we already store metadata for tokens about position in the file.
                                #  
                                # In my previous attemps to represent AST nodes with this sort of schema, I've also kept a 
                                # 'metadata' entry, which stores information needed for interpretation or codegen, like 
                                # function name for a function declare, argument names for for a function declare, variable 
                                # name for a variable lookup and so on.
                                #
                                # Furthermore, I did this in python where it's very easy to have a dict whose values could
                                # be strings, integers, a vector of strings, and so on.
                                #
                                # So I have to think about whether I want to structure this as:
                                #
                                #   unordered_map<
                                #        string, 
                                #        MetadataEntry { 
                                #            type: string | string_vector, 
                                #            value: variant<string, vector<string>>
                                #        }
                                #    >
                                #
                                # Or if I want to use the kind German fellow's JSON library for this.
}

*/


ASTNode function_declare(ParserState ps) {
    // 'function' keyword
    ps.expect(TokenType::FUNCTION);

    // function name identifier
    auto fn_name = std::get<string>(ps.expect(TokenType::IDENTIFIER).value);

    // '('
    ps.expect(TokenType::LPAREN);

    // parse zero or more arguments 
    // TODO: in the future args could contain type info, default values, etc.
    vector<string> arg_names;
    if ( ps.currentTokenIs(TokenType::IDENTIFIER) ) {
        // current token is an identifier 
        do {
            // get the identifier and add it to list
            auto arg = std::get<string>(ps.currentToken().value);
            arg_names.push_back(arg);

            // advance to the next token, either a comma or an rparen which closes the arg list
            ps.bumpToken();
        } 

        // TODO: there is probably a less hacky way to parse this
        // due to short-circuiting, bumpToken() is only called if current token is comma
        while ( ps.currentTokenIs(TokenType::COMMA) && ps.bumpToken().type != TokenType::END_OF_FILE );
    }

    ps.expect(TokenType::RPAREN);

    // TODO: add arg list to "data" in JSON format
    // make the AST node, parse the body, and so on

}

ASTNode const_var_declare(ParserState ps) {}
ASTNode let_var_declare(ParserState ps) {}

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
