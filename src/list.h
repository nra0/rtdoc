#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>

/*
 * A generic list, which can be implemented as either an array or linked data structure.
 */


typedef struct List List;
typedef struct ListIter ListIter;

/* Memory managment. */
List *listCreate(bool linked, void *(*copy)(void *value), void (*free)(void *value), int (*equals)(void *value1, void *value2));
void listFree(List *list);

/* List getters. */
unsigned int listLength(const List *list);
void *listGet(const List *list, unsigned int index);
int listIndex(const List *list, void *value);

/* List setters. */
List *listInsert(List *list, int index, void *value);
List *listAppend(List *list, void *value);
List *listPrepend(List *list, void *value);
List *listRemove(List *list, unsigned int index);

/* List iteration. */
ListIter *listIter(List *list, bool reverse);
void *listIterNext(ListIter *iter);

#endif

