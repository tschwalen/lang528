#include <cassert>
#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>


#include "astnode.h"
#include "nodetype.h"
#include "codegen.h"
#include <memory>

using std::string;
using std::unordered_map;
using std::unordered_set;

// TODO: define compiler symbol table ...
// Might need to pass around more context than that,
static int locals = 0;

struct CompSymbolTable {
    CompSymbolTable *parent;
    unordered_map<string, string> variables;
    unordered_set<string> functions;
    unordered_set<string> builtins;
    int locals;
};

static CompSymbolTable SYMBOLS {nullptr, {}, {}, {"print"}, 0};


void emit(string &s) {
    std::cout << s;
}

void emit(const char* s) {
    std::cout << s;
}

void gen_top_level(ASTNode &node) {
    emit("#include \"runtime.h\"\n");

    for (auto &child : node.children) {
        gen_node(child);
        emit(";\n");
    }

    // encode main entrypoint that goes from C main to program main.
    emit(
"int main(int argc, char **argv) { \n\
    L528_main(); \n\
    return 0; \n\
}\n");
}

void gen_function_declare(ASTNode &node) {
    string name = node.data.at("function_name").get<string>();
    vector<string> args = node.data.at("args").get<vector<string>>();
    auto body = node.children[0];

    // TODO: this is bad. all variables and functions need to be in the same symbol table
    // otherwise naming collisions won't be handled correctly.
    if(SYMBOLS.functions.contains(name)) {
        throw std::runtime_error("Function/global name already taken");
    }

    emit("void ");
    emit(" L528_");
    emit(name);
    emit("(");

    // TODO: we'd need to map from args to variable names and store this in the 
    // symbol table 
    for(size_t i = 0; i < args.size(); ++i) {
        std::stringstream arg;
        arg << "RuntimeObject* arg" << i;
        if(i != args.size() - 1) {
            arg << ", ";
        }
        auto s = arg.str();
        emit(s);
    }

    emit("){\n");
    gen_node(body);
    emit("}");
}

void gen_block(ASTNode &node) {
    for (auto &child : node.children) {
        gen_node(child);
        emit(";\n");
        // auto result = gen_node(child);
        // if (result.returned) {
        //   return result;
        // }
    }
}

void gen_var_declare(ASTNode &node) {
    string identifier = node.data.at("identifier").get<string>();
    bool is_const = node.data.at("const").get<bool>();

    // TODO: check if taken

    std::stringstream declare_stmt;
    if(is_const) {
        declare_stmt << "const ";
    }
    declare_stmt << "RuntimeObject* ";


    // TODO: shouldn't be global, should be passed in as context
    std::stringstream local_id;
    local_id << "local" << SYMBOLS.locals;
    SYMBOLS.locals++;

    SYMBOLS.variables[identifier] = local_id.str();

    declare_stmt << local_id.str() << " = ";

    auto s = declare_stmt.str();
    emit(s);
    auto rhs = node.children[0];
    gen_node(rhs);
}

void gen_int_literal(ASTNode &node) {
    auto value = node.data.at("value").get<int>();
    std::stringstream ss;
    ss << "make_int(" << value << ")";
    auto s = ss.str();
    emit(s);
}

void gen_string_literal(ASTNode &node) {
    auto value = node.data.at("value").get<string>();
    std::stringstream ss;
    ss << "make_string(\"" << value << "\")";
    auto s = ss.str();
    emit(s);
}

void gen_function_call(ASTNode &node) {
    const size_t FUNCTION = 0, ARGS = 1;

    // TODO: rigid, won't work everywhere
    auto lhs = node.children[FUNCTION];
    assert(lhs.type == NodeType::VAR_LOOKUP);

    std::stringstream fn_name;

    auto identifier = lhs.data.at("identifier").get<string>();
    if(SYMBOLS.builtins.contains(identifier)) {
        fn_name << "builtin_";
    } 
    else 
    if (SYMBOLS.functions.contains(identifier)) {
        fn_name << "L528_";
    } 
    else {
        throw std::runtime_error("Bad function name lookup");
    }

     fn_name << identifier;


    // TODO: some kind of arg matching?
    auto s = fn_name.str();
    emit(s);

    auto rhs = node.children[ARGS];
    gen_node(rhs);
}

void gen_var_lookup(ASTNode &node) {
    const string identifier = node.data.at("identifier").get<string>();
    auto var = SYMBOLS.variables.at(identifier);
    emit(var);
}

void gen_expr_list(ASTNode &node) {
    emit("(");
    auto child_nodes = node.children;
    for(size_t i = 0; i < child_nodes.size(); ++i) {
        auto this_node = child_nodes[i];
        gen_node(this_node);
        if(i != child_nodes.size() - 1) {
            emit(",");
        }
    }
    emit(")");
}

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
