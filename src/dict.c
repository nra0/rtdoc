#include "dict.h"
#include "list.h"
#include "mmalloc.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


#define DICT_NUM_BUCKETS_INITIAL 8
#define DICT_REHASH_CAPACITY     0.75


/**********************************************************************
 *                     Struct definitions.
 *********************************************************************/

/*
 * A basic hashmap with string keys and generic values.
 *
 * The caller should instantiate methods to free the value.
 */
struct Dict {
  unsigned int size;          /* How many entries are in the dict. */
  unsigned int numBuckets;    /* The number of buckets. */
  List **buckets;             /* Linked lists to chain values with the same hash. */
  void (*free)(void *value);
};

/*
 * A key/value entry placed in bucket chains.
 */
typedef struct DictEntry {
  char *key;                  /* The key to lookup the value by. */
  void *value;                /* The value associated with the key. */
  Dict *dict;                 /* The parent dictionary containing the entry. */
} DictEntry;

/*
 * Iterator through a dictionary's keys.
 */
 struct DictIter {
   Dict *dict;                /* The dictionary to iterate. */
   int bucket;                /* The current bucket we are iterating. */
   ListIter *iter;            /* The current iterator. */
 };


/**********************************************************************
 *                       Memory management.
 *********************************************************************/

/*
 * Create a new dictionary entry.
 *
 * @param key: The lookup key.
 * @param value: the associated value.
 * @return A dictionary entry holding the key/value pair.
 */
 static DictEntry *dictEntryCreate(Dict *dict, char *key, void *value) {
   DictEntry *entry = mmalloc(sizeof(DictEntry));
   entry->key = mmalloc(sizeof(char*) * strlen(key));
   strcpy(entry->key, key);
   entry->value = value;
   entry->dict = dict;
   return entry;
 }

/*
 * Free a dictionary entry.
 *
 * @param entry: The entry to free.
 */
static void dictEntryFree(void *entry) {
  assert(entry != NULL);
  DictEntry *e = (DictEntry*) entry;
  mfree(e->key);
  e->dict->free(e->value);
  mfree(entry);
}

/*
 * Create a new dictionary.
 *
 * @return The newly created dict.
 */
Dict *dictCreate(void (*freeFn)(void *value)) {
  Dict *dict = mmalloc(sizeof(Dict));

  dict->size = 0;
  dict->numBuckets = DICT_NUM_BUCKETS_INITIAL;
  dict->buckets = mcalloc(sizeof(List*) * dict->numBuckets);
  dict->free = freeFn != NULL ? freeFn : &free;

  return dict;
}

/*
 * Free an existing dict.
 *
 * @param dict: The dictionary to free.
 */
void dictFree(Dict *dict) {
  assert(dict != NULL);

  for (int i = 0; i < dict->numBuckets; i++)
    if (dict->buckets[i] != NULL)
      listFree(dict->buckets[i]);

  mfree(dict->buckets);
  mfree(dict);
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
unsigned int dictSize(const Dict *dict) {
  return dict->size;
}

/*
 * Compute the hash function for a given string.
 *
 * @param key: The key to hash.
 * @return The hash of the key, which can be used to place it in a bucket.
 */
static unsigned long hash(char *key) {
  unsigned long hash = 5381;
  char c;

  while ((c = *key++) != '\0')
    hash = ((hash << 5) + hash) + c;

  return hash;
}

/*
 * Get the bucket corresponding to the given key.
 *
 * @param dict: The dict to search.
 * @param key: The key to lookup.
 * @return The index of the corresponding bucket.
 */
static unsigned int getBucket(const Dict *dict, char *key) {
  return hash(key) % dict->numBuckets;
}

static int getDictEntry(List *list, char *key, DictEntry **entry) {
  int length = listLength(list);

  if (length == 0)
    return -1;

  if (length == 1) {
    /* Common case. */
    *entry = listGet(list, 0);
    if (strcmp(key, (*entry)->key)) {
      /* Wrong key. */
      *entry = NULL;
      return -1;
    }
    return 0;
  }

  /* Multiple entries in the list. */
  int index = 0;
  ListIter *iter = listIter(list);

  while ((*entry = listIterNext(iter)) != NULL) {
    if (!strcmp(key, (*entry)->key)) {
      /* Found a match! */
      listIterFree(iter);
      return index;
    }
    index++;
  }
  /* Did not find the key. */
  *entry = NULL;
  listIterFree(iter);
  return -1;
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
  assert(dict != NULL);
  assert(key != NULL);

  DictEntry *entry = dictEntryCreate(dict, key, value);
  int bucket = getBucket(dict, key);

  if (dict->buckets[bucket] == NULL)
    dict->buckets[bucket] = listCreate(LIST_TYPE_LINKED, &dictEntryFree);
  listAppend(dict->buckets[bucket], entry);
  dict->size++;

  return dict;
}

/*
 * Remove the object at the given key.
 *
 * @param dict: The dictionary to modify.
 * @param key: The key of the object to remove.
 *
 * @return The dictionary without the key.
 */
Dict *dictRemove(Dict *dict, char *key) {
  assert(dict != NULL);
  assert(key != NULL);

  List *list = dict->buckets[getBucket(dict, key)];
  if (list == NULL)
    return NULL;

  int index;
  DictEntry *entry;
  if ((index = getDictEntry(list, key, &entry)) < 0)
    return NULL;

  listRemove(list, index);
  dict->size--;

  return dict;
}

/*
 * Get the value for an existing key.
 *
 * @param dict: The dictionary to lookup.
 * @param key: The key corresponding to the value.
 * @return The value at the key.
 */
void *dictGet(const Dict *dict, char *key) {
  assert(dict != NULL);
  assert(key != NULL);

  List *list = dict->buckets[getBucket(dict, key)];
  if (list == NULL)
    return NULL;

  DictEntry *entry;
  if (getDictEntry(list, key, &entry) < 0)
    return NULL;

  return entry->value;
}


/**********************************************************************
 *                       Dictionary iteration.
 *********************************************************************/

/*
 * Scan the dictionary for the next bucket with a valid list to iterate.
 * This function should be called when the current list is finished iterating.
 *
 * @param iter: The iterator to scan.
 */
static void dictIterScan(DictIter *iter) {
  assert(iter != NULL);

  while (iter->bucket < iter->dict->numBuckets && iter->dict->buckets[iter->bucket] == NULL) {
    iter->bucket++;
  }

  if (iter->bucket == iter->dict->numBuckets)
    iter->bucket = -1;
  else
    iter->iter = listIter(iter->dict->buckets[iter->bucket]);
}

/*
 * Create a new dictionary iterator.
 *
 * @param dict: The hashmap to iterate.
 * @return An iterator object.
 */
DictIter *dictIter(Dict *dict) {
  assert(dict != NULL);

  DictIter *iter = mcalloc(sizeof(DictIter));
  iter->dict = dict;
  dictIterScan(iter);

  return iter;
}

/*
 * Get the next key from the iterator.
 *
 * @param iter: The iterator to forward.
 * @return The next key, or NULL if the iteration has ended.
 */
char *dictIterNext(DictIter *iter) {
  assert(iter != NULL);

  DictEntry *entry;

  if (iter->bucket == -1)
    return NULL;

  if ((entry = listIterNext(iter->iter)) == NULL) {
    listIterFree(iter->iter);
    iter->iter = NULL;
    iter->bucket++;
    dictIterScan(iter);
    return dictIterNext(iter);
  }

  return entry->key;
}

/*
 * Free an existing iterator.
 *
 * @param iter: The iterator to free.
 */
void dictIterFree(DictIter *iter) {
  assert(iter != NULL);

  if (iter->iter != NULL)
    listIterFree(iter->iter);
  mfree(iter);
}
