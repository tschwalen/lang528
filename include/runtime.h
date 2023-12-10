#pragma once

#include "interpreter.h"
#include "tokentype.h"

BoxedValue apply_binary_operator(TokenType op, BoxedValue lhs, BoxedValue rhs);
BoxedValue apply_unary_operator(TokenType op, BoxedValue rhs);