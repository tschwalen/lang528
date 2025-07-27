#include "datatype.h"
#include "dictionary.h"
#include "rtutil.h"
#include "runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

String *make_string_raw(char *value) {
  String *str = malloc(sizeof(String));
  str->length = strlen(value);
  str->contents = strdup(value);
  return str;
}

String *make_string_nocopy_raw(char *value) {
  String *str = malloc(sizeof(String));
  str->length = strlen(value);
  str->contents = value;
  return str;
}

String *str_concat_raw(String *lhs, String *rhs) {
  char *new_str = strdupcat(lhs->contents, rhs->contents);
  return make_string_nocopy_raw(new_str);
}

String *to_string_raw(RuntimeObject *obj) {
  char buffer[256];
  switch (obj->type) {
  case T_NOTHING: {
    return make_string_raw("nothing");
  } break;
  case T_BOOL: {
    return obj->value.v_bool ? make_string_raw("true")
                             : make_string_raw("false");
  } break;
  case T_FLOAT: {
    snprintf(buffer, sizeof(buffer), "%.1f", obj->value.v_float);
    return make_string_raw(buffer);
    break;
  } break;
  case T_INT: {
    snprintf(buffer, sizeof(buffer), "%lld", obj->value.v_int);
    return make_string_raw(buffer);
  } break;
  case T_STRING: {
    return obj->value.v_str;
  } break;
  case T_VECTOR: {
    // start a string accumulator, with an opening bracket
    String *acc = make_string_raw("[");

    size_t length = obj->value.v_vec->size;
    RuntimeObject *vec = obj->value.v_vec->contents;

    for (size_t i = 0; i < length; ++i) {
      RuntimeObject *elem = &vec[i];

      // check if elem is a string
      bool is_str = elem->type == T_STRING;

      // add opening quote if string
      if (is_str) {
        acc = str_concat_raw(acc, make_string_raw("\""));
      }

      acc = str_concat_raw(acc, to_string_raw(elem));

      // add closing quote if string
      if (is_str) {
        acc = str_concat_raw(acc, make_string_raw("\""));
      }

      // add comma separator unless we're on the last element
      if (i != length - 1) {
        acc = str_concat_raw(acc, make_string_raw(", "));
      }
    }

    // add the closing bracket and then return
    acc = str_concat_raw(acc, make_string_raw("]"));
    return acc;
    break;
  } break;
  case T_DICT: {
    // start a string accumulator, with an opening bracket
    String *acc = make_string_raw("{");
    Vector *keys = dict_keys(obj)->value.v_vec;

    for (size_t i = 0; i < keys->size; ++i) {
      RuntimeObject *key_obj = &keys->contents[i];
      String *key_hash = get_dict_key(key_obj);
      RuntimeObject *value_obj =
          dict_get(obj->value.v_dict, key_hash->contents);

      bool key_is_str = key_obj->type == T_STRING;
      bool value_is_str = value_obj->type == T_STRING;

      if (key_is_str) {
        acc = str_concat_raw(acc, make_string_raw("\""));
      }
      acc = str_concat_raw(acc, to_string_raw(key_obj));
      if (key_is_str) {
        acc = str_concat_raw(acc, make_string_raw("\""));
      }
      acc = str_concat_raw(acc, make_string_raw(": "));

      if (value_is_str) {
        acc = str_concat_raw(acc, make_string_raw("\""));
      }
      acc = str_concat_raw(acc, to_string_raw(value_obj));
      if (value_is_str) {
        acc = str_concat_raw(acc, make_string_raw("\""));
      }

      // add comma separator unless we're on the last element
      if (i != keys->size - 1) {
        acc = str_concat_raw(acc, make_string_raw(", "));
      }
    }
    // add the closing bracket and then return
    acc = str_concat_raw(acc, make_string_raw("}"));
    return acc;
  } break;
  case T_MODULE: {
  } break;
  case T_FUNCTION: {
    char *signature = obj->value.v_func.signature;
    signature = signature == NULL ? "(Signature Unknown)" : signature;
    return make_string_raw(strdupcat("function:", signature));
  } break;
  }
  return make_string_raw("NOT_IMPLEMENTED");
}
