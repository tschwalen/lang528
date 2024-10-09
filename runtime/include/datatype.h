#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


#define VEC_INITIAL_SIZE 4
#define DICT_INITIAL_SIZE 16

typedef struct RuntimeObject RuntimeObject;

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

typedef struct {
    size_t length;
    char* contents;
} String;

typedef struct {
    size_t size;
    size_t internal_size;
    RuntimeObject* contents; 
} Vector;

typedef struct {
    String key;
    RuntimeObject* value;
} DictEntry;

typedef struct {
    DictEntry* entries;
    size_t capacity;
    size_t size;
} Dict;

struct RuntimeObject {
    enum DataType type;
    union {
        bool    v_bool;
        int64_t v_int;
        double  v_float;
        String* v_str;
        Vector* v_vec;
        Dict*   v_dict;
    } value;
};