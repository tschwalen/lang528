#include <stdlib.h>
#include <string.h>

#include "datatype.h"
#include "dictionary.h"

static uint64_t hash_key(const char *key) {
  uint64_t hash = FNV_OFFSET;
  for (const char *p = key; *p; p++) {
    hash ^= (uint64_t)(unsigned char)(*p);
    hash *= FNV_PRIME;
  }
  return hash;
}

RuntimeObject *dict_get(Dict *dict, const char *key) {
  // AND hash with capacity-1 to ensure it's within entries array.
  uint64_t hash = hash_key(key);
  size_t index = (size_t)(hash & (uint64_t)(dict->capacity - 1));

  // Loop till we find an empty entry.
  while (dict->entries[index].key_hash.contents != NULL) {
    if (strcmp(key, dict->entries[index].key_hash.contents) == 0) {
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

DictEntry *dict_get_entry(Dict *dict, const char *key) {
  // AND hash with capacity-1 to ensure it's within entries array.
  uint64_t hash = hash_key(key);
  size_t index = (size_t)(hash & (uint64_t)(dict->capacity - 1));

  // Loop till we find an empty entry.
  while (dict->entries[index].key_hash.contents != NULL) {
    if (strcmp(key, dict->entries[index].key_hash.contents) == 0) {
      return &dict->entries[index];
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

static const char *dict_set_entry(DictEntry *entries, size_t capacity,
                                  const char *key_hash, RuntimeObject *key,
                                  RuntimeObject *value, size_t *plength) {
  // AND hash with capacity-1 to ensure it's within entries array.
  uint64_t hash = hash_key(key_hash);
  size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

  // Loop till we find an empty entry.
  while (entries[index].key_hash.contents != NULL) {
    if (strcmp(key_hash, entries[index].key_hash.contents) == 0) {
      // Found key (it already exists), update value.
      entries[index].value = value;
      return entries[index].key_hash.contents;
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
    key_hash = strdup(key_hash);
    if (key_hash == NULL) {
      return NULL;
    }
    (*plength)++;
  }
  entries[index].key_hash.contents = (char *)key_hash;
  entries[index].key_hash.length = strlen(key_hash);
  entries[index].key = key;
  entries[index].value = value;
  return key_hash;
}

// Expand hash table to twice its current size. Return true on success,
// false if out of memory.
static bool dict_expand(Dict *table) {
  // Allocate new entries array.
  size_t new_capacity = table->capacity * 2;
  if (new_capacity < table->capacity) {
    return false; // overflow (capacity would be too big)
  }
  DictEntry *new_entries = calloc(new_capacity, sizeof(DictEntry));
  if (new_entries == NULL) {
    return false;
  }
  for (size_t i = 0; i < new_capacity; ++i) {
    new_entries[i].key_hash.contents = NULL;
  }

  // Iterate entries, move all non-empty ones to new table's entries.
  for (size_t i = 0; i < table->capacity; i++) {
    DictEntry *entry = &(table->entries[i]);
    if (entry->key_hash.contents != NULL) {
      dict_set_entry(new_entries, new_capacity, entry->key_hash.contents,
                     entry->key, entry->value, NULL);
    }
  }

  // Free old entries array and update this table's details.
  free(table->entries);
  table->entries = new_entries;
  table->capacity = new_capacity;
  return true;
}

const char *dict_set(Dict *dict, String key_hash, RuntimeObject *key,
                     RuntimeObject *value) {
  if (value == NULL || key == NULL) {
    return NULL;
  }

  if (dict->size >= dict->capacity / 2) {
    if (!dict_expand(dict)) {
      return NULL;
    }
  }

  return dict_set_entry(dict->entries, dict->capacity, key_hash.contents, key,
                        value, &(dict->size));
}