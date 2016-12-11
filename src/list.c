#include "list.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>


#define ARRAY_LIST_INITIAL_CAPACITY  10
#define ARRAY_LIST_RESIZE_LOAD       0.75


/**********************************************************************
 *                         Struct definitions.
 **********************************************************************/

/*
 * A container around a value in a linked list, with pointers for navigation.
 */
typedef struct ListEntry {
  struct ListEntry *next;  /* The next entry in the list. */
  struct ListEntry *prev;  /* The previous entry. */
  void *value;             /* The value being stored. */
} ListEntry;

/*
 * A iterator, which can traverse the list in either direction.
 */
struct ListIter {
  List *list;                  /* The list we are iterating. */
  ListEntry *next;             /* For a linked list, the next entry in the iterator. */
  unsigned int index;          /* For an array list, the index of the next element. */
  bool reverse;                /* Are we iterating in reverse? */
};

/*
 * A generic doubly linked list.
 *
 */
typedef struct LinkedList {
  ListEntry *head;  /* The start of the list. */
  ListEntry *tail;  /* The end of the list. */
} LinkedList;

/*
 * A generic array list.
 */
typedef struct ArrayList {
  unsigned int capacity;  /* The maximum capacity, after which we need to resize. */
  void *entries;          /* The array of entries. */
} ArrayList;

/*
 * A list object, which can be either array or link based.
 *
 * If the value being inserted is not a primitive type, the caller should supply
 * methods to copy, free, and check equality for the value.
 */
struct List {
  bool linked;          /* True if linked list, false if array list. */
  unsigned int length;  /* The number of elements currently stored. */
  union {
    LinkedList *llist;  /* Pointer to the underlying array list. */
    ArrayList *alist;   /* Pointer to the underlying linked list. */
  };
  void *(*copy)(void *value);
  void (*free)(void *value);
  int (*equals)(void *value1, void *value2);
};


/********************************************************************************
 *                         List iteration.
 *******************************************************************************/

/*
 * Get an iterator to traverse the list.
 *
 * The iterator can run in either direction, and should be freed by the caller.
 * The next value can be retrieved by calling `listIterNext`, which will return NULL on completion.
 *
 * @param list: The list to iterate.
 * @param direction: Either `LIST_ITER_STD` or `LIST_ITER_REV`.
 * @return An iterator for the list.
 */
ListIter *listIter(List *list, bool reverse) {
  assert(list != NULL);

  ListIter *iter = malloc(sizeof(iter));
  iter->list = list;
  iter->reverse = reverse;

  if (list->linked)
    iter->next = reverse ? list->llist->tail : list->llist->head; 
  else
    iter->index = reverse ? list->length - 1 : 0;

  return iter;
}

/*
 * Next method for array lists.
 */
static void *listIterNextArray(ListIter *iter) {
  if (iter->index < 0 || iter->index >= iter->list->length)
    return NULL;
  if (iter->reverse)
    return iter->list->alist->entries + iter->index--;
  else
    return iter->list->alist->entries + iter->index++;
}

/*
 * Get the value of a list entry.
 */
static inline void *listEntryValue(ListEntry *entry) {
  return entry == NULL ? NULL : entry->value;
}

/*
 * Get the neighboring list entry.
 */
static inline void *listEntryNext(ListEntry *entry, bool reverse) {
  return reverse ? entry->prev : entry->next;  
}

/*
 * Get the next list entry for a linked list iterator.
 */
static ListEntry *listIterNextEntry(ListIter *iter) {
  ListEntry *entry = iter->next;

  if (entry != NULL)
    iter->next = listEntryNext(entry, iter->reverse);
  
  return entry;
}

/*
 * Next method for linked lists.
 */
static inline void *listIterNextLinked(ListIter *iter) {
  return listEntryValue(listIterNextEntry(iter)); 
}

/*
 * Get the next value of an iterator.
 *
 * @param iter: The iterator to traverse.
 * @return The next value in the list.
 */
void *listIterNext(ListIter *iter) {
  assert(iter != NULL);

  if (iter->list->linked)
    return listIterNextLinked(iter);
  else
    return listIterNextArray(iter);
}


/********************************************************************************
 *                              Memory management.
 *******************************************************************************/

/*
 * Allocate a linked list.
 */
static LinkedList *listCreateLinked(void) {
  LinkedList *list;

  if ((list = calloc(1, sizeof(LinkedList))) == NULL)
    return NULL;

  return list;
}

/*
 * Allocate an array list.
 */
static ArrayList *listCreateArray(void) {
  ArrayList *list;
  void *entries;

  if ((list = malloc(sizeof(ArrayList))) == NULL)
    return NULL;

  list->capacity = ARRAY_LIST_INITIAL_CAPACITY;
  if ((list->entries = malloc(sizeof(void*) * list->capacity)) == NULL) {
    free(list);
    return NULL;
  }

  return list;
}

/*
 * Create a new list, returning a NULL pointer on error.
 *
 * @param type: The type of list to create (either `LIST_TYPE_ARRAY` or `LIST_TYPE_LINKED`).
 * @return The newly created list.
 */
List *listCreate(bool linked) {
  List *list;
  bool success;

  if ((list = calloc(1, sizeof(List))) == NULL)
    return NULL;
  
  list->linked = linked;

  if (list->linked)
    success = (list->llist = listCreateLinked()) != NULL;
  else
    success = (list->alist = listCreateArray()) != NULL;
  
  if (!success) {
    free(list);
    return NULL;
  }

  return list;
}

/*
 * Free an array list.
 */
void listFreeArray(List *list) {
  if (list->free)
    for (int i = 0; i < list->length; i++)
      list->free(list->alist->entries + i);
  free(list->alist);  
}

/*
 * Free a linked list
 */
void listFreeLinked(List *list) {
  ListEntry *entry;
  ListIter *iter = listIter(list, false);
  
  while ((entry = listIterNextEntry(iter)) != NULL) {
    if (list->free)
      list->free(entry->value);
    free(entry);
  }
  free(list->llist);
}

/*
 * Free an existing list
 *
 * @param list: The list to free.
 */
void listFree(List *list) {
  assert(list != NULL);

  if (list->linked)
    listFreeLinked(list);
  else
    listFreeArray(list);
  free(list);
}

/*
 * Set the function to copy a list entry value.
 *
 * @param copy: The function that returns a copy of a value.
 */
inline void listSetCopyFn(List *list, void *(*copy)(void *value)) {
  list->copy = copy;
}

/*
 * Set the function to free a list entry value.
 *
 * @param free: The function that frees the value.
 */
inline void listSetFree(List *list, void (*free)(void *value)) {
  list->free = free;
}

/*
 * Set the equals function for the list entry value.
 *
 * @param equals: A function that compares two values.
 */
inline void listSetEquals(List *list, int (*equals)(void *value1, void *value2)) {
  list->equals = equals;
}


/********************************************************************************
 *                              List getters.
 *******************************************************************************/

/*
 * Get the length of a list.
 *
 * @param list: The list to examine.
 * @return The length of the list.
 */
unsigned int listLength(const List *list) {
  assert(list != NULL);
  return list->length;
}

/*
 * Get a value from an array list at an index.
 */
inline static void *listGetArray(const List *list, unsigned int index) {
  return list->alist->entries + index;
}

/*
 * Get an entry at a given index.
 */
static ListEntry *listEntryGet(const List *list, unsigned int index) {
  ListIter *iter;
  ListEntry *entry;

  if (index < list->length / 2) {
    iter = listIter((List*) list, false);
  } else {
    iter = listIter((List*) list, true);
    index = list->length - index - 1;
  }

  while (index >= 0)
    entry = listIterNextEntry(iter);

  return entry;
}

/*
 * Get a value from a linked list at an index.
 */
inline static void *listGetLinked(const List *list, unsigned int index) {
  return listEntryValue(listEntryGet(list, index));
}

/*
 * Get the entry at a given index.
 *
 * @param list: The list to fetch the entry from.
 * @param index: The index of the value to fetch.
 * @return The value at the index.
 */
void *listGet(const List *list, unsigned int index) {
  assert(list != NULL);
  assert(index < list->length);

  if (list->linked)
    return listGetLinked(list, index);
  else
    return listGetArray(list, index);
}































