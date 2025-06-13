#pragma once
#include "datatype.h"
#include <stdbool.h>

int placeholder(int x);
bool get_conditional_result(RuntimeObject *obj);

void builtin_print(RuntimeObject *arg);
void runtime_error(char *msg);

RuntimeObject *make_argv(int argc, char *argv[]);

RuntimeObject *get_index(RuntimeObject *lhs, RuntimeObject *rhs);

// ARITHMETIC OP + STRCAT
RuntimeObject *op_add(RuntimeObject *lhs, RuntimeObject *rhs);
RuntimeObject *op_sub(RuntimeObject *lhs, RuntimeObject *rhs);
RuntimeObject *op_mul(RuntimeObject *lhs, RuntimeObject *rhs);
RuntimeObject *op_div(RuntimeObject *lhs, RuntimeObject *rhs);
RuntimeObject *op_mod(RuntimeObject *lhs, RuntimeObject *rhs);

// COMPARISON OP
RuntimeObject *op_eq(RuntimeObject *lhs, RuntimeObject *rhs);
RuntimeObject *op_neq(RuntimeObject *lhs, RuntimeObject *rhs);
RuntimeObject *op_leq(RuntimeObject *lhs, RuntimeObject *rhs);
RuntimeObject *op_geq(RuntimeObject *lhs, RuntimeObject *rhs);
RuntimeObject *op_lt(RuntimeObject *lhs, RuntimeObject *rhs);
RuntimeObject *op_gt(RuntimeObject *lhs, RuntimeObject *rhs);

// BOOLEAN OP
RuntimeObject *op_and(RuntimeObject *lhs, RuntimeObject *rhs);
RuntimeObject *op_or(RuntimeObject *lhs, RuntimeObject *rhs);

// UNARY OP
RuntimeObject *op_unot(RuntimeObject *rhs);
RuntimeObject *op_umin(RuntimeObject *rhs);

// CONSTRUCTORS
RuntimeObject *make_nothing();
RuntimeObject *make_bool(bool value);
RuntimeObject *make_int(int64_t value);
RuntimeObject *make_float(double value);
RuntimeObject *make_string(char *value);
RuntimeObject *make_string_nocopy(char *value);
RuntimeObject *make_vector();
RuntimeObject *make_dict();
RuntimeObject *make_vector_known_size(size_t size);