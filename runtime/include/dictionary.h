#pragma once
#include <stdint.h>

#include "datatype.h"

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// static uint64_t hash_key(const char* key);

RuntimeObject* dict_get(Dict* dict, const char* key);

const char* dict_set(Dict* dict, const char* key, RuntimeObject* value);