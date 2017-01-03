#ifndef __DICT_H__
#define __DICT_H__

/*
 * A hashmap with string keys and arbitrary values.
 */

typedef struct Dict Dict;

/* Memory management. */
Dict *dictCreate(void (*freeFn)(void *value));
void dictFree(Dict *dict);

/* Map methods. */
unsigned int dictSize(const Dict *dict);
Dict *dictSet(Dict *dict, char *key, void *value);
Dict *dictRemove(Dict *dict, char *key);
void *dictGet(const Dict *dict, char *key);

#endif
