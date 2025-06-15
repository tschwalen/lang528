#include <cassert>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "astnode.h"
#include "codegen.h"
#include "nodetype.h"
#include "tokentype.h"

using std::string;
using std::unordered_map;
using std::vector;

// TODO: global context like this isn't great
// Might need to pass around more context than that,
static int LABELS = 0;
static int LOCALS = 0;

static vector<std::pair<string, ASTNode *>> toplevel_decls;

bool is_toplevel_st(CompSymbolTable &st) { return st.parent == nullptr; }

// TODO: I'm basically reinventing OOP here, may just need to refactor
std::optional<CompTableEntry> st_lookup_symbol(CompSymbolTable &st,
                                               string symbol) {
  CompSymbolTable *curr_st = &st;
  while (curr_st != nullptr) {
    if (curr_st->entries.contains(symbol)) {
      return curr_st->entries.at(symbol);
    }
    curr_st = curr_st->parent;
  }
  return {};
}

string st_new_intmdt(CompSymbolTable &st) {
  std::stringstream intmdt;
  intmdt << "_intmdt" << st.intermediates;
  st.intermediates++;
  auto intmdt_id = intmdt.str();
  return intmdt_id;
}

CompNodeResult gen_node(ASTNode &node, CompSymbolTable &st);
CompNodeResult gen_var_lookup(ASTNode &node, CompSymbolTable &st);

string get_new_label() {
  std::stringstream label;
  label << "L528LAB";
  label << LABELS;
  LABELS++;
  return label.str();
}

void emit(string &s) { std::cout << s; }

void emit(const char *s) { std::cout << s; }

string get_unary_op_method(TokenType op) {
  switch (op) {
  case TokenType::MINUS:
    return "op_umin";
  case TokenType::NOT:
    return "op_unot";
  default:
    break;
  }
  throw std::runtime_error("TokenType argument op must be a unary operator");
}

string get_binary_op_method(TokenType op) {
  switch (op) {
  case TokenType::PLUS:
    return "op_add";
  case TokenType::MINUS:
    return "op_sub";
  case TokenType::TIMES:
    return "op_mul";
  case TokenType::DIV:
    return "op_div";
  case TokenType::MOD:
    return "op_mod";
  case TokenType::EQUALS_EQUALS:
    return "op_eq";
  case TokenType::NOT_EQUALS:
    return "op_neq";
  case TokenType::LESS_EQUALS:
    return "op_leq";
  case TokenType::GREATER_EQUALS:
    return "op_geq";
  case TokenType::LESS:
    return "op_lt";
  case TokenType::GREATER:
    return "op_gt";
  case TokenType::AND:
    return "op_and";
  case TokenType::OR:
    return "op_or";
  default:
    break;
  }
  throw std::runtime_error("TokenType argument op must be a binary operator");
}

CompNodeResult gen_top_level(ASTNode &node, CompSymbolTable &st) {
  emit("#include \"runtime.h\"\n");

  for (auto &child : node.children) {
    gen_node(child, st);
    emit(";\n");
  }

  // encode main entrypoint that goes from C main to program main.
  emit("int main(int argc, char **argv) { \n");
  // global declarations go here
  for (auto &entry : toplevel_decls) {
    auto rhs = entry.second;
    auto rhs_result = gen_node(*rhs, st);
    std::stringstream stmt;
    stmt << entry.first << " = " << rhs_result.result_loc.value() << ";\n";
    auto s = stmt.str();
    emit(s);
  }
  emit("L528_main(make_argv(argc, argv)); \n");
  emit("return 0;\n");
  emit("}\n");

  return CompNodeResult{};
}

CompNodeResult gen_function_declare(ASTNode &node, CompSymbolTable &st) {
  string name = node.data.at("function_name").get<string>();
  vector<string> args = node.data.at("args").get<vector<string>>();
  auto body = node.children[0];

  // implicitly add argv even if it isn't used
  if (name == "main" && args.size() == 0) {
    args.push_back("argv");
  }

  // Make sure name isn't taken
  if (st_lookup_symbol(st, name)) {
    throw std::runtime_error("Function/global name already taken");
  }

  // All program-defined functions get this prefix
  std::string internal_fn_name = "L528_";
  internal_fn_name += name;
  // Add symbol table entry
  st.entries[name] = CompTableEntry{internal_fn_name, CompTableEntryType::FUNC};

  emit("RuntimeObject* ");
  emit(internal_fn_name);
  emit("(");

  // TODO: we'd need to map from args to variable names and store this in the
  // symbol table
  unordered_map<string, CompTableEntry> variables{};
  for (size_t i = 0; i < args.size(); ++i) {
    std::stringstream arg_name_ss;
    arg_name_ss << "arg" << i;
    string arg_name = arg_name_ss.str();
    variables[args[i]] = CompTableEntry{arg_name, CompTableEntryType::VAR};

    std::stringstream arg;
    arg << "RuntimeObject* " << arg_name;
    if (i != args.size() - 1) {
      arg << ", ";
    }
    auto s = arg.str();
    emit(s);
  }
  CompSymbolTable inner_st{
      &st,
      variables,
  };

  emit("){\n");
  auto final = gen_node(body, inner_st);
  if (!final.final_return) {
    emit("return make_nothing();\n");
  }
  emit("}");
  return CompNodeResult{};
}

CompNodeResult gen_block(ASTNode &node, CompSymbolTable &st) {
  auto most_recent_node = NodeType::BLOCK;
  for (auto &child : node.children) {
    most_recent_node = child.type;
    auto result = gen_node(child, st);
    if (result.result_loc.has_value()) {
      emit(result.result_loc.value());
    }
    emit(";\n");
  }

  return CompNodeResult{{}, {}, most_recent_node == NodeType::RETURN};
}

CompNodeResult gen_node_lvalue(ASTNode &node, CompSymbolTable &st) {
  // Lvalue can be either a variable, an index access into a data type (e.g.
  // x[y]) or a field access into a module (or class later)

  if (node.type == NodeType::VAR_LOOKUP) {
    const string identifier = node.data.at("identifier").get<string>();
    auto lookup_result = st_lookup_symbol(st, identifier);
    if (!lookup_result.has_value()) {
      string msg = "Bad var name lookup: ";
      msg += identifier;
      throw std::runtime_error(msg);
    }
    if (lookup_result->type != CompTableEntryType::VAR) {
      string msg = "Unasignable symbol used as assignment lvalue.";
      throw std::runtime_error(msg);
    }
    auto var = lookup_result->location;
    return CompNodeResult{var};
  }

  if (node.type == NodeType::INDEX_ACCESS) {
    const size_t LHS = 0, RHS = 1;

    auto lhs = gen_node(node.children[LHS], st).result_loc.value();
    auto rhs = gen_node(node.children[RHS], st).result_loc.value();
    auto intmdt_id = st_new_intmdt(st);
    std::stringstream lvalue;
    lvalue << "RuntimeObject * " << intmdt_id << " = get_index(" << lhs << ","
           << rhs << ");";
    auto lvalue_str = lvalue.str();
    emit(lvalue_str);
    return CompNodeResult{intmdt_id};
  }

  if (node.type == NodeType::FIELD_ACESS) {
    throw std::runtime_error("Field access lvaue not implemented yet");
  }

  throw std::runtime_error("Invalid nodetype for lvalue, must be var lookup, "
                           "index access, or field access.");
}

CompNodeResult gen_index_access(ASTNode &node, CompSymbolTable &st) {
  const size_t LHS = 0, RHS = 1;

  auto lhs = gen_node(node.children[LHS], st).result_loc.value();
  auto rhs = gen_node(node.children[RHS], st).result_loc.value();
  std::stringstream result;
  result << "get_index(" << lhs << "," << rhs << ")";
  auto result_str = result.str();
  return CompNodeResult{result_str};
}

CompNodeResult gen_field_access(ASTNode &node, CompSymbolTable &st) {
  const size_t LHS = 0, RHS = 1;
  auto lhs = gen_node(node.children[LHS], st).result_loc.value();

  auto rhs_node = node.children[RHS];
  assert(rhs_node.type == NodeType::VAR_LOOKUP);
  string identifier = rhs_node.data.at("identifier").get<string>();

  std::stringstream result;
  result << "field_access(" << lhs << ", \"" << identifier << "\")";
  auto result_str = result.str();
  return CompNodeResult{result_str, lhs};
}

CompNodeResult gen_assign_op(ASTNode &node, CompSymbolTable &st) {
  const size_t LHS = 0, RHS = 1;
  const string op_key = "op";
  auto op = int_to_token_type(node.data.at(op_key).get<int>());
  auto lhs = gen_node_lvalue(node.children[LHS], st).result_loc.value();
  auto rhs = gen_node(node.children[RHS], st).result_loc.value();

  auto new_value = rhs;

  if (op != TokenType::EQUALS) {
    auto bin_op = assign_op_to_binary_op(op);
    auto op_method = get_binary_op_method(bin_op);
    std::stringstream new_value_ss;
    new_value_ss << op_method << "(" << lhs << "," << rhs << ")";
    new_value = new_value_ss.str();
  }

  std::stringstream assign_statement_ss;
  assign_statement_ss << lhs << "=" << new_value;
  auto assign_statement = assign_statement_ss.str();
  emit(assign_statement);
  return CompNodeResult{};
}

CompNodeResult gen_var_declare(ASTNode &node, CompSymbolTable &st) {
  string identifier = node.data.at("identifier").get<string>();
  bool is_const = node.data.at("const").get<bool>();
  bool is_toplevel = is_toplevel_st(st);
  auto type = is_const ? CompTableEntryType::CONST : CompTableEntryType::VAR;

  if (st.entries.contains(identifier)) {
    throw std::runtime_error("Variable name already taken in scope.");
  }
  std::stringstream local_id;
  local_id << "local" << LOCALS;
  LOCALS++;
  string local_id_str = local_id.str();
  st.entries[identifier] = CompTableEntry{local_id_str, type};

  std::stringstream declare_stmt;
  declare_stmt << "RuntimeObject* " << local_id_str;

  // top level declarations get declared globally, but aren't initialized until
  // the main method.
  if (is_toplevel) {
    declare_stmt << ";\n";
    auto s = declare_stmt.str();
    emit(s);

    toplevel_decls.push_back({local_id_str, &node.children[0]});
    return CompNodeResult{};
  }

  // eval rhs
  auto rhs = node.children[0];
  auto rhs_result = gen_node(rhs, st);

  declare_stmt << " = " << rhs_result.result_loc.value();
  auto s = declare_stmt.str();
  emit(s);
  return CompNodeResult{};
}

CompNodeResult gen_bool_literal(ASTNode &node, CompSymbolTable &st) {
  auto value = node.data.at("value").get<bool>();
  std::stringstream ss;
  ss << "make_bool(" << value << ")";
  auto s = ss.str();
  return CompNodeResult{s};
}

CompNodeResult gen_int_literal(ASTNode &node, CompSymbolTable &st) {
  auto value = node.data.at("value").get<int>();
  std::stringstream ss;
  ss << "make_int(" << value << ")";
  auto s = ss.str();
  return CompNodeResult{s};
}

CompNodeResult gen_float_literal(ASTNode &node, CompSymbolTable &st) {
  auto value = node.data.at("value").get<float>();
  std::stringstream ss;
  ss << "make_float(" << value << ")";
  auto s = ss.str();
  return CompNodeResult{s};
}

CompNodeResult gen_string_literal(ASTNode &node, CompSymbolTable &st) {
  auto value = node.data.at("value").get<string>();
  std::stringstream ss;
  ss << "make_string(\"" << value << "\")";
  auto s = ss.str();
  //   emit(s);
  return CompNodeResult{s};
}

CompNodeResult gen_nothing_literal(ASTNode &node, CompSymbolTable &st) {
  return CompNodeResult{"make_nothing()"};
}

CompNodeResult gen_function_call(ASTNode &node, CompSymbolTable &st) {
  const size_t FUNCTION = 0, ARGS = 1;
  auto lhs = node.children[FUNCTION];
  auto rhs = node.children[ARGS];

  //
  // I guess we'd call this a "known" function call.
  // These are function calls referred to by the name
  // that we know in the symbol table at compile-time.
  //
  if (lhs.type == NodeType::VAR_LOOKUP) {
    // build function name
    auto identifier = lhs.data.at("identifier").get<string>();
    auto lookup_result = st_lookup_symbol(st, identifier);
    if (!lookup_result.has_value()) {
      std::string msg = "Bad function name lookup: ";
      msg += identifier;
      throw std::runtime_error(msg);
    }
    auto lookup_value = lookup_result.value();
    if (!(lookup_value.type == CompTableEntryType::BUILTIN ||
          lookup_value.type == CompTableEntryType::FUNC)) {
      std::string msg = identifier + " is not a function or builtin.";
      throw std::runtime_error(msg);
    }
    auto fn_name = lookup_value.location;

    // TODO: some kind of arg matching?
    auto rhs_result = gen_node(rhs, st);
    auto result = fn_name + "(" + rhs_result.result_loc.value() + ")";
    return CompNodeResult{result};
  }
  // Dynamic function call where the function value itself is known only
  // at run-time.
  else {
    /*
        lhs contains a node which should evaluate to a dynamic function
        rhs

    */
    auto lhs_result = gen_node(lhs, st);
    auto rhs_result = gen_node(rhs, st);

    auto fn_loc = lhs_result.result_loc.value();
    auto argc = rhs_result.argc;
    auto argv_intmdt = st_new_intmdt(st);
    auto argv_arglist = rhs_result.result_loc.value();

    // for x.y(), need to pass x as an implicit first parameter
    if (lhs.type == NodeType::FIELD_ACESS) {
      auto new_argv = lhs_result.accessee_loc.value();
      if (argc == 0) {
        argv_arglist = new_argv;
      } else {
        argv_arglist = new_argv + "," + argv_arglist;
      }
      argc++;
    }

    std::stringstream argv;
    argv << "RuntimeObject* " << argv_intmdt << "[] = {" << argv_arglist
         << "};\n";
    auto argv_str = argv.str();
    emit(argv_str);
    std::stringstream result;
    result << "dynamic_function_call(" << fn_loc << "," << argc << ","
           << argv_intmdt << ")";
    return CompNodeResult{result.str()};
  }
}

CompNodeResult gen_var_lookup(ASTNode &node, CompSymbolTable &st) {
  const string identifier = node.data.at("identifier").get<string>();
  auto lookup_result = st_lookup_symbol(st, identifier);
  if (!lookup_result.has_value()) {
    std::string msg = "Bad var name lookup: ";
    msg += identifier;
    throw std::runtime_error(msg);
  }
  auto var = lookup_result->location;
  return CompNodeResult{var};
}

CompNodeResult gen_expr_list(ASTNode &node, CompSymbolTable &st) {
  vector<string> results;
  for (auto &node : node.children) {
    auto result = gen_node(node, st);
    results.push_back(result.result_loc.value());
  }
  std::stringstream expr_list;
  for (size_t i = 0; i < results.size(); ++i) {
    expr_list << results[i];
    if (i != results.size() - 1) {
      expr_list << ',';
    }
  }

  return CompNodeResult{expr_list.str(), {}, false, results.size()};
}

CompNodeResult gen_vec_literal(ASTNode &node, CompSymbolTable &st) {
  vector<string> results;
  for (auto &node : node.children) {
    auto result = gen_node(node, st);
    results.push_back(result.result_loc.value());
  }

  auto intmdt_id = st_new_intmdt(st);

  std::stringstream decl;
  decl << "RuntimeObject * " << intmdt_id << " = "
       << "make_vector_known_size(" << results.size() << ");\n";
  auto decl_str = decl.str();

  emit(decl_str);

  size_t i = 0;
  for (auto &result : results) {
    std::stringstream stmt;
    stmt << intmdt_id << "->value.v_vec->contents[" << i << "] = *" << result
         << ";\n";

    auto stmt_str = stmt.str();
    emit(stmt_str);
    ++i;
  }

  return CompNodeResult{intmdt_id};
}

CompNodeResult gen_binary_op(ASTNode &node, CompSymbolTable &st) {
  const size_t LHS = 0, RHS = 1;
  const string op_key = "op";
  auto op = int_to_token_type(node.data.at(op_key).get<int>());

  std::stringstream intmdt;
  intmdt << "_intmdt" << st.intermediates;
  st.intermediates++;
  auto lhs = gen_node(node.children[LHS], st);
  auto rhs = gen_node(node.children[RHS], st);

  auto op_method = get_binary_op_method(op);
  auto intmdt_str = intmdt.str();
  emit("RuntimeObject* ");
  emit(intmdt_str);
  emit(" = ");
  std::stringstream add;
  add << op_method << "(" << lhs.result_loc.value() << ", "
      << rhs.result_loc.value() << ");\n";
  auto s = add.str();
  emit(s);

  return CompNodeResult{intmdt_str};
}

CompNodeResult gen_unary_op(ASTNode &node, CompSymbolTable &st) {
  const size_t RHS = 0;
  const string op_key = "op";
  auto op = int_to_token_type(node.data.at(op_key).get<int>());
  auto rhs = gen_node(node.children[RHS], st);
  auto op_method = get_unary_op_method(op);

  // NOTE intermediates are probably unneccesary, but are the right pattern to
  // use if we were going to convert this to generating 3-address code later on.
  std::stringstream intmdt;
  intmdt << "_intmdt" << st.intermediates;
  st.intermediates++;
  auto intmdt_str = intmdt.str();
  emit("RuntimeObject* ");
  emit(intmdt_str);
  emit(" = ");
  std::stringstream expr;
  expr << op_method << "(" << rhs.result_loc.value() << ");\n";
  auto s = expr.str();
  emit(s);

  return CompNodeResult{intmdt_str};
}

CompNodeResult gen_return(ASTNode &node, CompSymbolTable &st) {
  auto return_value_expr = node.children.at(0);
  auto result = gen_node(return_value_expr, st);
  emit("return ");
  emit(result.result_loc.value());
  emit(";\n");
  return CompNodeResult{{}, {}, true};
}

CompNodeResult gen_if(ASTNode &node, CompSymbolTable &st) {
  const size_t CONDITION = 0, IF_BODY = 1, ELSE_BODY = 2, SIZE_IF_ELSE = 3;

  // need test method runtime result?
  auto condition_result = gen_node(node.children[CONDITION], st);
  emit("if (get_conditional_result(");
  emit(condition_result.result_loc.value());
  emit(")) {\n");
  CompSymbolTable if_block_st{&st, {}};
  gen_node(node.children[IF_BODY], if_block_st);
  emit("}\n");

  // an if-statement may or may not have an else
  if (node.children.size() == SIZE_IF_ELSE) {
    // could be another if (e.g., else if), or just a block
    auto else_node = node.children[ELSE_BODY];
    emit("else {\n");
    CompSymbolTable else_block_st{&st, {}};

    // either way, we compile to a normal else-block because we need to be in
    // a block to compile and execute the else-if statement expression.
    gen_node(else_node, else_block_st);
    emit("}\n");
  }

  return CompNodeResult{};
}

CompNodeResult gen_while(ASTNode &node, CompSymbolTable &st) {
  const size_t CONDITION = 0, BODY = 1;
  auto condition_label = get_new_label();
  emit(condition_label);
  emit(":;\n"); // semicolon due to compiler error
                // https://github.com/llvm/llvm-project/issues/77057
  auto condition_result = gen_node(node.children[CONDITION], st);
  emit("if (get_conditional_result(");
  emit(condition_result.result_loc.value());
  emit(")) {\n");
  CompSymbolTable while_body_st{&st, {}};
  gen_node(node.children[BODY], while_body_st);
  emit("goto ");
  emit(condition_label);
  emit(";\n}\n");
  return CompNodeResult{};
}

CompNodeResult gen_node(ASTNode &node, CompSymbolTable &st) {
  //   std::cerr << "right here : " << node_type_to_string(node.type) << ", "
  //             << "line: " << node.metadata.line
  //             << ", column: " << node.metadata.column << " \n";

  switch (node.type) {
  case NodeType::TOP_LEVEL:
    return gen_top_level(node, st);
    break;
  case NodeType::BLOCK:
    return gen_block(node, st);
    break;
  case NodeType::ASSIGN_OP:
    return gen_assign_op(node, st);
    break;
  case NodeType::VAR_DECLARE:
    return gen_var_declare(node, st);
    break;
  case NodeType::FUNC_DECLARE:
    return gen_function_declare(node, st);
    break;
  // case NodeType::MODULE_IMPORT:
  //   return eval_module_import(node, st);
  //   break;
  case NodeType::IF:
    return gen_if(node, st);
    break;
  case NodeType::RETURN:
    return gen_return(node, st);
    break;
  case NodeType::WHILE:
    return gen_while(node, st);
    break;
  case NodeType::BINARY_OP:
    return gen_binary_op(node, st);
    break;
  case NodeType::UNARY_OP:
    return gen_unary_op(node, st);
    break;
  case NodeType::FUNC_CALL:
    return gen_function_call(node, st);
    break;
  case NodeType::INDEX_ACCESS:
    return gen_index_access(node, st);
    break;
  case NodeType::FIELD_ACESS:
    return gen_field_access(node, st);
    break;
  case NodeType::VAR_LOOKUP:
    return gen_var_lookup(node, st);
    break;
  case NodeType::EXPR_LIST:
    return gen_expr_list(node, st);
    break;
  case NodeType::VEC_LITERAL:
    return gen_vec_literal(node, st);
    break;
  // case NodeType::DICT_LITERAL:
  //   return eval_dict_literal(node, st);
  //   break;
  case NodeType::BOOL_LITERAL:
    return gen_bool_literal(node, st);
    break;
  case NodeType::INT_LITERAL:
    return gen_int_literal(node, st);
    break;
  case NodeType::FLOAT_LITERAL:
    return gen_float_literal(node, st);
    break;
  case NodeType::STRING_LITERAL:
    return gen_string_literal(node, st);
    break;
  case NodeType::NOTHING_LITERAL:
    return gen_nothing_literal(node, st);
    break;

  default:
    throw std::runtime_error("Not implemented: " +
                             node_type_to_string(node.type));
  }
}

CompNodeResult gen_node_root(ASTNode &node) {
  CompSymbolTable root_symbol_table{
      nullptr, {{"print", {"builtin_print", CompTableEntryType::BUILTIN}}}};
  return gen_node(node, root_symbol_table);
}
