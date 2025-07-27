#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datatype.h"
#include "rtutil.h"
#include "runtime.h"

RuntimeObject *_op_add_int(int64_t lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->type = T_INT;
    obj->value.v_int = lhs + rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->type = T_FLOAT;
    obj->value.v_float = lhs + rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for integer add.");
  }
  }
  return obj;
}

RuntimeObject *_op_add_float(double lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->type = T_FLOAT;
    obj->value.v_int = lhs + rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->type = T_FLOAT;
    obj->value.v_float = lhs + rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for floating-point add.");
  }
  }
  return obj;
}

RuntimeObject *_op_sub_int(int64_t lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->type = T_INT;
    obj->value.v_int = lhs - rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->type = T_FLOAT;
    obj->value.v_float = lhs - rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for integer subtract.");
  }
  }
  return obj;
}

RuntimeObject *_op_sub_float(double lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->type = T_FLOAT;
    obj->value.v_int = lhs - rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->type = T_FLOAT;
    obj->value.v_float = lhs - rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for floating-point subtract.");
  }
  }
  return obj;
}

RuntimeObject *_op_mul_int(int64_t lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->type = T_INT;
    obj->value.v_int = lhs * rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->type = T_FLOAT;
    obj->value.v_float = lhs * rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for integer multiply.");
  }
  }
  return obj;
}

RuntimeObject *_op_mul_float(double lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->type = T_FLOAT;
    obj->value.v_int = lhs * rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->type = T_FLOAT;
    obj->value.v_float = lhs * rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for floating-point multiply.");
  }
  }
  return obj;
}

RuntimeObject *_op_div_int(int64_t lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->type = T_INT;
    obj->value.v_int = lhs / rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->type = T_FLOAT;
    obj->value.v_float = lhs / rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for integer divide.");
  }
  }
  return obj;
}

RuntimeObject *_op_div_float(double lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->type = T_FLOAT;
    obj->value.v_int = lhs / rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->type = T_FLOAT;
    obj->value.v_float = lhs / rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for floating-point divide.");
  }
  }
  return obj;
}

RuntimeObject *_op_leq_int(int64_t lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_BOOL;

  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->value.v_bool = lhs <= rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->value.v_bool = lhs <= rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for integer less-equals.");
  }
  }
  return obj;
}

RuntimeObject *_op_leq_float(double lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_BOOL;

  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->type = T_FLOAT;
    obj->value.v_bool = lhs <= rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->type = T_FLOAT;
    obj->value.v_bool = lhs <= rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for floating-point less-equals.");
  }
  }
  return obj;
}

RuntimeObject *_op_geq_int(int64_t lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_BOOL;

  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->value.v_bool = lhs >= rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->value.v_bool = lhs >= rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for integer greater-equals.");
  }
  }
  return obj;
}

RuntimeObject *_op_geq_float(double lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_BOOL;

  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->value.v_bool = lhs >= rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->value.v_bool = lhs >= rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for floating-point greater-equals.");
  }
  }
  return obj;
}

RuntimeObject *_op_lt_int(int64_t lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_BOOL;

  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->value.v_bool = lhs < rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->value.v_bool = lhs < rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for integer less-than.");
  }
  }
  return obj;
}

RuntimeObject *_op_lt_float(double lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_BOOL;
  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->value.v_bool = lhs < rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->value.v_bool = lhs < rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for floating-point less-than.");
  }
  }
  return obj;
}

RuntimeObject *_op_gt_int(int64_t lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_BOOL;

  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->value.v_bool = lhs > rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->value.v_bool = lhs > rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for integer greater-than.");
  }
  }
  return obj;
}

RuntimeObject *_op_gt_float(double lhs, RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_BOOL;

  switch (rhs->type) {
  case T_INT: {
    int64_t rhs_value = rhs->value.v_int;
    obj->value.v_bool = lhs > rhs_value;
    break;
  }
  case T_FLOAT: {
    double rhs_value = rhs->value.v_float;
    obj->value.v_bool = lhs > rhs_value;
    break;
  }
  default: {
    runtime_error("Invalid type for floating-point greater-than.");
  }
  }
  return obj;
}

bool equality_comparison(RuntimeObject *lhs, RuntimeObject *rhs) {
  if (lhs->type != rhs->type) {
    return false;
  }

  switch (lhs->type) {
  case T_NOTHING:
    return true;
  case T_INT: {
    return lhs->value.v_int == rhs->value.v_int;
  }
  case T_FLOAT: {
    return lhs->value.v_float == rhs->value.v_float;
  }
  case T_BOOL: {
    return lhs->value.v_bool == rhs->value.v_bool;
  }
  case T_STRING: {
    char *lhs_str = lhs->value.v_str->contents;
    char *rhs_str = rhs->value.v_str->contents;
    return strcmp(lhs_str, rhs_str) == 0;
  }
  // TODO - vector and dict
  case T_FUNCTION:
    runtime_error("Equality Comparison not supported for function type");
  case T_MODULE:
    runtime_error("Equality Comparison not supported for module type");
  default: {
  }
  }
  return false;
}

RuntimeObject *_str_concat(String *lhs, RuntimeObject *rhs) {
  char rhs_buffer[256] = "nothing";
  char *rhs_string = rhs_buffer;
  switch (rhs->type) {
  case T_NOTHING:
    break;
  case T_BOOL:
    if (rhs->value.v_bool) {
      strcpy(rhs_string, "true");
    } else {
      strcpy(rhs_string, "false");
    }
    break;
  case T_FLOAT:
    snprintf(rhs_buffer, sizeof(rhs_buffer), "%.1f", rhs->value.v_float);
    break;
  case T_INT:
    snprintf(rhs_buffer, sizeof(rhs_buffer), "%lld", rhs->value.v_int);
    break;
  case T_STRING:
    rhs_string = rhs->value.v_str->contents;
    break;
  case T_VECTOR: {
    // start a string accumulator, with an opening bracket
    RuntimeObject *acc = _str_concat(lhs, make_string("["));

    size_t length = rhs->value.v_vec->size;
    RuntimeObject *vec = rhs->value.v_vec->contents;

    for (size_t i = 0; i < length; ++i) {
      RuntimeObject *elem = &vec[i];

      // check if elem is a string
      bool is_str = elem->type == T_STRING;

      // add opening quote if string
      if (is_str) {
        acc = _str_concat(acc->value.v_str, make_string("\""));
      }

      acc = _str_concat(acc->value.v_str, elem);

      // add closing quote if string
      if (is_str) {
        acc = _str_concat(acc->value.v_str, make_string("\""));
      }

      // add comma separator unless we're on the last element
      if (i != length - 1) {
        acc = _str_concat(acc->value.v_str, make_string(", "));
      }
    }

    // add the closing bracket and then return
    acc = _str_concat(acc->value.v_str, make_string("]"));
    return acc;
    break;
  }
  case T_DICT:
  case T_MODULE:
  case T_FUNCTION:
    strcpy(rhs_string, "NOT_IMPLEMENTED");
    break;
  }

  char *new_str = strdupcat(lhs->contents, rhs_string);
  return make_string_nocopy(new_str);
}

RuntimeObject *op_add(RuntimeObject *lhs, RuntimeObject *rhs) {
  switch (lhs->type) {
  case T_FLOAT: {
    double value = lhs->value.v_float;
    return _op_add_float(value, rhs);
  }
  case T_INT: {
    int64_t value = lhs->value.v_int;
    return _op_add_int(value, rhs);
  }

  case T_STRING: {
    String *value = lhs->value.v_str;
    return _str_concat(value, rhs);
  }
  default: {
  }
  }
  runtime_error("Invalid type for add.");
  return NULL;
}

RuntimeObject *op_sub(RuntimeObject *lhs, RuntimeObject *rhs) {
  switch (lhs->type) {
  case T_FLOAT: {
    double value = lhs->value.v_float;
    return _op_sub_float(value, rhs);
  }
  case T_INT: {
    int64_t value = lhs->value.v_int;
    return _op_sub_int(value, rhs);
  }
  default: {
  }
  }
  runtime_error("Invalid type for subtract.");
  return NULL;
}

RuntimeObject *op_mul(RuntimeObject *lhs, RuntimeObject *rhs) {
  switch (lhs->type) {
  case T_FLOAT: {
    double value = lhs->value.v_float;
    return _op_mul_float(value, rhs);
  }
  case T_INT: {
    int64_t value = lhs->value.v_int;
    return _op_mul_int(value, rhs);
  }
  default: {
  }
  }
  runtime_error("Invalid type for multiply.");
  return NULL;
}

RuntimeObject *op_div(RuntimeObject *lhs, RuntimeObject *rhs) {
  switch (lhs->type) {
  case T_FLOAT: {
    double value = lhs->value.v_float;
    return _op_div_float(value, rhs);
  }
  case T_INT: {
    int64_t value = lhs->value.v_int;
    return _op_div_int(value, rhs);
  }
  default: {
  }
  }
  runtime_error("Invalid type for divide.");
  return NULL;
}

RuntimeObject *op_mod(RuntimeObject *lhs, RuntimeObject *rhs) {
  // Modulo is only supported on two integers
  if (lhs->type == T_INT && rhs->type == T_INT) {
    int64_t lhs_value = lhs->value.v_int;
    int64_t rhs_value = rhs->value.v_int;
    return make_int(lhs_value % rhs_value);
  }

  runtime_error("Invalid type for modulo.");
  return NULL;
}

RuntimeObject *op_eq(RuntimeObject *lhs, RuntimeObject *rhs) {
  return make_bool(equality_comparison(lhs, rhs));
}

RuntimeObject *op_neq(RuntimeObject *lhs, RuntimeObject *rhs) {
  return make_bool(!equality_comparison(lhs, rhs));
}

RuntimeObject *op_leq(RuntimeObject *lhs, RuntimeObject *rhs) {
  switch (lhs->type) {
  case T_FLOAT: {
    double value = lhs->value.v_float;
    return _op_leq_float(value, rhs);
  }
  case T_INT: {
    int64_t value = lhs->value.v_int;
    return _op_leq_int(value, rhs);
  }
  default: {
  }
  }
  runtime_error("Invalid type for less-equals.");
  return NULL;
}

RuntimeObject *op_geq(RuntimeObject *lhs, RuntimeObject *rhs) {
  switch (lhs->type) {
  case T_FLOAT: {
    double value = lhs->value.v_float;
    return _op_geq_float(value, rhs);
  }
  case T_INT: {
    int64_t value = lhs->value.v_int;
    return _op_geq_int(value, rhs);
  }
  default: {
  }
  }
  runtime_error("Invalid type for greater-equals.");
  return NULL;
}

RuntimeObject *op_lt(RuntimeObject *lhs, RuntimeObject *rhs) {
  switch (lhs->type) {
  case T_FLOAT: {
    double value = lhs->value.v_float;
    return _op_lt_float(value, rhs);
  }
  case T_INT: {
    int64_t value = lhs->value.v_int;
    return _op_lt_int(value, rhs);
  }
  default: {
  }
  }
  runtime_error("Invalid type for less-than.");
  return NULL;
}

RuntimeObject *op_gt(RuntimeObject *lhs, RuntimeObject *rhs) {
  switch (lhs->type) {
  case T_FLOAT: {
    double value = lhs->value.v_float;
    return _op_gt_float(value, rhs);
  }
  case T_INT: {
    int64_t value = lhs->value.v_int;
    return _op_gt_int(value, rhs);
  }
  default: {
  }
  }
  runtime_error("Invalid type for greater-than.");
  return NULL;
}

RuntimeObject *op_and(RuntimeObject *lhs, RuntimeObject *rhs) {
  // And is only supported on two booleans
  if (lhs->type == T_BOOL && rhs->type == T_BOOL) {
    bool lhs_value = lhs->value.v_bool;
    bool rhs_value = rhs->value.v_bool;
    return make_bool(lhs_value && rhs_value);
  }

  runtime_error("Invalid type for boolean and.");
  return NULL;
}

RuntimeObject *op_or(RuntimeObject *lhs, RuntimeObject *rhs) {
  // Or is only supported on two booleans
  if (lhs->type == T_BOOL && rhs->type == T_BOOL) {
    bool lhs_value = lhs->value.v_bool;
    bool rhs_value = rhs->value.v_bool;
    return make_bool(lhs_value || rhs_value);
  }

  runtime_error("Invalid type for boolean or.");
  return NULL;
}

RuntimeObject *op_umin(RuntimeObject *rhs) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  // unary minus requires a numeric argument
  switch (rhs->type) {
  case T_INT: {
    int64_t new_value = -(rhs->value.v_int);
    obj->type = T_INT;
    obj->value.v_int = new_value;
    break;
  }
  case T_FLOAT: {
    double new_value = -(rhs->value.v_float);
    obj->type = T_FLOAT;
    obj->value.v_float = new_value;
    break;
  }
  default: {
    runtime_error("Invalid type for unary minus");
  }
  }
  return obj;
}

RuntimeObject *op_unot(RuntimeObject *rhs) {
  // Not is only supported on booleans
  if (rhs->type == T_BOOL) {
    bool rhs_value = rhs->value.v_bool;
    return make_bool(!rhs_value);
  }

  runtime_error("Invalid type for unary not.");
  return NULL;
}