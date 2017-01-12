#ifndef __DICT_H__
#define __DICT_H__

/*
 * A hashmap with string keys and arbitrary values.
 */

typedef struct Dict Dict;
typedef struct DictIter DictIter;

/* Memory management. */
Dict *dictCreate(void (*freeFn)(void *value));
void dictFree(Dict *dict);

/* Map methods. */
unsigned int dictSize(const Dict *dict);
Dict *dictSet(Dict *dict, const char *key, void *value);
Dict *dictRemove(Dict *dict, const char *key);
void *dictGet(const Dict *dict, const char *key);

/* Map iteration. */
DictIter *dictIter(Dict *dict);
char *dictIterNext(DictIter *iter);
void dictIterFree(DictIter *iter);

#endif
