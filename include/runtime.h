#pragma once

#include <stdexcept>

#include "interpreter.h"
#include "tokentype.h"

class ArithmeticOp {
protected:
    virtual int   apply_raw(int lhs, int rhs) = 0;
    virtual float apply_raw(int lhs, float rhs) = 0;
    virtual float apply_raw(float lhs, int rhs) = 0;
    virtual float apply_raw(float lhs, float rhs) = 0;

public:
    BoxedValue apply(BoxedValue lhs, BoxedValue rhs) {
        if(lhs.type == DataType::INT) {
            if(rhs.type == DataType::INT) {
                return BoxedValue {
                    DataType::INT,
                    this->apply_raw(std::get<int>(lhs.value), std::get<int>(rhs.value))
                };
            }
            if(rhs.type == DataType::FLOAT) {
                return BoxedValue {
                    DataType::FLOAT,
                    this->apply_raw(std::get<int>(lhs.value), std::get<float>(rhs.value))
                };
            }
        }
        if(lhs.type == DataType::FLOAT) {
            if(rhs.type == DataType::INT) {
                return BoxedValue {
                    DataType::FLOAT,
                    this->apply_raw(std::get<float>(lhs.value), std::get<int>(rhs.value))
                };
            }
            if(rhs.type == DataType::FLOAT) {
                return BoxedValue {
                    DataType::FLOAT,
                    this->apply_raw(std::get<float>(lhs.value), std::get<float>(rhs.value))
                };
            }
        }

        throw std::runtime_error("Arithmetic Operators are only supported between numeric types");
    }
};

class Multiply : public ArithmeticOp {
    int   apply_raw(int lhs, int rhs) override {return lhs * rhs;}
    float apply_raw(int lhs, float rhs) override {return lhs * rhs;}
    float apply_raw(float lhs, int rhs) override {return lhs * rhs;}
    float apply_raw(float lhs, float rhs) override {return lhs * rhs;}
};

class Add : public ArithmeticOp {
    int   apply_raw(int lhs, int rhs) override {return lhs + rhs;}
    float apply_raw(int lhs, float rhs) override {return lhs + rhs;}
    float apply_raw(float lhs, int rhs) override {return lhs + rhs;}
    float apply_raw(float lhs, float rhs) override {return lhs + rhs;}
};

class Subtract : public ArithmeticOp {
    int   apply_raw(int lhs, int rhs) override {return lhs - rhs;}
    float apply_raw(int lhs, float rhs) override {return lhs - rhs;}
    float apply_raw(float lhs, int rhs) override {return lhs - rhs;}
    float apply_raw(float lhs, float rhs) override {return lhs - rhs;}
};

class Divide : public ArithmeticOp {
    int   apply_raw(int lhs, int rhs) override {return lhs / rhs;}
    float apply_raw(int lhs, float rhs) override {return lhs / rhs;}
    float apply_raw(float lhs, int rhs) override {return lhs / rhs;}
    float apply_raw(float lhs, float rhs) override {return lhs / rhs;}
};

BoxedValue apply_binary_operator(TokenType op, BoxedValue lhs, BoxedValue rhs);
BoxedValue apply_unary_operator(TokenType op, BoxedValue rhs);