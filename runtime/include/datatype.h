#include <stdint.h>
#include <stddef.h>

enum DataType {
    T_NOTHING,
    T_BOOL,
    T_FLOAT,
    T_INT,
    T_STRING,
    T_VECTOR,
    T_DICT,
    T_MODULE,
    T_FUNCTION
};

typedef struct String {
    size_t length;
    char* contents;
} String;

typedef struct RuntimeObject {
    enum DataType type;
    union {
        int64_t v_int;
        double  v_float;
        String* v_str;
    } value;
} RuntimeObject;