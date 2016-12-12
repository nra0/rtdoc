#ifndef __DICT_H__
#define __DICT_H__

/*
 * A hashmap with string keys and arbitrary values.
 */

typedef struct Dict Dict;

/* Memory management. */
Dict *dictCreate(void);
void dictFree(Dict *dict);
void dictSetCopyFn(Dict *dict, void *(*copy)(void *value));
void dictSetFreeFn(Dict *dict, void (*free)(void *value));
void dictSetEqualsFn(Dict *dict, int (*equals)(void *value1, void *value2));

/* Map methods. */
unsigned int dictSize(const Dict *dict);
Dict *dictSet(Dict *dict, char *key, void *value);
void *dictGet(const Dict *dict, char *key);

#endif

