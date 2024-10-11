#include <string>
#include <iostream>

#include "astnode.h"
#include "nodetype.h"
#include "codegen.h"

using std::string;

// TODO: define compiler symbol table ...
// Might need to pass around more context than that,

void emit(string &s) {
    std::cout << s;
}

void emit(const char* s) {
    std::cout << s;
}

void gen_top_level(ASTNode &node) {
    for (auto &child : node.children) {
        gen_node(child);
    }

    // encode main entrypoint that goes from C main to program main.
    emit(
"int main(int argc, char **argv) { \
    L528_main(); \
    return 0; \
}");
}

void gen_function_declare(ASTNode &node) {}

void gen_block(ASTNode &node) {}

void gen_var_declare(ASTNode &node) {}

void gen_int_literal(ASTNode &node) {}

void gen_string_literal(ASTNode &node) {}

void gen_function_call(ASTNode &node) {}

void gen_var_lookup(ASTNode &node) {}

void gen_expr_list(ASTNode &node) {}

void gen_node(ASTNode &node) {
    switch (node.type) {
        case NodeType::TOP_LEVEL:
            gen_top_level(node);
            break;
        case NodeType::FUNC_DECLARE:
            gen_function_declare(node);
            break;
        case NodeType::BLOCK:
            gen_block(node);
            break;
        case NodeType::VAR_DECLARE:
            gen_var_declare(node);
            break;
        case NodeType::INT_LITERAL:
            gen_int_literal(node);
            break;
        case NodeType::STRING_LITERAL:
            gen_string_literal(node);
            break;
        case NodeType::FUNC_CALL:
            gen_function_call(node);
            break;
        case NodeType::VAR_LOOKUP:
            gen_var_lookup(node);
            break;
        case NodeType::EXPR_LIST:
            gen_expr_list(node);
            break;
        default:
    }
}
