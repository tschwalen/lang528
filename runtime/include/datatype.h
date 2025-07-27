#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define VEC_INITIAL_SIZE 16
#define DICT_INITIAL_SIZE 16

typedef struct RuntimeObject RuntimeObject;

enum DataType {
  T_NOTHING,
  T_BOOL,
  T_FLOAT,
  T_INT,
  T_STRING,
  T_VECTOR,
  T_DICT,
  T_MODULE,  // TBD
  T_FUNCTION // TBD
};

typedef struct {
  size_t length;
  char *contents;
} String;

typedef struct {
  size_t size;
  size_t internal_size;
  RuntimeObject *contents;
} Vector;

typedef struct {
  String key_hash;
  RuntimeObject *key;
  RuntimeObject *value;
} DictEntry;

typedef struct {
  DictEntry *entries;
  size_t capacity; // internal size
  size_t size;     // utilized size
} Dict;

typedef struct {
  // potentially other things like number and names of args.
  RuntimeObject *(*fn_ptr)(size_t argc, RuntimeObject *argv[]);
  char *signature;
} Function;

struct RuntimeObject {
  enum DataType type;
  union {
    bool v_bool;
    int64_t v_int;
    double v_float;
    String *v_str;
    Vector *v_vec;
    Dict *v_dict;
    Function v_func;
  } value;
};