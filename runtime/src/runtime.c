#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "datatype.h"
#include "runtime.h"

int placeholder(int x) { return x + x; }

RuntimeObject *handle_builtin(RuntimeObject *lhs, String name, size_t nargs,
                              RuntimeObject *args[]) {}

/*
 * Implements index access (x[y]) at runtime.
 */
RuntimeObject *get_index(RuntimeObject *lhs, RuntimeObject *rhs) {
  if (lhs->type == T_VECTOR) {
    if (rhs->type != T_INT) {
      runtime_error("Vector index value must be int.");
    }
    int64_t index = rhs->value.v_int;

    return &(lhs->value.v_vec->contents[index]);
  }

  runtime_error("Not impemented (get_index)");
}

RuntimeObject *make_argv(int argc, char *argv[]) {
  RuntimeObject *rt_argv = make_vector_known_size(argc - 1);
  Vector *vec = rt_argv->value.v_vec;
  for (size_t i = 1; i < argc; ++i) {
    vec->contents[i - 1] = *make_string(argv[i]);
  }
  return rt_argv;
}

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

// VECTOR methods
RuntimeObject *vec_length(RuntimeObject *self) {
  return make_int(self->value.v_vec->size);
}

RuntimeObject *vec_append(RuntimeObject *self, RuntimeObject *obj) {
  Vector *vec = self->value.v_vec;
  size_t new_size = vec->size + 1;

  if (new_size > vec->internal_size) {
    // double internal cap
    size_t new_internal_size = vec->internal_size * 2;
    // allocate new internal storage
    RuntimeObject *new_contents =
        calloc(new_internal_size, sizeof(RuntimeObject));
    // copy existing elements over
    for (size_t i = 0; i < vec->internal_size; i++) {
      RuntimeObject *elem = &(vec->contents[i]);
      new_contents[i] = *elem;
    }
    // free old array, assign new array to contents
    free(vec->contents);
    vec->contents = new_contents;
    vec->internal_size = new_internal_size;
  }
  vec->size = new_size;
  vec->contents[vec->size] = *obj;
  return make_nothing();
}
