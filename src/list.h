#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>

/*
 * A generic list, which can be implemented as either an array or linked data structure.
 */

#define LIST_ITER_FORWARD false
#define LIST_ITER_REVERSE true
#define LIST_TYPE_ARRAY   false
#define LIST_TYPE_LINKED  true


typedef struct List List;
typedef struct ListIter ListIter;

/* Memory managment. */
List *listCreate(bool linked, void (*freeFn)(void *value));
void listFree(List *list);

/* List getters. */
unsigned int listLength(const List *list);
void *listGet(const List *list, unsigned int index);

/* List setters. */
List *listInsert(List *list, int index, void *value);
List *listAppend(List *list, void *value);
List *listPrepend(List *list, void *value);
List *listRemove(List *list, unsigned int index);

/* List iteration. */
ListIter *listIter(List *list, bool reverse);
void *listIterNext(ListIter *iter);
void listIterFree(ListIter *iter);

#endif
