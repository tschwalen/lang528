#pragma once

#include <memory>
#include <stdexcept>

#include "interpreter.h"
#include "tokentype.h"

class ArithmeticCompBinOp {
protected:
    virtual bool apply_raw(int lhs, int rhs) = 0;
    virtual bool apply_raw(int lhs, float rhs) = 0;
    virtual bool apply_raw(float lhs, int rhs) = 0;
    virtual bool apply_raw(float lhs, float rhs) = 0;

public:
    BoxedValue apply(BoxedValue lhs, BoxedValue rhs);
};

class Less : public ArithmeticCompBinOp {
    bool apply_raw(int lhs, int rhs) override {return lhs < rhs;}
    bool apply_raw(int lhs, float rhs) override {return lhs < rhs;}
    bool apply_raw(float lhs, int rhs) override {return lhs < rhs;}
    bool apply_raw(float lhs, float rhs) override {return lhs < rhs;}
};

class Greater : public ArithmeticCompBinOp {
    bool apply_raw(int lhs, int rhs) override {return lhs > rhs;}
    bool apply_raw(int lhs, float rhs) override {return lhs > rhs;}
    bool apply_raw(float lhs, int rhs) override {return lhs > rhs;}
    bool apply_raw(float lhs, float rhs) override {return lhs > rhs;}
};

class LessEqual : public ArithmeticCompBinOp {
    bool apply_raw(int lhs, int rhs) override {return lhs <= rhs;}
    bool apply_raw(int lhs, float rhs) override {return lhs <= rhs;}
    bool apply_raw(float lhs, int rhs) override {return lhs <= rhs;}
    bool apply_raw(float lhs, float rhs) override {return lhs <= rhs;}
};

class GreaterEqual : public ArithmeticCompBinOp {
    bool apply_raw(int lhs, int rhs) override {return lhs >= rhs;}
    bool apply_raw(int lhs, float rhs) override {return lhs >= rhs;}
    bool apply_raw(float lhs, int rhs) override {return lhs >= rhs;}
    bool apply_raw(float lhs, float rhs) override {return lhs >= rhs;}
};

class ArithmeticBinOp {
protected:
    virtual int   apply_raw(int lhs, int rhs) = 0;
    virtual float apply_raw(int lhs, float rhs) = 0;
    virtual float apply_raw(float lhs, int rhs) = 0;
    virtual float apply_raw(float lhs, float rhs) = 0;

public:
    BoxedValue apply(BoxedValue lhs, BoxedValue rhs);
};

class Multiply : public ArithmeticBinOp {
    int   apply_raw(int lhs, int rhs) override {return lhs * rhs;}
    float apply_raw(int lhs, float rhs) override {return lhs * rhs;}
    float apply_raw(float lhs, int rhs) override {return lhs * rhs;}
    float apply_raw(float lhs, float rhs) override {return lhs * rhs;}
};

class Add : public ArithmeticBinOp {
    int   apply_raw(int lhs, int rhs) override {return lhs + rhs;}
    float apply_raw(int lhs, float rhs) override {return lhs + rhs;}
    float apply_raw(float lhs, int rhs) override {return lhs + rhs;}
    float apply_raw(float lhs, float rhs) override {return lhs + rhs;}
};

class Subtract : public ArithmeticBinOp {
    int   apply_raw(int lhs, int rhs) override {return lhs - rhs;}
    float apply_raw(int lhs, float rhs) override {return lhs - rhs;}
    float apply_raw(float lhs, int rhs) override {return lhs - rhs;}
    float apply_raw(float lhs, float rhs) override {return lhs - rhs;}
};

class Divide : public ArithmeticBinOp {
    int   apply_raw(int lhs, int rhs) override {return lhs / rhs;}
    float apply_raw(int lhs, float rhs) override {return lhs / rhs;}
    float apply_raw(float lhs, int rhs) override {return lhs / rhs;}
    float apply_raw(float lhs, float rhs) override {return lhs / rhs;}
};

void runtime_assertion(bool condition, string message);

BoxedValue apply_binary_operator(TokenType op, BoxedValue lhs, BoxedValue rhs);
BoxedValue apply_unary_operator(TokenType op, BoxedValue rhs);
string getDictKey(BoxedValue bv) ;

void builtin_print(BoxedValue arg);
BoxedValue builtin_vector_length(BoxedValue arg);
BoxedValue builtin_string_length(BoxedValue arg);
BoxedValue builtin_dict_length(BoxedValue arg);
BoxedValue builtin_dict_keys(BoxedValue arg);
BoxedValue builtin_dict_contains(BoxedValue arg, BoxedValue key);
void builtin_vector_append(BoxedValue vec, BoxedValue elem);
