#include <cstddef>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>

#include "interpreter.h"
#include "runtime.h"
#include "tokentype.h"

using std::shared_ptr;
using std::string;

/*
Binary Operator valid type defitions

op          lhs         rhs
+           int/float   int/float
+           string      string/int/float/bool/vec
/           int/float   int/float
-           int/float   int/float
*           int/float   int/float
%           int         int
==          [any]       [any]
!=          [any]       [any]
<=          int/float   int/float
>=          int/float   int/float
<           int/float   int/float
>           int/float   int/float
()          function    expr-list
[]          indexable   key
&           bool        bool
|           bool        bool

Unary operator valid type definitions
op          rhs
-           int/float
!           bool

*/

static Multiply multiply;
static Add add;
static Subtract subtract;
static Divide divide;
static Less less;
static Greater greater;
static LessEqual lessEqual;
static GreaterEqual greaterEqual;

bool equality_comparison(BoxedValue lhs, BoxedValue rhs);

string toString(BoxedValue bv) {
  std::stringstream result;
  switch (bv.type) {
  case DataType::NOTHING:
    result << "nothing";
    break;
  case DataType::BOOL:
    result << (std::get<bool>(bv.value) ? "true" : "false");
    break;
  case DataType::FLOAT: {
    // char buffer[64];
    // const int DBL_DECIMAL_DIG = 10;
    // snprintf(buffer, sizeof(buffer), "%.*g", DBL_DECIMAL_DIG,
    //          std::get<double>(bv.value));
    // result << buffer << "ahhhhhh";
    result << std::setprecision(std::numeric_limits<double>::max_digits10)
           << std::get<double>(bv.value);
    break;
  }
  case DataType::INT:
    result << std::get<int>(bv.value);
    break;
  case DataType::STRING:
    result << std::get<string>(bv.value);
    break;
  case DataType::VECTOR: {
    auto vec = std::get<shared_ptr<HeVec>>(bv.value);
    result << "[";
    size_t i = 0;
    size_t length = vec->size();
    while (i < length) {
      auto elem = vec->at(i);
      auto quotes = elem->type == DataType::STRING ? "\"" : "";
      result << quotes;
      result << toString(BoxedValue{elem->type, elem->value});
      result << quotes;
      if (i != length - 1) {
        result << ", ";
      }
      ++i;
    }
    result << "]";
    break;
  }
  case DataType::DICT: {
    auto dict = std::get<shared_ptr<Dict>>(bv.value);
    result << "{";
    size_t i = 0;
    size_t length = dict->size();
    for (const auto &[_raw_key, kv_pair] : *dict) {
      auto key = kv_pair.first;
      auto quotes = key.type == DataType::STRING ? "\"" : "";

      auto value = kv_pair.second;
      result << quotes << toString(key) << quotes << ": "
             << toString(BoxedValue{value->type, value->value});
      if (i != length - 1) {
        result << ", ";
      }
      ++i;
    }
    result << "}";
    break;
  }
  case DataType::MODULE: {
    auto module = std::get<Module>(bv.value);
    result << "module:" << module.name;
    break;
  }
  case DataType::FUNCTION: {
    auto function = std::get<Function>(bv.value);
    result << "function:" << function.name << "(";
    size_t i = 0;
    size_t length = function.args.size();
    while (i < length) {
      result << function.args.at(i);
      if (i != length - 1) {
        result << ", ";
      }
      ++i;
    }
    result << ")";
    break;
  }
  }
  return result.str();
}

BoxedValue apply_times(BoxedValue lhs, BoxedValue rhs) {
  return multiply.apply(lhs, rhs);
}

BoxedValue apply_div(BoxedValue lhs, BoxedValue rhs) {
  return divide.apply(lhs, rhs);
}

BoxedValue apply_minus(BoxedValue lhs, BoxedValue rhs) {
  return subtract.apply(lhs, rhs);
}

BoxedValue apply_plus(BoxedValue lhs, BoxedValue rhs) {
  if (lhs.type == DataType::STRING) {
    std::stringstream new_value;
    new_value << std::get<string>(lhs.value);
    new_value << toString(rhs);
    return BoxedValue{DataType::STRING, new_value.str()};
  }
  return add.apply(lhs, rhs);
}

BoxedValue apply_mod(BoxedValue lhs, BoxedValue rhs) {
  if (lhs.type == DataType::INT && rhs.type == DataType::INT) {
    return BoxedValue{DataType::INT,
                      std::get<int>(lhs.value) % std::get<int>(rhs.value)};
  }
  throw std::runtime_error(
      "Modulo operator is only supported between integer types");
}

bool vector_equality_comparison(shared_ptr<HeVec> lhs, shared_ptr<HeVec> rhs) {
  // trivially, if vectors don't have the same size then they're not equal
  if (lhs->size() != rhs->size()) {
    return false;
  }

  // trivially, if vectors are the same size and one is empty, then both are
  // empty and they are equal
  if (lhs->size() == 0) {
    return true;
  }

  // otherwise, do an equality comparison for every element, recursively
  // checking any sub-vectors in the same way
  for (size_t i = 0; i < lhs->size(); ++i) {
    auto lhs_elem = lhs->at(i);
    auto rhs_elem = rhs->at(i);

    if (!equality_comparison(BoxedValue{lhs_elem->type, lhs_elem->value},
                             BoxedValue{rhs_elem->type, rhs_elem->value})) {
      return false;
    }
  }

  // if we've made it here, then both vectors are equal
  return true;
}

bool dict_equality_comparison(shared_ptr<Dict> lhs, shared_ptr<Dict> rhs) {
  // trivially, if dicts don't have the same size then they're not equal
  if (lhs->size() != rhs->size()) {
    return false;
  }

  // trivially, if dicts are the same size and one is empty, then both are
  // empty and they are equal
  if (lhs->size() == 0) {
    return true;
  }

  // otherwise, check that each key-value pair in the left dict matches
  // the right dict. Since we checked the sizes, if we don't fail any
  // equality checks then they're equal.
  for (const auto &[_raw_key, kv_pair] : *lhs) {
    auto str_key = getDictKey(kv_pair.first);

    // if the lhs key isn't in the rhs dict, then we already know
    // they're not equal
    if (!rhs->contains(str_key)) {
      return false;
    }

    // compare the values of each
    auto lhs_value = kv_pair.second;
    auto rhs_value = rhs->at(str_key).second;
    if (!equality_comparison(BoxedValue{lhs_value->type, lhs_value->value},
                             BoxedValue{rhs_value->type, rhs_value->value})) {
      return false;
    }
  }

  // if we've made it here, then both dicts are equal
  return true;
}

bool equality_comparison(BoxedValue lhs, BoxedValue rhs) {
  if (lhs.type != rhs.type) {
    return false;
  }

  switch (lhs.type) {
  case DataType::NOTHING:
    return true;
  case DataType::INT:
    return std::get<int>(lhs.value) == std::get<int>(rhs.value);
  case DataType::FLOAT:
    return std::get<double>(lhs.value) == std::get<double>(rhs.value);
  case DataType::BOOL:
    return std::get<bool>(lhs.value) == std::get<bool>(rhs.value);
  case DataType::STRING:
    return std::get<string>(lhs.value) == std::get<string>(rhs.value);
  case DataType::FUNCTION:
    throw std::runtime_error(
        "Equality Comparison not supported for function type");
  case DataType::MODULE:
    throw std::runtime_error(
        "Equality Comparison not supported for module type");
  case DataType::VECTOR: {
    return vector_equality_comparison(std::get<shared_ptr<HeVec>>(lhs.value),
                                      std::get<shared_ptr<HeVec>>(rhs.value));
  }
  case DataType::DICT:
    return dict_equality_comparison(std::get<shared_ptr<Dict>>(lhs.value),
                                    std::get<shared_ptr<Dict>>(rhs.value));
  default:
    return false;
  }
}

string getDictKey(BoxedValue bv) {
  switch (bv.type) {
  case DataType::BOOL:
    return "bool:" + toString(bv);
    break;
  case DataType::FLOAT:
    return "float:" + toString(bv);
    break;
    break;
  case DataType::INT:
    return "int:" + toString(bv);
    break;
  case DataType::STRING:
    return "string:" + toString(bv);
    break;
  default: {
  }
  }
  throw std::runtime_error("Unhashable type used for dictionary key");
  return "";
}

bool get_conditional_result(BoxedValue bv) {
  // "nothing" is falsey
  if (bv.type == DataType::NOTHING) {
    return false;
  }
  if (bv.type == DataType::BOOL) {
    return std::get<bool>(bv.value);
  }
  throw std::runtime_error(
      "Conditional expression must have boolean or nothing result.");
}

BoxedValue apply_equals(BoxedValue lhs, BoxedValue rhs) {
  return BoxedValue{DataType::BOOL, equality_comparison(lhs, rhs)};
}

BoxedValue apply_not_equals(BoxedValue lhs, BoxedValue rhs) {
  return BoxedValue{DataType::BOOL, !equality_comparison(lhs, rhs)};
}

BoxedValue apply_less_equals(BoxedValue lhs, BoxedValue rhs) {
  return lessEqual.apply(lhs, rhs);
}

BoxedValue apply_greater_equals(BoxedValue lhs, BoxedValue rhs) {
  return greaterEqual.apply(lhs, rhs);
}

BoxedValue apply_less(BoxedValue lhs, BoxedValue rhs) {
  return less.apply(lhs, rhs);
}

BoxedValue apply_greater(BoxedValue lhs, BoxedValue rhs) {
  return greater.apply(lhs, rhs);
}

BoxedValue apply_and(BoxedValue lhs, BoxedValue rhs) {
  if (lhs.type == DataType::BOOL && rhs.type == DataType::BOOL) {
    return BoxedValue{DataType::BOOL,
                      std::get<bool>(lhs.value) && std::get<bool>(rhs.value)};
  }
  throw std::runtime_error("Boolean operators require boolean operands");
}

BoxedValue apply_or(BoxedValue lhs, BoxedValue rhs) {
  if (lhs.type == DataType::BOOL && rhs.type == DataType::BOOL) {
    return BoxedValue{DataType::BOOL,
                      std::get<bool>(lhs.value) || std::get<bool>(rhs.value)};
  }
  throw std::runtime_error("Boolean operators require boolean operands");
}

BoxedValue apply_unary_not(BoxedValue rhs) {
  if (rhs.type == DataType::BOOL) {
    return BoxedValue{DataType::BOOL, !std::get<bool>(rhs.value)};
  }
  throw std::runtime_error("Unary not requires a boolean operand");
}

BoxedValue apply_unary_minus(BoxedValue rhs) {
  if (rhs.type == DataType::FLOAT) {
    return BoxedValue{DataType::FLOAT, -std::get<double>(rhs.value)};
  }
  if (rhs.type == DataType::INT) {
    return BoxedValue{DataType::INT, -std::get<int>(rhs.value)};
  }
  throw std::runtime_error("Unary minus requires a numeric operand");
}

BoxedValue apply_binary_operator(TokenType op, BoxedValue lhs, BoxedValue rhs) {
  switch (op) {
  case TokenType::PLUS:
    return apply_plus(lhs, rhs);
  case TokenType::MINUS:
    return apply_minus(lhs, rhs);
  case TokenType::TIMES:
    return apply_times(lhs, rhs);
  case TokenType::DIV:
    return apply_div(lhs, rhs);
  case TokenType::MOD:
    return apply_mod(lhs, rhs);
  case TokenType::EQUALS_EQUALS:
    return apply_equals(lhs, rhs);
  case TokenType::NOT_EQUALS:
    return apply_not_equals(lhs, rhs);
  case TokenType::LESS_EQUALS:
    return apply_less_equals(lhs, rhs);
  case TokenType::GREATER_EQUALS:
    return apply_greater_equals(lhs, rhs);
  case TokenType::LESS:
    return apply_less(lhs, rhs);
  case TokenType::GREATER:
    return apply_greater(lhs, rhs);
  case TokenType::AND:
    return apply_and(lhs, rhs);
  case TokenType::OR:
    return apply_or(lhs, rhs);
  default:
    break;
  }
  throw std::runtime_error("TokenType argument op must be a binary operator");
}

BoxedValue apply_unary_operator(TokenType op, BoxedValue rhs) {
  switch (op) {
  case TokenType::NOT:
    return apply_unary_not(rhs);
    break;
  case TokenType::MINUS:
    return apply_unary_minus(rhs);
    break;
  default:
    break;
  }
  throw std::runtime_error("TokenType argument op must be a unary operator");
}

BoxedValue ArithmeticCompBinOp::apply(BoxedValue lhs, BoxedValue rhs) {
  if (lhs.type == DataType::INT) {
    if (rhs.type == DataType::INT) {
      return BoxedValue{
          DataType::BOOL,
          this->apply_raw(std::get<int>(lhs.value), std::get<int>(rhs.value))};
    }
    if (rhs.type == DataType::FLOAT) {
      return BoxedValue{DataType::BOOL,
                        this->apply_raw(std::get<int>(lhs.value),
                                        std::get<double>(rhs.value))};
    }
  }
  if (lhs.type == DataType::FLOAT) {
    if (rhs.type == DataType::INT) {
      return BoxedValue{DataType::BOOL,
                        this->apply_raw(std::get<double>(lhs.value),
                                        std::get<int>(rhs.value))};
    }
    if (rhs.type == DataType::FLOAT) {
      return BoxedValue{DataType::BOOL,
                        this->apply_raw(std::get<double>(lhs.value),
                                        std::get<double>(rhs.value))};
    }
  }

  throw std::runtime_error("Arithmetic Comparison Operators are only supported "
                           "between numeric types");
}

BoxedValue ArithmeticBinOp::apply(BoxedValue lhs, BoxedValue rhs) {
  if (lhs.type == DataType::INT) {
    if (rhs.type == DataType::INT) {
      return BoxedValue{
          DataType::INT,
          this->apply_raw(std::get<int>(lhs.value), std::get<int>(rhs.value))};
    }
    if (rhs.type == DataType::FLOAT) {
      return BoxedValue{DataType::FLOAT,
                        this->apply_raw(std::get<int>(lhs.value),
                                        std::get<double>(rhs.value))};
    }
  }
  if (lhs.type == DataType::FLOAT) {
    if (rhs.type == DataType::INT) {
      return BoxedValue{DataType::FLOAT,
                        this->apply_raw(std::get<double>(lhs.value),
                                        std::get<int>(rhs.value))};
    }
    if (rhs.type == DataType::FLOAT) {
      return BoxedValue{DataType::FLOAT,
                        this->apply_raw(std::get<double>(lhs.value),
                                        std::get<double>(rhs.value))};
    }
  }

  throw std::runtime_error(
      "Arithmetic Operators are only supported between numeric types");
}

void builtin_print(BoxedValue arg) { std::cout << toString(arg) << "\n"; }

BoxedValue builtin_vector_length(BoxedValue arg) {
  auto vec = std::get<shared_ptr<HeVec>>(arg.value);
  return BoxedValue{DataType::INT, (int)vec->size()};
}

void builtin_vector_append(BoxedValue vec, BoxedValue elem) {
  auto raw_vec = std::get<shared_ptr<HeVec>>(vec.value);
  raw_vec->push_back(std::make_shared<BoxedValue>(elem.type, elem.value));
}

BoxedValue builtin_string_length(BoxedValue arg) {
  auto str = std::get<string>(arg.value);
  return BoxedValue{DataType::INT, (int)str.size()};
}

BoxedValue builtin_dict_length(BoxedValue arg) {
  auto dict = std::get<shared_ptr<Dict>>(arg.value);
  return BoxedValue{DataType::INT, (int)dict->size()};
}

BoxedValue builtin_dict_keys(BoxedValue arg) {
  auto dict = std::get<shared_ptr<Dict>>(arg.value);
  auto keys = std::make_shared<HeVec>();
  for (const auto &[_, kv_pair] : *dict) {
    keys->push_back(
        std::make_shared<BoxedValue>(kv_pair.first.type, kv_pair.first.value));
  }

  return BoxedValue{DataType::VECTOR, keys};
}

BoxedValue builtin_dict_contains(BoxedValue arg, BoxedValue key) {
  auto dict = std::get<shared_ptr<Dict>>(arg.value);
  auto contains = dict->contains(getDictKey(key));
  return BoxedValue{DataType::BOOL, contains};
}

void runtime_assertion(bool condition, string message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}
