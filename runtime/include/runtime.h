#pragma once

#include "datatype.h"

int placeholder(int x);

void builtin_print(RuntimeObject *arg);

RuntimeObject *op_add(RuntimeObject *lhs, RuntimeObject *rhs);

RuntimeObject *make_nothing();

RuntimeObject *make_bool(bool value);

RuntimeObject *make_int(int64_t value);

RuntimeObject *make_float(double value);

RuntimeObject *make_string(char *value);

RuntimeObject *make_vector();

RuntimeObject *make_dict();