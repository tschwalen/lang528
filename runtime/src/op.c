#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datatype.h"
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
  }
  }
  runtime_error("Invalid type for integer add.");
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
  }
  }
  runtime_error("Invalid type for floating-point add.");
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
  }
  }
  runtime_error("Invalid type for integer subtract.");
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
  }
  }
  runtime_error("Invalid type for floating-point subtract.");
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
  }
  }
  runtime_error("Invalid type for integer multiply.");
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
  }
  }
  runtime_error("Invalid type for floating-point multiply.");
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
  }
  }
  runtime_error("Invalid type for integer divide.");
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
  }
  }
  runtime_error("Invalid type for floating-point divide.");
  return obj;
}

/*
 * Take two C strings, allocate heap space for their combined length,
 * then concatenate them.
 */
char *strdupcat(const char *str1, const char *str2) {
  size_t len = strlen(str1) + strlen(str2) + 1;
  char *result = malloc(len);
  if (result) {
    strcpy(result, str1);
    strcat(result, str2);
  }
  return result;
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
    snprintf(rhs_buffer, sizeof(rhs_buffer), "%f", rhs->value.v_float);
    break;
  case T_INT:
    snprintf(rhs_buffer, sizeof(rhs_buffer), "%lld", rhs->value.v_int);
    break;
  case T_STRING:
    rhs_string = rhs->value.v_str->contents;
    break;
  case T_VECTOR:
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