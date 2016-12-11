/*
 * A generic list, which can be implemented as either an array or linked data structure.
 */

#ifndef __LIST_H__
#define __LIST_H__

typedef struct List List;
typedef struct ListIter ListIter;

/* Memory managment. */
List *listCreate(bool linked);
void listFree(List *list);
void listSetCopyFn(List *list, void *(*copy)(void *value));
void listSetFreeFn(List *list, void (*free)(void *value));
void listSetEqualsFn(List *list, int (*equals)(void *value1, void *value2));

/* List getters. */
unsigned int listLength(const List *list);
void *listGet(const List *list, unsigned int index);
int indexOf(List *list, void *value);
unsigned int listType(const List *list);

/* List setters. */
List listAdd(List *list, unsigned int index, void *value);
List listAppend(List *list, void *value);
List listPrepend(List *list, void *value);
void *listRemove(List *list, unsigned int index);
List listRemoveValue(List *list, void *value);

/* List iteration. */
ListIter *listIter(List *list, bool reverse);
void *listIterNext(ListIter *iter);

#endif

