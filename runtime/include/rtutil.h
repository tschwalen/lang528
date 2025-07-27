#pragma once

#include "datatype.h"

char *strdupcat(const char *str1, const char *str2);

String *make_string_raw(char *value);

String *make_string_nocopy_raw(char *value);

String *str_concat_raw(String *lhs, String *rhs);

String *to_string_raw(RuntimeObject *obj);

String *get_dict_key(RuntimeObject *key);
