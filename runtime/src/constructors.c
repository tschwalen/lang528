#include <stdlib.h>
#include <stdbool.h>

#include "datatype.h"

RuntimeObject* make_nothing() {
    RuntimeObject* obj = malloc(sizeof(RuntimeObject));
    obj->type = T_NOTHING;
    obj->value.v_bool = false;
    return obj;
}

RuntimeObject* make_bool(bool value) {
    RuntimeObject* obj = malloc(sizeof(RuntimeObject));
    obj->type = T_BOOL;
    obj->value.v_bool = value;
    return obj;
}

RuntimeObject* make_int(int64_t value) {
    RuntimeObject* obj = malloc(sizeof(RuntimeObject));
    obj->type = T_INT;
    obj->value.v_int = value;
    return obj;
}

RuntimeObject* make_float(double value) {
    RuntimeObject* obj = malloc(sizeof(RuntimeObject));
    obj->type = T_FLOAT;
    obj->value.v_float = value;
    return obj;
}