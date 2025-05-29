#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "astnode.h"
#include "codegen.h"
#include "nodetype.h"
#include "tokentype.h"

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
  int intermediates;
};

static CompSymbolTable SYMBOLS{nullptr, {}, {}, {"print"}, 0, 0};

void emit(string &s) { std::cout << s; }

void emit(const char *s) { std::cout << s; }

CompNodeResult gen_top_level(ASTNode &node) {
  emit("#include \"runtime.h\"\n");

  for (auto &child : node.children) {
    gen_node(child);
    emit(";\n");
  }

  // encode main entrypoint that goes from C main to program main.
  emit("int main(int argc, char **argv) { \n\
    L528_main(); \n\
    return 0; \n\
}\n");

  return CompNodeResult{};
}

CompNodeResult gen_function_declare(ASTNode &node) {
  string name = node.data.at("function_name").get<string>();
  vector<string> args = node.data.at("args").get<vector<string>>();
  auto body = node.children[0];

  // TODO: this is bad. all variables and functions need to be in the same
  // symbol table otherwise naming collisions won't be handled correctly.
  if (SYMBOLS.functions.contains(name)) {
    throw std::runtime_error("Function/global name already taken");
  }

  emit("void ");
  emit(" L528_");
  emit(name);
  emit("(");

  // TODO: we'd need to map from args to variable names and store this in the
  // symbol table
  for (size_t i = 0; i < args.size(); ++i) {
    std::stringstream arg;
    arg << "RuntimeObject* arg" << i;
    if (i != args.size() - 1) {
      arg << ", ";
    }
    auto s = arg.str();
    emit(s);
  }

  emit("){\n");
  gen_node(body);
  emit("}");
  return CompNodeResult{};
}

CompNodeResult gen_block(ASTNode &node) {
  for (auto &child : node.children) {
    gen_node(child);
    emit(";\n");
    // auto result = gen_node(child);
    // if (result.returned) {
    //   return result;
    // }
  }
  return CompNodeResult{};
}

CompNodeResult gen_var_declare(ASTNode &node) {
  string identifier = node.data.at("identifier").get<string>();
  bool is_const = node.data.at("const").get<bool>();

  // TODO: check if taken
  std::stringstream declare_stmt;
  if (is_const) {
    declare_stmt << "const ";
  }
  declare_stmt << "RuntimeObject* ";

  // TODO: shouldn't be global, should be passed in as context
  std::stringstream local_id;
  local_id << "local" << SYMBOLS.locals;
  SYMBOLS.locals++;
  SYMBOLS.variables[identifier] = local_id.str();

  // eval rhs
  auto rhs = node.children[0];
  auto rhs_result = gen_node(rhs);

  declare_stmt << local_id.str() << " = " << rhs_result.result_loc.value()
               << ";";
  auto s = declare_stmt.str();
  emit(s);

  return CompNodeResult{local_id.str()};
}

CompNodeResult gen_bool_literal(ASTNode &node) {
  auto value = node.data.at("value").get<bool>();
  std::stringstream ss;
  ss << "make_bool(" << value << ")";
  auto s = ss.str();
  //   emit(s);
  return CompNodeResult{s};
}

CompNodeResult gen_int_literal(ASTNode &node) {
  auto value = node.data.at("value").get<int>();
  std::stringstream ss;
  ss << "make_int(" << value << ")";
  auto s = ss.str();
  //   emit(s);
  return CompNodeResult{s};
}

CompNodeResult gen_float_literal(ASTNode &node) {
  auto value = node.data.at("value").get<float>();
  std::stringstream ss;
  ss << "make_float(" << value << ")";
  auto s = ss.str();
  //   emit(s);
  return CompNodeResult{s};
}

CompNodeResult gen_string_literal(ASTNode &node) {
  auto value = node.data.at("value").get<string>();
  std::stringstream ss;
  ss << "make_string(\"" << value << "\")";
  auto s = ss.str();
  //   emit(s);
  return CompNodeResult{s};
}

CompNodeResult gen_nothing_literal(ASTNode &node) {
  //   emit("make_nothing()");
  return CompNodeResult{"make_nothing()"};
}

CompNodeResult gen_function_call(ASTNode &node) {
  const size_t FUNCTION = 0, ARGS = 1;

  // TODO: rigid, won't work everywhere
  auto lhs = node.children[FUNCTION];
  assert(lhs.type == NodeType::VAR_LOOKUP);

  std::stringstream fn_name;

  auto identifier = lhs.data.at("identifier").get<string>();
  if (SYMBOLS.builtins.contains(identifier)) {
    fn_name << "builtin_";
  } else if (SYMBOLS.functions.contains(identifier)) {
    fn_name << "L528_";
  } else {
    throw std::runtime_error("Bad function name lookup");
  }

  fn_name << identifier;

  // TODO: some kind of arg matching?
  auto s = fn_name.str();
  emit(s);

  auto rhs = node.children[ARGS];
  gen_node(rhs);
  return CompNodeResult{};
}

CompNodeResult gen_var_lookup(ASTNode &node) {
  const string identifier = node.data.at("identifier").get<string>();
  auto var = SYMBOLS.variables.at(identifier);
  emit(var);
  return CompNodeResult{};
}

CompNodeResult gen_expr_list(ASTNode &node) {
  emit("(");
  auto child_nodes = node.children;
  for (size_t i = 0; i < child_nodes.size(); ++i) {
    auto this_node = child_nodes[i];
    gen_node(this_node);
    if (i != child_nodes.size() - 1) {
      emit(",");
    }
  }
  emit(")");

  return CompNodeResult{};
}

CompNodeResult gen_binary_op(ASTNode &node) {
  const size_t LHS = 0, RHS = 1;
  const string op_key = "op";
  auto op = int_to_token_type(node.data.at(op_key).get<int>());

  assert(op == TokenType::PLUS);

  std::stringstream intmdt;
  intmdt << "_intmdt" << SYMBOLS.intermediates;
  SYMBOLS.intermediates++;
  auto lhs = gen_node(node.children[LHS]);
  auto rhs = gen_node(node.children[RHS]);

  auto intmdt_str = intmdt.str();
  emit("RuntimeObject* ");
  emit(intmdt_str);
  emit(" = ");
  std::stringstream add;
  add << "op_add(" << lhs.result_loc.value() << ", " << rhs.result_loc.value()
      << ");\n";
  auto s = add.str();
  emit(s);

  return CompNodeResult{intmdt_str};
}

CompNodeResult gen_node(ASTNode &node) {
  switch (node.type) {
  case NodeType::TOP_LEVEL:
    return gen_top_level(node);
    break;
  case NodeType::BLOCK:
    return gen_block(node);
    break;
    //   case NodeType::ASSIGN_OP:
    //     return eval_assign_op(node, st);
    //     break;
  case NodeType::VAR_DECLARE:
    return gen_var_declare(node);
    break;
  case NodeType::FUNC_DECLARE:
    return gen_function_declare(node);
    break;
  // case NodeType::MODULE_IMPORT:
  //   return eval_module_import(node, st);
  //   break;
  // case NodeType::IF:
  //   return eval_if(node, st);
  //   break;
  // case NodeType::RETURN:
  //   return eval_return(node, st);
  //   break;
  // case NodeType::WHILE:
  //   return eval_while(node, st);
  //   break;
  case NodeType::BINARY_OP:
    return gen_binary_op(node);
    break;
    // case NodeType::UNARY_OP:
    //   return eval_unary_op(node, st);
    //   break;
  case NodeType::FUNC_CALL:
    return gen_function_call(node);
    break;
    // case NodeType::INDEX_ACCESS:
    //   return eval_index_access(node, st, vt);
    //   break;
    // case NodeType::FIELD_ACESS:
    //   return eval_field_access(node, st, vt);
    //   break;
  case NodeType::VAR_LOOKUP:
    return gen_var_lookup(node);
    break;
  case NodeType::EXPR_LIST:
    return gen_expr_list(node);
    break;
  // case NodeType::VEC_LITERAL:
  //   return eval_vec_literal(node, st);
  //   break;
  // case NodeType::DICT_LITERAL:
  //   return eval_dict_literal(node, st);
  //   break;
  case NodeType::BOOL_LITERAL:
    return gen_bool_literal(node);
    break;
  case NodeType::INT_LITERAL:
    return gen_int_literal(node);
    break;
  case NodeType::FLOAT_LITERAL:
    return gen_float_literal(node);
    break;
  case NodeType::STRING_LITERAL:
    return gen_string_literal(node);
    break;
  case NodeType::NOTHING_LITERAL:
    return gen_nothing_literal(node);
    break;

  default:
    throw std::runtime_error("Not implemented: " +
                             node_type_to_string(node.type));
  }
}
