

#include "astnode.h"
#include <memory>
#include <optional>
#include <variant>
#include <vector>
#include <unordered_map>


using std::string;
using std::vector;
using std::shared_ptr;
using std::optional;
using std::unordered_map;

// struct BoxedValue;

enum class ValueType {
    LVALUE,
    RVALUE
};

enum class DataType {
    BOOL,
    FLOAT,
    INT,
    STRING,
    VECTOR,
    FUNCTION
};

struct Function {
    string name;
    vector<string> args;
    ASTNode body;
};

enum class VarType {
    CONST, VAR, FUNCTION
};

struct BoxedValue;
struct SymbolTableEntry {
    VarType type;
    shared_ptr<BoxedValue> value;
};

struct SymbolTable;
struct SymbolTable {
    SymbolTable *parent = nullptr;
    unordered_map<string, SymbolTableEntry> entries;
};

// change this unless at some point we put things other than a symbol table in the execution context
typedef SymbolTable ExecutionContext;
// struct ExecutionContext {

// };

typedef vector<shared_ptr<BoxedValue>> HeVec;

typedef std::variant<
    std::monostate, 
    bool, 
    int, 
    float, 
    string, 
    shared_ptr<HeVec>, 
    Function
> RawValue;

struct BoxedValue {
    DataType type;
    RawValue value;
};

struct EvalResult {
    optional<BoxedValue> lv_result;
    bool returned = false;
};

EvalResult eval_node(ASTNode &node, ExecutionContext &ec, ValueType vt=ValueType::RVALUE);


EvalResult eval_var_declare(ASTNode &node, ExecutionContext &ec) {
  /*
    What is the var's id?
    Is it const or var?
    Does the var exist in the current symbol table?
  */

  string identifier = node.data.at("identifier").get<string>();
  bool is_const = node.data.at("const").get<bool>();

  // make sure we don't declare more than once
  // TODO: better error handling
  assert(!ec.entries.contains(identifier));

  // eval the right hand side of the expression (first and only child)
  auto rhs = node.children[0];
  auto result = eval_node(rhs,  ec);
  auto value = std::make_shared<BoxedValue>(result.lv_result.value());

  ec.entries["indentifier"] = SymbolTableEntry {
    is_const ? VarType::CONST : VarType::VAR,
    value
  };

  return EvalResult {};
}

EvalResult eval_func_declare(ASTNode &node, ExecutionContext &ec) {
  string name = node.data.at("function_name").get<string>();
  vector<string> args = node.data.at("args").get<vector<string>>();
  auto body = node.children[0];

  assert(!ec.entries.contains(name));

  ec.entries["name"] = SymbolTableEntry {
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

EvalResult eval_vec_literal(ASTNode &node, ExecutionContext &ec, ValueType vt) {
    auto child_nodes = node.children;
    auto vec_value = std::make_shared<HeVec>();
    for (auto &node : child_nodes) {
        auto result = eval_node(node, ec);
        vec_value->push_back(std::make_shared<BoxedValue>(result.lv_result->value));
    }
    return EvalResult {
        BoxedValue {
            DataType::VECTOR,
            vec_value
        }
    };
}


EvalResult eval_string_literal(ASTNode &node, ExecutionContext &ec, ValueType vt) {
    auto value = node.data.at("value").get<string>();
    return EvalResult {
        BoxedValue {
            DataType::STRING,
            value
        }
    };
}

EvalResult eval_float_literal(ASTNode &node, ExecutionContext &ec, ValueType vt) {
    auto value = node.data.at("value").get<float>();
    return EvalResult {
        BoxedValue {
            DataType::FLOAT,
            value
        }
    };
}

EvalResult eval_int_literal(ASTNode &node, ExecutionContext &ec, ValueType vt) {
    auto value = node.data.at("value").get<int>();
    return EvalResult {
        BoxedValue {
            DataType::INT,
            value
        }
    };
}

EvalResult eval_bool_literal(ASTNode &node, ExecutionContext &ec, ValueType vt) {
    auto value = node.data.at("value").get<bool>();
    return EvalResult {
        BoxedValue {
            DataType::BOOL,
            value
        }
    };
}

EvalResult eval_block(ASTNode &node, ExecutionContext &ec) {
    for (auto &child : node.children) {
        auto result = eval_node(child, ec);
        if (result.returned) {
          return result;
        }
    }
    return EvalResult {};
}

EvalResult eval_while(ASTNode &node, ExecutionContext &ec) {
  const int CONDITION = 0, BODY = 1;
  EvalResult result;

CHECK_CONDITION:
  auto condition = eval_node(node.children[CONDITION], ec).lv_result.value();
  assert(condition.type == DataType::BOOL);
  auto conditional_result = std::get<bool>(condition.value);

  if (conditional_result) {
    ExecutionContext block_ec {&ec, {}};
    result = eval_node(node.children[BODY], block_ec);

    // exit early and propate return value if we returned from block
    if (result.returned) {
      return result;
    }
    goto CHECK_CONDITION;
  }

  return EvalResult {};
}

EvalResult eval_if(ASTNode &node, ExecutionContext &ec) {
  /*
      node.children[0] => conditional
      node.children[1] => if-body
      node.children[2] => ?else-body
  */

  const int CONDITION = 0, IF_BODY = 1, ELSE_BODY = 2, SIZE_IF_ELSE = 3;

  auto condition = eval_node(node.children[CONDITION], ec).lv_result.value();
  assert(condition.type == DataType::BOOL);
  auto conditional_result = std::get<bool>(condition.value);

  if (conditional_result) {
    ExecutionContext if_block_ec {&ec, {}};
    return eval_node(node.children[IF_BODY], if_block_ec);
  }
  else if (node.children.size() == SIZE_IF_ELSE) {
    ExecutionContext else_block_ec {&ec, {}};
    return eval_node(node.children[ELSE_BODY], else_block_ec);
  }
  return EvalResult {};
}

EvalResult eval_top_level(ASTNode &node) {
    // TODO: argv needs to make it in somehow
    ExecutionContext top_level_ec;

    for (auto &child : node.children) {
        eval_node(child, top_level_ec);
    }

    if(top_level_ec.entries.contains("main")) {
        auto main = top_level_ec.entries.at("main");
        if(main.type == VarType::FUNCTION) {
            // TODO: actually call main
        }
    }

    return EvalResult {};
}

EvalResult eval_node(ASTNode &node, ExecutionContext &ec, ValueType vt) {
  switch (node.type) {

  case NodeType::TOP_LEVEL:
    return eval_top_level(node);
    break;
  case NodeType::BLOCK:
    return eval_block(node, ec);
    break;
  case NodeType::ASSIGN_OP:

    break;
  case NodeType::VAR_DECLARE:
    return eval_var_declare(node, ec);
    break;
  case NodeType::FUNC_DECLARE:
    return eval_func_declare(node, ec);
    break;
  case NodeType::IF:
    return eval_if(node, ec);
    break;
  case NodeType::RETURN:

    break;
  case NodeType::WHILE:
    return eval_while(node, ec);
    break;
  case NodeType::BINARY_OP:

    break;
  case NodeType::UNARY_OP:

    break;
  case NodeType::FUNC_CALL:

    break;
  case NodeType::INDEX_ACCESS:

    break;
  case NodeType::FIELD_ACESS:

    break;
  case NodeType::VAR_LOOKUP:

    break;
  case NodeType::EXPR_LIST:

    break;
  case NodeType::VEC_LITERAL:
    return eval_vec_literal(node, ec, vt);
    break;
  case NodeType::BOOL_LITERAL:
    return eval_bool_literal(node, ec, vt);
    break;
  case NodeType::INT_LITERAL:
    return eval_int_literal(node, ec, vt);
    break;
  case NodeType::FLOAT_LITERAL:
    return eval_float_literal(node, ec, vt);
    break;
  case NodeType::STRING_LITERAL:
    return eval_string_literal(node, ec, vt);
    break;
  }
}

