#include <stdexcept>

#include "runtime.h"
#include "tokentype.h"
#include "interpreter.h"

/* 
Binary Operator valid type defitions

op          lhs         rhs
+           int/float   int/float
+           string      string/int/float/bool/vec
/           int/float   int/float
-           int/float   int/float
*           int/float   int/float
%           int         int
==          [any]       [any]
!=          [any]       [any]
<=          int/float   int/float
>=          int/float   int/float
<           int/float   int/float
>           int/float   int/float
()          function    expr-list                                       -- handled elsewhere --
[]          vec         int         (in future: lhs=indexable, rhs=key) -- handled elsewhere --
.           [any]       identifier                                      -- handled elsewhere --
&           bool        bool
|           bool        bool

Unary operator valid type definitions
op          rhs
-           int/float
!           bool

*/

static Multiply multiply;
static Add add;
static Subtract subtract;
static Divide divide;

BoxedValue apply_times(BoxedValue lhs, BoxedValue rhs) {
    return multiply.apply(lhs, rhs);
}

BoxedValue apply_div(BoxedValue lhs, BoxedValue rhs) {
    return divide.apply(lhs, rhs);
}

BoxedValue apply_minus(BoxedValue lhs, BoxedValue rhs) {
    return subtract.apply(lhs, rhs);
}

BoxedValue apply_plus(BoxedValue lhs, BoxedValue rhs) {
    if (lhs.type == DataType::STRING) {

    }
    return add.apply(lhs, rhs);
}

BoxedValue apply_mod(BoxedValue lhs, BoxedValue rhs) {}

BoxedValue apply_equals(BoxedValue lhs, BoxedValue rhs) {}

BoxedValue apply_not_equals(BoxedValue lhs, BoxedValue rhs) {}

BoxedValue apply_less_equals(BoxedValue lhs, BoxedValue rhs) {}

BoxedValue apply_greater_equals(BoxedValue lhs, BoxedValue rhs) {}

BoxedValue apply_less(BoxedValue lhs, BoxedValue rhs) {}

BoxedValue apply_greater(BoxedValue lhs, BoxedValue rhs) {}

BoxedValue apply_and(BoxedValue lhs, BoxedValue rhs) {}

BoxedValue apply_or(BoxedValue lhs, BoxedValue rhs) {}

BoxedValue apply_binary_operator(TokenType op, BoxedValue lhs, BoxedValue rhs) {
    switch(op) {
        case TokenType::PLUS:
            return apply_plus(lhs, rhs);
        case TokenType::MINUS:
            return apply_minus(lhs, rhs);
        case TokenType::TIMES:
            return apply_times(lhs, rhs); 
        case TokenType::DIV:
            return apply_div(lhs, rhs);
        case TokenType::MOD: 
            return apply_mod(lhs, rhs);
        case TokenType::EQUALS_EQUALS:
            return apply_equals(lhs, rhs);
        case TokenType::NOT_EQUALS:
            return apply_not_equals(lhs, rhs);
        case TokenType::LESS_EQUALS:
            return apply_less_equals(lhs, rhs);
        case TokenType::GREATER_EQUALS:
            return apply_greater_equals(lhs, rhs);
        case TokenType::LESS:
            return apply_less(lhs, rhs);
        case TokenType::GREATER:
            return apply_greater(lhs, rhs);
        case TokenType::AND:
            return apply_and(lhs, rhs);
        case TokenType::OR:
            return apply_or(lhs, rhs);
        default: break;
    }
    throw std::runtime_error("TokenType argument op must be a binary operator");
}

BoxedValue apply_unary_operator(TokenType op, BoxedValue rhs);