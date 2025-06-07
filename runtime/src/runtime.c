#include <stdio.h>
#include <stdlib.h>

#include "datatype.h"
#include "runtime.h"

int placeholder(int x) { return x + x; }

void runtime_error(char *msg) {
  // very barebones for now, could report more debug info like
  // line number later on.
  printf("Runtime error: %s", msg);
  exit(1);
}

bool get_conditional_result(RuntimeObject *obj) {
  // Shouldn't happen, but handle it
  if (obj == NULL) {
    return false;
  }
  // "nothing" is falsey
  if (obj->type == T_NOTHING) {
    return false;
  }
  if (obj->type == T_BOOL) {
    return obj->value.v_bool;
  }
  runtime_error(
      "Conditional expression must have boolean or nothing result type.");
  return NULL;
}

void _print_vector(Vector *vec);

void _print_helper(RuntimeObject *obj) {
  // prints object with no newline
  switch (obj->type) {
  case T_NOTHING:
    printf("nothing");
    return;
  case T_BOOL:
    printf(obj->value.v_bool ? "true" : "false");
    return;
  case T_INT:
    printf("%lld", obj->value.v_int);
    return;
  case T_FLOAT:
    printf("%f", obj->value.v_float);
    return;
  case T_STRING:
    printf("%s", obj->value.v_str->contents);
    return;
  case T_VECTOR:
    _print_vector(obj->value.v_vec);
    return;
  case T_DICT:
  case T_MODULE:
  case T_FUNCTION:
  default:
    break;
  }
  printf("NOT IMPLEMENTED");
  exit(1);
}

void _print_vector(Vector *vec) {
  printf("[");
  // fencepost problem
  int i = 0;
  for (; i < vec->size - 1; ++i) {
    _print_helper(&vec->contents[i]);
    printf(", ");
  }
  _print_helper(&vec->contents[i]);
  printf("]");
}

void builtin_print(RuntimeObject *arg) {
  _print_helper(arg);
  printf("\n");
}
