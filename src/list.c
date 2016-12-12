#include "list.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


#define ARRAY_LIST_INITIAL_CAPACITY  8


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
 * An iterator that can traverse the list in either direction.
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

  ListIter *iter;

  if ((iter = malloc(sizeof(ListIter))) == NULL)
    return NULL;

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
static void listFreeArray(List *list) {
  for (int i = 0; i < list->length; i++)
    list->free(list->alist->entries + i);
  free(list->alist);  
}

/*
 * Free a linked list
 */
static void listFreeLinked(List *list) {
  ListEntry *entry;
  ListIter *iter = listIter(list, false);
  
  while ((entry = listIterNextEntry(iter)) != NULL) {
    list->free(entry->value);
    free(entry);
  }
  free(iter);

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
static ListEntry *listGetEntry(const List *list, unsigned int index) {
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
  free(iter);

  return entry;
}

/*
 * Get a value from a linked list at an index.
 */
inline static void *listGetLinked(const List *list, unsigned int index) {
  return listEntryValue(listGetEntry(list, index));
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

/*
 * Find a value in an array list.
 */
static int listIndexArray(const List *list, void *value) {
  int index = -1;

  for (int i = 0; i < list->length; i++) {
    if (list->equals(list->alist->entries + i, value)) {
      index = i;
      break;
    }
  }

  return index;
}

/*
 * Find a value in a linked list.
 */
static int listIndexLinked(const List *list, void *value) {
  ListIter *iter;
  int index = -1;

  iter = listIter((List*) list, false);
  for (int i = 0; i < list->length; i++) {
    if (list->equals(listIterNext(iter), value)) {
      index = i;
      break;
    }
  }
  free(iter);

  return index;
}

/*
 * Get the index of the first occurrence of a value in a list.
 *
 * Returns -1 if the value is not found.
 *
 * @param list: The list to search.
 * @param value:  The value to search for.
 * @return The index of the value.
 */
int listIndex(const List *list, void *value) {
  assert(list != NULL);

  if (list->linked)
    return listIndexLinked(list, value);
  else
    return listIndexArray(list, value);
}


/********************************************************************************
 *                              List mutations.
 *******************************************************************************/

/*
 * Insert a value into an array list.
 */
static List *listInsertArray(List *list, int index, void *value) {
  if (list->length == list->alist->capacity) {
    /* Reallocate array. */
    char *entries;

    list->alist->capacity = list->alist->capacity * 2 + 1;
    if ((entries = malloc(sizeof(void*) * list->alist->capacity)))
      return NULL;
    
    memcpy(list->alist->entries, entries, sizeof(void*) * list->length);
    free(list->alist->entries);
    list->alist->entries = entries;
    return listInsertArray(list, index, value);
  } 

  if (index < 0) {
    /* Append to end of list. */
    memcpy(value, list->alist->entries + list->length, sizeof(void*));
  } else {
    /* Insert into list. */
    for (int i = list->length; i >= index; i++)
      memcpy(list->alist->entries + i - 1, list->alist->entries + i, sizeof(void*));
    memcpy(value, list->alist->entries + index, sizeof(void*));
  }

  return list;
}

/*
 * Insert a value into a linked list.
 */
static List *listInsertLinked(List *list, int index, void *value) {
  ListEntry *entry, *current;

  if ((entry = malloc(sizeof(ListEntry))) == NULL)
    return NULL;
  entry->value = value;

  if (!list->length) {
    /* This is the first entry in the list. */
    list->llist->head = entry;
    list->llist->tail = entry;
  } else if (index < 0) {
    /* Append to the end of the list. */
    entry->prev = list->llist->tail;
    entry->next = NULL;
    list->llist->tail = entry;
  } else {
    /* Insert into an existing position. */
    current = listGetEntry(list, index);
    entry->next = current;
    entry->prev = current->prev;
    current->prev = entry;
    if (entry->prev != NULL)
      entry->prev->next = entry;
    if (index == 0)
      list->llist->head = entry;
  }

  return list;
}

/*
 * Insert a value into a list at an index.
 *
 * An index of -1 will append the value to the end of the list.
 *
 * @param list: The list to insert into.
 * @param index: The position to put the value.
 * @return The list with the value added.
 */
List *listInsert(List *list, int index, void *value) {
  assert(list != NULL);
  assert(index < list->length);
  
  List *ret;

  if (list->linked)
    ret = listInsertLinked(list, index, value);
  else
    ret = listInsertArray(list, index, value);
  if (ret != NULL)
    ret->length++;

  return ret;
}

/*
 * Append a value to the end of a list.
 *
 * This is the equivalent of `listInsert` with an index of -1.
 *
 * @param list: The list to append to.
 * @param value: The value to append.
 * @return The list with the value appended.
 */
inline List *listAppend(List *list, void *value) {
  return listInsert(list, -1, value);
}

/*
 * Insert a value at the beginning of a list.
 *
 * This is the equivaleng of `listInsert` with an index of 0.
 *
 * @param list: The list to prepend to.
 * @param value: The value to prepend.
 * @return The list with the new value.
 */
inline List *listPrepend(List *list, void *value) {
  return listInsert(list, 0, value); 
}

/*
 * Remove a value from an array list.
 */
static List *listRemoveArray(List *list, unsigned int index) {
  list->free(list->alist->entries + index);
  
  for (int i = index; i < list->length - 1; i++)
    memcpy(list->alist->entries + i + 1, list->alist->entries + i, sizeof(void*));

  return list;
}

/*
 * Remove a value from a linked list.
 */
static List *listRemoveLinked(List *list, unsigned int index) {
  ListEntry *entry = listGetEntry(list, index);
  
  list->free(entry->value);
  if (entry->prev != NULL)
    entry->prev->next = entry->next;
  if (entry->next != NULL)
    entry->next->prev = entry->prev;
  if (entry == list->llist->head)
    list->llist->head = entry->next;
  if (entry == list->llist->tail)
    list->llist->tail = entry->prev;

  return list;
}

/*
 * Remove a value from a list at an index.
 *
 * @param list: The list to remove from.
 * @param index: The position in the list to remove from.
 * @return The list without the value.
 */
List *listRemove(List *list, unsigned int index) {
  assert(list != NULL);
  assert(index < list->length);

  if (list->linked)
    return listRemoveLinked(list, index);
  else
    return listRemoveArray(list, index);
}

