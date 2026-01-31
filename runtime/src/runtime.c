#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "datatype.h"
#include "dictionary.h"
#include "rtutil.h"
#include "runtime.h"

int placeholder(int x) { return x + x; }
RuntimeSymbolTableEntry *runtime_st_lookup(RuntimeSymbolTable *st,
                                           char *identifier);

String *get_dict_key(RuntimeObject *key) {
  char *type_id = NULL;
  switch (key->type) {
  case T_BOOL:
    type_id = "bool:";
  case T_FLOAT:
    type_id = "float:";
    break;
  case T_INT:
    type_id = "int:";
    break;
  case T_STRING:
    type_id = "string:";
    break;
  default: {
    runtime_error("Unhashable type used for dictionary key");
  }
  }

  return str_concat_raw(make_string_raw(type_id), to_string_raw(key));
}

RuntimeObject *dynamic_function_call(RuntimeObject *dynamic_fn, size_t argc,
                                     RuntimeObject *argv[]) {
  if (dynamic_fn->type != T_FUNCTION) {
    runtime_error("Invalid type for dynamic function call.");
  }

  Function fn = dynamic_fn->value.v_func;
  return fn.fn_ptr(argc, argv);
}

RuntimeObject *field_access(RuntimeObject *lhs, char *identifier) {
  // For built-in data types we have known hard-coded function names

  if (lhs->type == T_VECTOR) {
    if (strcmp(identifier, "length") == 0) {
      return make_function(vec_length_dynamic);
    } else if (strcmp(identifier, "append") == 0) {
      return make_function(vec_append_dynamic);
    }
  }

  if (lhs->type == T_STRING) {
    if (strcmp(identifier, "length") == 0) {
      return make_function(str_length_dynamic);
    }
  }

  if (lhs->type == T_DICT) {
    if (strcmp(identifier, "length") == 0) {
      return make_function(dict_length_dynamic);
    } else if (strcmp(identifier, "contains") == 0) {
      return make_function(dict_contains_dynamic);
    } else if (strcmp(identifier, "keys") == 0) {
      return make_function(dict_keys_dynamic);
    }
  }

  if (lhs->type == T_MODULE) {
    return runtime_st_lookup(&lhs->value.v_mod->table, identifier)->value;
  }

  runtime_error("invalid or unimplemented field access");

  // For modules (and later, classes) there will be a symbol table
  // injected in the runtime.
  return make_nothing();
}

/*
 * Implements index access (x[y]) at runtime.
 */
RuntimeObject *get_index(RuntimeObject *lhs, RuntimeObject *rhs) {
  if (lhs->type == T_VECTOR) {
    if (rhs->type != T_INT) {
      runtime_error("Vector index value must be int.");
    }
    int64_t index = rhs->value.v_int;

    if (index >= lhs->value.v_vec->size) {
      runtime_error("Vector index out of bounds.");
    }

    return &(lhs->value.v_vec->contents[index]);
  }

  if (lhs->type == T_STRING) {
    if (rhs->type != T_INT) {
      runtime_error("String index value must be int.");
    }
    int64_t index = rhs->value.v_int;
    if (index >= lhs->value.v_str->length) {
      runtime_error("String index out of bounds.");
    }

    char string_value[2] = " ";
    string_value[0] = lhs->value.v_str->contents[index];
    return make_string(string_value);
  }

  if (lhs->type == T_DICT) {
    String *key_hash = get_dict_key(rhs);
    RuntimeObject *maybe_result =
        dict_get(lhs->value.v_dict, key_hash->contents);

    if (maybe_result != NULL) {
      return maybe_result;
    }

    dict_set(lhs->value.v_dict, *key_hash, rhs, make_nothing());
    maybe_result = dict_get(lhs->value.v_dict, key_hash->contents);

    return maybe_result;
  }

  printf("LHS type: %d\n", lhs->type);
  runtime_error("Not impemented (get_index)");
  return make_nothing();
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
  printf("Runtime error: %s\n", msg);
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
  return make_nothing();
}

void _print_vector(Vector *vec);

void _print_helper(RuntimeObject *obj) {
  // prints object with no newline
  switch (obj->type) {
  case T_NOTHING:
    printf(STRING_NOTHING);
    return;
  case T_BOOL:
    printf(obj->value.v_bool ? STRING_TRUE : STRING_FALSE);
    return;
  case T_INT:
    printf("%lld", obj->value.v_int);
    return;
  case T_FLOAT:
    printf("%.1f", obj->value.v_float);
    return;
  case T_STRING:
    printf("%s", obj->value.v_str->contents);
    return;
  case T_VECTOR:
    _print_vector(obj->value.v_vec);
    return;
  case T_DICT:
    printf("%s", to_string_raw(obj)->contents);
    return;
  case T_FUNCTION: {
    char *signature = obj->value.v_func.signature;
    signature = signature == NULL ? "(Signature Unknown)" : signature;
    printf("function:%s", signature);
    return;
  }
  case T_MODULE:
  default:
    break;
  }
  printf("NOT IMPLEMENTED");
  exit(1);
}

void _print_vector(Vector *vec) {
  printf("[");
  int i = 0;
  int length = vec->size;
  while (i < length) {
    RuntimeObject *elem = &(vec->contents[i]);
    bool is_str = elem->type == T_STRING;
    if (is_str) {
      printf("\"");
    }
    _print_helper(elem);
    if (is_str) {
      printf("\"");
    }
    if (i != length - 1) {
      printf(", ");
    }
    ++i;
  }

  printf("]");
}

void _dict_put(RuntimeObject *dict, RuntimeObject *key, RuntimeObject *value) {
  // make sure dict is a dict
  if (dict->type != T_DICT) {
    runtime_error("_dict_put called on a non-dictionary object.");
  }

  // Check if hashable, and get hash
  String *key_hash = get_dict_key(key);

  // create and insert dictionary entry
  dict_set(dict->value.v_dict, *key_hash, key, value);
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
  vec->contents[vec->size - 1] = *obj;
  return make_nothing();
}

RuntimeObject *vec_length_dynamic(size_t argc, RuntimeObject *argv[]) {
  if (argc != 1) {
    runtime_error("Argument number mismatch for vec length");
  }
  return vec_length(argv[0]);
}

RuntimeObject *vec_append_dynamic(size_t argc, RuntimeObject *argv[]) {
  if (argc != 2) {
    runtime_error("Argument number mismatch for vec append./");
  }
  return vec_append(argv[0], argv[1]);
}

RuntimeObject *str_length(RuntimeObject *self) {
  return make_int(self->value.v_str->length);
}

RuntimeObject *str_length_dynamic(size_t argc, RuntimeObject *argv[]) {
  if (argc != 1) {
    runtime_error("Argument number mismatch for string length");
  }
  return str_length(argv[0]);
}

RuntimeObject *dict_length(RuntimeObject *self) {
  return make_int(self->value.v_dict->size);
}

RuntimeObject *dict_keys_raw(Dict *dict) {
  RuntimeObject *result_vec = make_vector();
  for (size_t i = 0; i < dict->capacity; ++i) {
    if (dict->entries[i].key_hash.contents != NULL) {
      DictEntry *entry = &dict->entries[i];
      vec_append(result_vec, entry->key);
    }
  }
  return result_vec;
}

RuntimeObject *dict_keys(RuntimeObject *self) {
  Dict *dict = self->value.v_dict;
  return dict_keys_raw(dict);
}

RuntimeObject *dict_contains(RuntimeObject *self, RuntimeObject *key) {
  Dict *dict = self->value.v_dict;
  String *hash_key = get_dict_key(key);
  RuntimeObject *result = dict_get(dict, hash_key->contents);
  return make_bool(result != NULL);
}

RuntimeObject *dict_length_dynamic(size_t argc, RuntimeObject *argv[]) {
  if (argc != 1) {
    runtime_error("Argument number mismatch for dict length");
  }
  return dict_length(argv[0]);
}

RuntimeObject *dict_keys_dynamic(size_t argc, RuntimeObject *argv[]) {
  if (argc != 1) {
    runtime_error("Argument number mismatch for dict keys");
  }
  return dict_keys(argv[0]);
}

RuntimeObject *dict_contains_dynamic(size_t argc, RuntimeObject *argv[]) {
  if (argc != 2) {
    runtime_error("Argument number mismatch for dict contains");
  }
  return dict_contains(argv[0], argv[1]);
}

RuntimeSymbolTableEntry *runtime_st_lookup(RuntimeSymbolTable *st,
                                           char *identifier) {
  // dead simple linear lookup
  for (size_t i = 0; i < st->size; ++i) {
    RuntimeSymbolTableEntry *p = &st->entries[i];
    if (strcmp(p->name, identifier) == 0) {
      return p;
    }
  }

  runtime_error("Bad module lookup, identifier not found.");
  return NULL;
}