#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "astnode.h"
#include "nodetype.h"
#include "runtime.h"
#include "interpreter.h"
#include "tokentype.h"
#include "util.h"

using std::string;
using std::vector;
using std::unordered_map;

static string WORKING_DIRECTORY = std::getenv("PWD");

static unordered_map<DataType, SymbolTable> builtin_type_methods {
  {DataType::VECTOR, 
    {nullptr, {}, 
      {
        {"length", SymbolTableEntry {
        VarType::FUNCTION,
        std::make_shared<BoxedValue>(
          DataType::FUNCTION,
          Function {
            "length", 
            {}, 
            ASTNode {NodeType::BUILTIN_VECTOR_LENGTH, {}, {}, {}}
          })
        }},
        {"append", SymbolTableEntry {
        VarType::FUNCTION,
        std::make_shared<BoxedValue>(
          DataType::FUNCTION,
          Function {
            "append", 
            {"elem"}, 
            ASTNode {NodeType::BUILTIN_VECTOR_APPEND, {}, {}, {}}
          })
        }}
      }
    }
  },
  {DataType::STRING, 
    {nullptr, {}, 
      {{"length", SymbolTableEntry {
        VarType::FUNCTION,
        std::make_shared<BoxedValue>(
          DataType::FUNCTION,
          Function {
            "length", 
            {}, 
            ASTNode {NodeType::BUILTIN_STRING_LENGTH, {}, {}, {}}
          })
      }}}
    }
  },
  {DataType::DICT, 
    {nullptr, {},  
      {
        // length
        {"length", SymbolTableEntry {
          VarType::FUNCTION,
          std::make_shared<BoxedValue>(
            DataType::FUNCTION,
            Function {
              "length", 
              {}, 
              ASTNode {NodeType::BUILTIN_DICT_LENGTH, {}, {}, {}}
          })
        }},
        // keys
        {"keys", SymbolTableEntry {
          VarType::FUNCTION,
          std::make_shared<BoxedValue>(
            DataType::FUNCTION,
            Function {
              "keys", 
              {}, 
              ASTNode {NodeType::BUILTIN_DICT_KEYS, {}, {}, {}}
          })
        }},
        // contains
        {"contains", SymbolTableEntry {
          VarType::FUNCTION,
          std::make_shared<BoxedValue>(
            DataType::FUNCTION,
            Function {
              "contains", 
              {"key"}, 
              ASTNode {NodeType::BUILTIN_DICT_CONTAINS, {}, {}, {}}
          })
        }}
      },
    }
  }
};

EvalResult eval_node(ASTNode &node, SymbolTable &st, ValueType vt=ValueType::RVALUE);

EvalResult eval_var_declare(ASTNode &node, SymbolTable &st) {
  /*
    What is the var's id?
    Is it const or var?
    Does the var exist in the current symbol table?
  */

  string identifier = node.data.at("identifier").get<string>();
  bool is_const = node.data.at("const").get<bool>();

  // make sure we don't declare more than once in the same scope
  runtime_assertion(
    !st.entries.contains(identifier), 
    "Identifiers cannot be redefined in the same scope." 
  );

  // eval the right hand side of the expression (first and only child)
  auto rhs = node.children[0];
  auto result = eval_node(rhs,  st);
  auto value = std::make_shared<BoxedValue>(result.rv_result.value());

  st.entries[identifier] = SymbolTableEntry {
    is_const ? VarType::CONST : VarType::VAR,
    value
  };

  return EvalResult {};
}

EvalResult eval_module_import(ASTNode &node, SymbolTable &st) {
  /**
    TODO: should non-named imports just get evaluated directly with the passed 
    symbol table's context? Is there even any need to make a module symbol table
    in this case? Think about this question and refactor this method if so.

    No, unless we have a way for the interpreter to know we're in a module context
    so that main() can be skipped.
  */

  string module_path = node.data.at("module_path").get<string>();

  string path = WORKING_DIRECTORY + "/" + module_path;
  
  // read the file at path if it exists, load its contents as AST
  auto module_nodes = UTIL::load_module(path);
  
  // AST root should always be TOP_LEVEL
  runtime_assertion(
    module_nodes.type == NodeType::TOP_LEVEL, 
    "Malformed module: " + path
  );

  // this is still a hack, but works for now
  SymbolTable t {&st, {}, {}};
  auto module_st = std::make_shared<SymbolTable>(t);
  st.module_symbol_tables.push_back(module_st);

  // interpret every node in the AST
  for (auto &child : module_nodes.children) {
      eval_node(child, *module_st);
  }

  // if this is a named import, create a module object and place it in the symbol table
  if ( node.data.contains("module_name") ) {
    auto module_name = node.data.at("module_name").get<string>();
    st.entries[module_name] = SymbolTableEntry {
      VarType::CONST,
      std::make_shared<BoxedValue>(
        DataType::MODULE,
        Module {
          module_name,
          module_st
        }
      )
    };
  }
  // otherwise merge all symbol table entries except for main
  else {
    for ( const auto& [ id, entry ] : module_st->entries ) {
      if(id != "main") {
        st.entries[id] = entry;
      }
    }
  }

  return EvalResult {};
}

EvalResult eval_func_declare(ASTNode &node, SymbolTable &st) {
  string name = node.data.at("function_name").get<string>();
  vector<string> args = node.data.at("args").get<vector<string>>();
  auto body = node.children[0];

  runtime_assertion(
    !st.entries.contains(name), 
    "Function/global name already taken" 
  );


  st.entries[name] = SymbolTableEntry {
    VarType::FUNCTION,
      std::make_shared<BoxedValue>(
      DataType::FUNCTION,
      Function {
        name, args, body  
      }  
  )
  };

  return EvalResult {};
}

EvalResult eval_assign_op(ASTNode &node, SymbolTable &st) {
  /*
    several things to do:
      1. determine lvalue - can be either a variable in the SymbolTable/SymbolTable,
         and object field access, or an index in another data structure (e.g.) a vector

      2. determine RHS, e.g. evaluate the expression to the right of the assign operator

      3. if the assign operator is +=,*=,..., apply that operation on the current value

      4. set the new value in the appropriate place

  */
  const size_t LHS = 0, RHS = 1;
  const string op_key = "op";
  auto op = int_to_token_type(node.data.at(op_key).get<int>());
  auto lhs = eval_node(node.children[LHS], st, ValueType::LVALUE).lv_result;
  auto rhs = eval_node(node.children[RHS], st).rv_result.value();

  auto new_value = rhs;

  if (op != TokenType::EQUALS) {
    auto bin_op = assign_op_to_binary_op(op);
    new_value = apply_binary_operator(bin_op, lhs->currentValue(),  new_value);
  }

  lhs->assign(new_value);
  return EvalResult {};
}

EvalResult eval_binary_op(ASTNode &node, SymbolTable &st) {
  const size_t LHS = 0, RHS = 1;
  const string op_key = "op";
  auto op = int_to_token_type(node.data.at(op_key).get<int>());
  auto lhs = eval_node(node.children[LHS], st).rv_result.value();
  auto rhs = eval_node(node.children[RHS], st).rv_result.value();

  return EvalResult {
    apply_binary_operator(op, lhs, rhs)
  };
}

EvalResult eval_unary_op(ASTNode &node, SymbolTable &st) {
  const size_t RHS = 0;
  const string op_key = "op";
  auto op = int_to_token_type(node.data.at(op_key).get<int>());
  auto rhs = eval_node(node.children[RHS], st).rv_result.value();

  return EvalResult {
    apply_unary_operator(op, rhs)
  };
}

EvalResult eval_func_call(ASTNode &node, SymbolTable &st) {
  /*
    return ASTNode{
      NodeType::FUNC_CALL, {lvalue_expr, arg_expr_list}, {}, metadata};
  */

  const size_t FUNCTION = 0, ARGS = 1;

  auto callee = eval_node(node.children[FUNCTION], st).rv_result.value();
  runtime_assertion(
    callee.type == DataType::FUNCTION, 
    "Function callee value must be a function."
  );

  auto args = eval_node(node.children[ARGS], st).rv_result.value();
  runtime_assertion(
    args.type == DataType::VECTOR,
    
    "Function arguments must be an expression list"
  );

  auto function_rawvalue = std::get<Function>(callee.value);
  auto arg_names = function_rawvalue.args;

  auto arg_values = std::get<shared_ptr<HeVec>>(args.value);

  // prep function's symbol table
  SymbolTable fn_st;
  if (function_rawvalue.module_st == nullptr) {
    fn_st = SymbolTable {&st,{}};
  }
  else {
    fn_st = SymbolTable{ function_rawvalue.module_st.get(), {} };
  }

  runtime_assertion(
    arg_names.size() == arg_values->size(),
    "Number of arguments does not match function definition."
  );
  auto len = arg_names.size();
  for (size_t i = 0; i < len; i++) {
    auto arg_name = arg_names.at(i);
    auto arg_value = arg_values->at(i);

    fn_st.entries[arg_name] = SymbolTableEntry {
      VarType::CONST,
      arg_value
    };
  }

  // bit of a hack to propagate "this" into functions that call for it
  if (function_rawvalue._this != nullptr) {
    fn_st.entries["this"] = SymbolTableEntry {
      VarType::CONST,
      function_rawvalue._this
    };
  }
  
  auto result = eval_node(function_rawvalue.body, fn_st);
  return EvalResult {result.rv_result, result.lv_result};
}

EvalResult eval_vec_literal(ASTNode &node, SymbolTable &st) {
    auto child_nodes = node.children;
    auto vec_value = std::make_shared<HeVec>();
    for (auto &node : child_nodes) {
        auto result = eval_node(node, st);
        vec_value->push_back(std::make_shared<BoxedValue>(result.rv_result.value()));
    }
    return EvalResult {
        BoxedValue {
            DataType::VECTOR,
            vec_value
        }
    };
}

EvalResult eval_dict_literal(ASTNode &node, SymbolTable &st) {
    auto child_nodes = node.children;
    auto dict_value = std::make_shared<Dict>();

    // iterate two at a time to simulate pairs
    // {a: b, c: d} -> [a, b, c, d]
    size_t v_index = 1;
    for (size_t v_index = 1; v_index < child_nodes.size(); v_index += 2) {
      size_t k_index = v_index - 1;
      auto key = eval_node( child_nodes[k_index], st ).rv_result.value();
      auto key_str = getDictKey(key);
      auto value = std::make_shared<BoxedValue>(
        eval_node( child_nodes[v_index], st ).rv_result.value()
      );

      dict_value->operator[](key_str) = std::make_pair(key, value);
    }

    return EvalResult {
        BoxedValue {
            DataType::DICT,
            dict_value
        }
    };
}

EvalResult eval_string_literal(ASTNode &node, SymbolTable &st) {
    auto value = node.data.at("value").get<string>();
    return EvalResult {
        BoxedValue {
            DataType::STRING,
            value
        }
    };
}

EvalResult eval_float_literal(ASTNode &node, SymbolTable &st) {
    auto value = node.data.at("value").get<float>();
    return EvalResult {
        BoxedValue {
            DataType::FLOAT,
            value
        }
    };
}

EvalResult eval_int_literal(ASTNode &node, SymbolTable &st) {
    auto value = node.data.at("value").get<int>();
    return EvalResult {
        BoxedValue {
            DataType::INT,
            value
        }
    };
}

EvalResult eval_bool_literal(ASTNode &node, SymbolTable &st) {
    auto value = node.data.at("value").get<bool>();
    return EvalResult {
        BoxedValue {
            DataType::BOOL,
            value
        }
    };
}

EvalResult eval_block(ASTNode &node, SymbolTable &st) {
    for (auto &child : node.children) {
        auto result = eval_node(child, st);
        if (result.returned) {
          return result;
        }
    }
    return EvalResult {};
}

EvalResult eval_while(ASTNode &node, SymbolTable &st) {
  const size_t CONDITION = 0, BODY = 1;

CHECK_CONDITION:
  bool raw_condition_value;
  EvalResult result;
  BoxedValue condition_value = eval_node(node.children[CONDITION], st).rv_result.value();
  runtime_assertion(
    condition_value.type == DataType::BOOL,
    "While loop condition expression must have boolean result."
  );
  raw_condition_value = std::get<bool>(condition_value.value);

  if (raw_condition_value) {
    SymbolTable block_st {&st, {}};
    result = eval_node(node.children[BODY], block_st);

    // exit early and propate return value if we returned from block
    if (result.returned) {
      return result;
    }
    goto CHECK_CONDITION;
  }

  return EvalResult {};
}

EvalResult eval_return(ASTNode &node, SymbolTable &st) {
  auto return_value_expr = node.children.at(0);
  auto return_value = eval_node(return_value_expr, st);

  // signals to any blocks/functions that a return statement has been encountered.
  return_value.returned = true;

  return return_value;
}


EvalResult eval_if(ASTNode &node, SymbolTable &st) {
  /*
      node.children[0] => conditional
      node.children[1] => if-body
      node.children[2] => ?else-body
  */

  const size_t CONDITION = 0, IF_BODY = 1, ELSE_BODY = 2, SIZE_IF_ELSE = 3;

  auto condition = eval_node(node.children[CONDITION], st).rv_result.value();
  runtime_assertion(
    condition.type == DataType::BOOL,
    "If-statement condition expression must have boolean result."
  );
  auto conditional_result = std::get<bool>(condition.value);

  if (conditional_result) {
    SymbolTable if_block_st {&st, {}};
    return eval_node(node.children[IF_BODY], if_block_st);
  }
  else if (node.children.size() == SIZE_IF_ELSE) {
    SymbolTable else_block_st {&st, {}};
    return eval_node(node.children[ELSE_BODY], else_block_st);
  }
  return EvalResult {};
}

EvalResult eval_var_lookup(ASTNode &node, SymbolTable &st, ValueType vt) {
  const string identifier_key = "identifier";
  const string identifier = node.data.at(identifier_key).get<string>();

  if (vt == ValueType::LVALUE) {
    return st.lookup_lvalue(identifier);
  }
  return st.lookup_rvalue(identifier);
}

EvalResult SymbolTable::lookup_lvalue(string var) {
  if (this->entries.contains(var)) {
    auto st_entry = this->entries.at(var);
    runtime_assertion(
      st_entry.type == VarType::VAR, 
      "Assignment is not supported on constants or functions."
    );
    return EvalResult { 
      std::nullopt, 
      std::make_shared<VariableLV>(this, var)
    };
  }

  if (this->parent != nullptr) {
    return this->parent->lookup_lvalue(var);
  }
  
  std::stringstream err;
  err << "Lookup of identifier '" << var << "' failed";
  throw std::runtime_error(err.str());
}


EvalResult SymbolTable::lookup_rvalue(string var) {
  if (this->entries.contains(var)) {
    auto st_entry = this->entries.at(var);
    return EvalResult { BoxedValue {st_entry.value->type, st_entry.value->value} };
  }

  // todo: make this iterative instead of recursive
  if (this->parent != nullptr) {
    return this->parent->lookup_rvalue(var);
  }

  std::stringstream err;
  err << "Lookup of identifier '" << var << "' failed";
  throw std::runtime_error(err.str());
}

EvalResult eval_builtin_print(ASTNode &node, SymbolTable &st) {
  auto lookup_er = st.lookup_rvalue("arg");
  builtin_print(lookup_er.rv_result.value());
  return EvalResult {};
}

EvalResult eval_builtin_vector_append(ASTNode &node, SymbolTable &st) {
  auto lookup_this = st.lookup_rvalue("this").rv_result.value();
  auto lookup_elem = st.lookup_rvalue("elem").rv_result.value();

  builtin_vector_append(lookup_this, lookup_elem);
  return EvalResult {};
}

EvalResult eval_builtin_vector_length(ASTNode &node, SymbolTable &st) {
  auto lookup_er = st.lookup_rvalue("this");
  return EvalResult {
    builtin_vector_length(lookup_er.rv_result.value()),
    nullptr
  };
}

EvalResult eval_builtin_dict_length(ASTNode &node, SymbolTable &st) {
  auto lookup_er = st.lookup_rvalue("this");
  return EvalResult {
    builtin_dict_length(lookup_er.rv_result.value()),
    nullptr
  };
}

EvalResult eval_builtin_dict_keys(ASTNode &node, SymbolTable &st) {
  auto lookup_er = st.lookup_rvalue("this");
  return EvalResult {
    builtin_dict_keys(lookup_er.rv_result.value()),
    nullptr
  };
}

EvalResult eval_builtin_dict_contains(ASTNode &node, SymbolTable &st) {
  auto lookup_er = st.lookup_rvalue("this");
  auto lookup_key = st.lookup_rvalue("key").rv_result.value();
  return EvalResult {
    builtin_dict_contains(lookup_er.rv_result.value(), lookup_key),
    nullptr
  };
}

EvalResult eval_builtin_string_length(ASTNode &node, SymbolTable &st) {
  auto lookup_er = st.lookup_rvalue("this");
  return EvalResult {
    builtin_string_length(lookup_er.rv_result.value()),
    nullptr
  };
}

EvalResult eval_field_access(ASTNode &node, SymbolTable &st, ValueType vt) {
  const size_t LHS = 0, RHS = 1;
  auto lhs = eval_node(node.children[LHS], st).rv_result.value();

  auto field = node.children[RHS];

  runtime_assertion(
    field.type == NodeType::VAR_LOOKUP,
    "Field access requires a identifier"
  );
  
  const string identifier_key = "identifier";
  const string identifier = field.data.at(identifier_key).get<string>();

  bool is_module = false;
  SymbolTable* lhs_symbol_table;

  if (lhs.type == DataType::MODULE) {
    is_module = true;
    lhs_symbol_table = std::get<Module>(lhs.value).symbol_table.get();
  }
  else {
    lhs_symbol_table = &builtin_type_methods.at(lhs.type);
  }


  if (vt == ValueType::RVALUE) {
    auto result = lhs_symbol_table->lookup_rvalue(identifier);
    if (result.rv_result.value().type == DataType::FUNCTION) {

      // inject "this" so the built in functions work like object instance methods
      std::get<Function>(result.rv_result.value().value)._this = std::make_shared<BoxedValue>(
        lhs.type,
        lhs.value
      );

      // inject the module symbol table so variable lookups inside them work correctly
      if(is_module) {
        std::get<Function>(result.rv_result.value().value).module_st = 
          std::get<Module>(lhs.value).symbol_table;
      }
    }
    return result;
  }
  return lhs_symbol_table->lookup_lvalue(identifier);
}

EvalResult eval_index_access(ASTNode &node, SymbolTable &st, ValueType vt) {
  // Handles vectors (for lvalue and rvalue) and strings (rvalue only).
  // Later down the road there may be a builtin dict/hash type that will also need
  // to be handled.

  const size_t LHS = 0, RHS = 1;

  auto lhs = eval_node(node.children[LHS], st).rv_result.value();
  auto rhs =  eval_node(node.children[RHS], st).rv_result.value();


  // dict index case
  if (lhs.type == DataType::DICT) {
    auto dict = std::get<shared_ptr<Dict>>(lhs.value);


    if(vt == ValueType::LVALUE) {
      return EvalResult {
        std::nullopt,
        std::make_shared<DictIndexLV>(
          dict,
          rhs
        )
      };
    }

    auto key = getDictKey(rhs);
    auto kv_pair = dict->at(key);
    return EvalResult {
      BoxedValue {
        kv_pair.second->type,
        kv_pair.second->value
      }
    };
  }

  // arrays and strings only support integer indexes
  runtime_assertion(rhs.type == DataType::INT, "Index value must be an int.");
  auto index = std::get<int>(rhs.value);


  // vector index case
  if (lhs.type == DataType::VECTOR) {
    auto hevec = std::get<shared_ptr<HeVec>>(lhs.value);

    if(vt == ValueType::LVALUE) {
      return EvalResult {
        std::nullopt,
        std::make_shared<VectorIndexLV>(
          hevec,
          BoxedValue {DataType::INT, index}
        )
      };
    }

    auto value = hevec->at(index);
    return EvalResult {
      BoxedValue {
        value->type,
        value->value
      }
    };
  }

  // string index case
  if (lhs.type == DataType::STRING) {
    auto str = std::get<string>(lhs.value);
    if(vt == ValueType::LVALUE) {
      throw std::runtime_error("Assignment is not supported on string indexes.");
    }
    string value { str.at(index) };
    return EvalResult {
      BoxedValue {
        DataType::STRING,
        value
      }
    };
  }

  throw std::runtime_error("Index access only supported on strings, vectors, and dictionaries.");
}

void VariableLV::assign(BoxedValue value) {
  auto id = this->identifier;

  this->symbol_table->entries.at(id) = SymbolTableEntry {
    VarType::VAR,
    std::make_shared<BoxedValue>(value.type, value.value)
  };
}

BoxedValue VariableLV::currentValue() {
  auto id = this->identifier;
  auto cv = this->symbol_table->entries.at(id).value;
  return BoxedValue {
    cv->type, cv->value
  };
}

void VectorIndexLV::assign(BoxedValue value) {
  auto index = std::get<int>(this->index.value);
  this->vector->at(index) = std::make_shared<BoxedValue>(
    value.type,
    value.value
  );
}

BoxedValue VectorIndexLV::currentValue() {
  auto index = std::get<int>(this->index.value);
  auto cv = this->vector->at(index);
  return BoxedValue {
    cv->type, cv->value
  };
}

void DictIndexLV::assign(BoxedValue value) {
  auto key = getDictKey(this->key);

  this->dict->operator[](key).second = std::make_shared<BoxedValue>(
    value.type,
    value.value
  );
}

BoxedValue DictIndexLV::currentValue() {
  auto key = getDictKey(this->key);
  auto kv_pair = this->dict->at(key);
  auto current_value = kv_pair.second;
  return BoxedValue {
    current_value->type, current_value->value
  };
}

EvalResult call_main_function(Function main_function, vector<string> argv, SymbolTable &st) {
  SymbolTable main_function_st {&st, {}, {}};

  // vector.contains does not exist, but this line noise does the same thing
  if ( std::find(main_function.args.begin(), main_function.args.end(), "argv")
        != main_function.args.end() )
  {
    // make vector of strings out of argv
    auto argv_hevec = std::make_shared<HeVec>();
    for(auto &str : argv) {
      argv_hevec->push_back(
        std::make_shared<BoxedValue>(
          DataType::STRING,
          str
        )
      );
    }

    // place the newly created argv into the symbol table
    main_function_st.entries["argv"] = SymbolTableEntry {
      VarType::CONST,
      std::make_shared<BoxedValue>(
        DataType::VECTOR,
        argv_hevec
      )
    };
  }

  return eval_node(main_function.body, main_function_st);
}

EvalResult eval_top_level(ASTNode &node, string module_wd, vector<string> argv) {
  WORKING_DIRECTORY = module_wd;
  return eval_top_level(node, argv);
}

EvalResult eval_top_level(ASTNode &node, vector<string> argv) {
    assert(node.type == NodeType::TOP_LEVEL);

    SymbolTable top_level_st;

    // hard code a built-in function for print
    top_level_st.entries["print"] = SymbolTableEntry {
      VarType::FUNCTION,
      std::make_shared<BoxedValue>(
        DataType::FUNCTION,
        Function {
          "print", 
          {"arg"}, 
          ASTNode {NodeType::BUILTIN_PRINT, {}, {}, {}}
        }
      )
    };

    for (auto &child : node.children) {
        eval_node(child, top_level_st);
    }

    if(top_level_st.entries.contains("main")) {
        auto main = top_level_st.entries.at("main");
        if(main.type == VarType::FUNCTION) {
            // actually call main
            auto main_function = std::get<Function>(main.value->value);
            return call_main_function(main_function, argv, top_level_st);
        }
    }

    return EvalResult {};
}

EvalResult eval_node(ASTNode &node, SymbolTable &st, ValueType vt) {
  try{
    switch (node.type) {
    case NodeType::TOP_LEVEL:
      return eval_top_level(node);
      break;
    case NodeType::BLOCK:
      return eval_block(node, st);
      break;
    case NodeType::ASSIGN_OP:
      return eval_assign_op(node, st);
      break;
    case NodeType::VAR_DECLARE:
      return eval_var_declare(node, st);
      break;
    case NodeType::FUNC_DECLARE:
      return eval_func_declare(node, st);
      break;
    case NodeType::MODULE_IMPORT:
      return eval_module_import(node, st);
      break;
    case NodeType::IF:
      return eval_if(node, st);
      break;
    case NodeType::RETURN:
      return eval_return(node, st);
      break;
    case NodeType::WHILE:
      return eval_while(node, st);
      break;
    case NodeType::BINARY_OP:
      return eval_binary_op(node, st);
      break;
    case NodeType::UNARY_OP:
      return eval_unary_op(node, st);
      break;
    case NodeType::FUNC_CALL:
      return eval_func_call(node, st);
      break;
    case NodeType::INDEX_ACCESS:
      return eval_index_access(node, st, vt);
      break;
    case NodeType::FIELD_ACESS:
      return eval_field_access(node, st, vt);
      break;
    case NodeType::VAR_LOOKUP:
      return eval_var_lookup(node, st, vt);
      break;
    case NodeType::EXPR_LIST:
      // not a typo, in current implementation the expr list is basically a 
      // special case of a vector expression/literal
      return eval_vec_literal(node, st);
      break;
    case NodeType::BUILTIN_PRINT:
      return eval_builtin_print(node, st);
      break;
    case NodeType::BUILTIN_VECTOR_LENGTH:
      return eval_builtin_vector_length(node, st);
      break;
    case NodeType::BUILTIN_VECTOR_APPEND:
      return eval_builtin_vector_append(node, st);
      break;
    case NodeType::BUILTIN_STRING_LENGTH:
      return eval_builtin_string_length(node, st);
      break;
    case NodeType::BUILTIN_DICT_LENGTH:
      return eval_builtin_dict_length(node, st);
      break;
    case NodeType::BUILTIN_DICT_KEYS:
      return eval_builtin_dict_keys(node, st);
      break;
    case NodeType::BUILTIN_DICT_CONTAINS:
      return eval_builtin_dict_contains(node, st);
      break;
    case NodeType::VEC_LITERAL:
      return eval_vec_literal(node, st);
      break;
    case NodeType::DICT_LITERAL:
      return eval_dict_literal(node, st);
      break;
    case NodeType::BOOL_LITERAL:
      return eval_bool_literal(node, st);
      break;
    case NodeType::INT_LITERAL:
      return eval_int_literal(node, st);
      break;
    case NodeType::FLOAT_LITERAL:
      return eval_float_literal(node, st);
      break;
    case NodeType::STRING_LITERAL:
      return eval_string_literal(node, st);
      break;
    }
  }
  catch (std::exception &e) {
    // if a runtime error occurs, report the error and report where in the source file it
    // happens, then exit.
    std::cerr << "Runtime error encountered at line " << (node.metadata.line + 1) 
      << ", column " << (node.metadata.column) << ":\n"
      << "   " << e.what() << "\n";
    exit(-1);
  }
}
