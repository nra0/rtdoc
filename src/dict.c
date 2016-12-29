#include "dict.h"
#include "list.h"

#include <stdlib.h>


#define DICT_NUM_BUCKETS_INITIAL 256
#define DICT_REHASH_CAPACITY     0.75


/**********************************************************************
 *                     Struct definitions.
 *********************************************************************/

/*
 * A basic hashmap with string keys and generic values.
 *
 * The caller should instantiate methods to copy, free, and check equality.
 */
struct Dict {
  unsigned int size;        /* How many entries are in the dict. */
  unsigned int numBuckets;  /* The number of buckets. */
  List **buckets;           /* Linked lists to chain values with the same hash. */
  void *(*copy)(void *value);
  void (*free)(void *value);
  int (*equals)(void *value1, void *value2);
};


/**********************************************************************
 *                       Memory management.
 *********************************************************************/

/*
 * Create a new dictionary.
 *
 * @return The newly created dict.
 */
Dict *dictCreate(void) {
  return NULL;
}

/*
 * Free an existing dict.
 *
 * @param dict: The dictionary to free.
 */
void dictFree(Dict *dict) {

}

/*
 * Set the function to copy an inserted value.
 */
void dictSetCopyFn(Dict *dict, void *(*copy)(void *value)) {
  dict->copy = copy;
}

/*
 * Set the function to free a value.
 */
void dictSetFreeFn(Dict *dict, void (*free)(void *value)) {
  dict->free = free;
}

/*
 * Set the equals function to compare two values.
 */
void dictSetEqualsFn(Dict *dict, int (*equals)(void *value1, void *value2)) {
  dict->equals = equals;
}


/**********************************************************************
 *                        Dictionary methods.
 *********************************************************************/

/*
 * Get the number of entries in the dictionary.
 *
 * @param dict: The dict whose size to get.
 * @return The number of entries in the dict.
 */
inline unsigned int dictSize(const Dict *dict) {
  return dict->size;
}

/*
 * Set a new value, or update an existing one.
 *
 * @param dict: The dictionary to update.
 * @param key: The lookup key for the value.
 * @param value: The value to insert.
 * @return The updated dictionary.
 */
Dict *dictSet(Dict *dict, char *key, void *value) {
  return NULL;
}

/*
 * Get the value for an existing key.
 *
 * @param dict: The dictionary to lookup.
 * @param key: The key corresponding to the value.
 * @return The value at the key.
 */
void *dictGet(const Dict *dict, char *key) {
  return NULL;
}
