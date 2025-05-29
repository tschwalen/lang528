#include <stdint.h>
#include <stdlib.h>

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
    // runtime error
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
    // runtime error
  }
  }

  return obj;
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
    // TODO: str cat
  }

  default: {
    // runtime error, unsupported type
  }
  }
  return NULL;
}
