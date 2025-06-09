#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "datatype.h"

#define max(a, b) a > b ? a : b;

Vector *make_empty_vector() {
  Vector *vec = malloc(sizeof(Vector));
  vec->size = 0;
  vec->internal_size = VEC_INITIAL_SIZE;
  vec->contents = calloc(VEC_INITIAL_SIZE, sizeof(RuntimeObject));
  // for(size_t i = 0; i < VEC_INITIAL_SIZE; ++i) {
  //     vec->contents[i].type = T_NOTHING;
  //     vec->contents[i].value.v_bool = false;
  // }
  return vec;
}

RuntimeObject *make_vector_known_size(size_t size) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_VECTOR;
  Vector *vec = malloc(sizeof(Vector));
  size_t final_size = max(size, VEC_INITIAL_SIZE);
  vec->size = size;
  vec->internal_size = final_size;
  vec->contents = calloc(final_size, sizeof(RuntimeObject));

  obj->value.v_vec = vec;
  return obj;
}

Dict *make_empty_dict() {
  Dict *dict = malloc(sizeof(Dict));
  dict->size = 0;
  dict->capacity = DICT_INITIAL_SIZE;
  dict->entries = calloc(DICT_INITIAL_SIZE, sizeof(DictEntry));
  return dict;
}

RuntimeObject *make_nothing() {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_NOTHING;
  obj->value.v_bool = false;
  return obj;
}

RuntimeObject *make_bool(bool value) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_BOOL;
  obj->value.v_bool = value;
  return obj;
}

RuntimeObject *make_int(int64_t value) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_INT;
  obj->value.v_int = value;
  return obj;
}

RuntimeObject *make_float(double value) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_FLOAT;
  obj->value.v_float = value;
  return obj;
}

RuntimeObject *make_string(char *value) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_STRING;
  String *str = malloc(sizeof(String));
  str->length = strlen(value);
  str->contents = strdup(value);
  obj->value.v_str = str;
  return obj;
}

RuntimeObject *make_string_nocopy(char *value) {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_STRING;
  String *str = malloc(sizeof(String));
  str->length = strlen(value);
  str->contents = value;
  obj->value.v_str = str;
  return obj;
}

RuntimeObject *make_vector() {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_VECTOR;
  obj->value.v_vec = make_empty_vector();
  return obj;
}

RuntimeObject *make_dict() {
  RuntimeObject *obj = malloc(sizeof(RuntimeObject));
  obj->type = T_DICT;
  obj->value.v_dict = make_empty_dict();
  return obj;
}