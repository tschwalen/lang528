#include <string.h>
#include <stdlib.h>

#include "dictionary.h"
#include "datatype.h"

static uint64_t hash_key(const char* key) {
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

RuntimeObject* dict_get(Dict* dict, const char* key) {
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(dict->capacity - 1));

    // Loop till we find an empty entry.
    while (dict->entries[index].key.contents != NULL) {
        if (strcmp(key, dict->entries[index].key.contents) == 0) {
            return dict->entries[index].value;
        }
        // Key wasn't in this slot, move to next (linear probing).
        index++;
        if (index >= dict->capacity) {
            // At end of entries array, wrap around.
            index = 0;
        }
    }
    return NULL;
}

static const char* dict_set_entry(DictEntry* entries, size_t capacity,
        const char* key, RuntimeObject* value, size_t* plength) {
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    // Loop till we find an empty entry.
    while (entries[index].key.contents != NULL) {
        if (strcmp(key, entries[index].key.contents) == 0) {
            // Found key (it already exists), update value.
            entries[index].value = value;
            return entries[index].key.contents;
        }
        // Key wasn't in this slot, move to next (linear probing).
        index++;
        if (index >= capacity) {
            // At end of entries array, wrap around.
            index = 0;
        }
    }

    // Didn't find key, allocate+copy if needed, then insert it.
    if (plength != NULL) {
        key = strdup(key);
        if (key == NULL) {
            return NULL;
        }
        (*plength)++;
    }
    entries[index].key.contents = (char*) key;
    entries[index].key.length = strlen(key);
    entries[index].value = value;
    return key;
}

// Expand hash table to twice its current size. Return true on success,
// false if out of memory.
static bool dict_expand(Dict* table) {
    // Allocate new entries array.
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) {
        return false;  // overflow (capacity would be too big)
    }
    DictEntry* new_entries = calloc(new_capacity, sizeof(DictEntry));
    if (new_entries == NULL) {
        return false;
    }

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < table->capacity; i++) {
        DictEntry* entry = &(table->entries[i]);
        if (entry->key.contents != NULL) {
            dict_set_entry(new_entries, new_capacity, entry->key.contents,
                         entry->value, NULL);
        }
    }

    // Free old entries array and update this table's details.
    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
    return true;
}

const char* dict_set(Dict* dict, const char* key, RuntimeObject* value) {
    if (value == NULL) {
        return NULL;
    }

    // If length will exceed half of current capacity, expand it.
    if (dict->size >= dict->capacity / 2) {
        if (!dict_expand(dict)) {
            return NULL;
        }
    }

    // Set entry and update length.
    return dict_set_entry(dict->entries, dict->capacity, key, value, &(dict->size));
}