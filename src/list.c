#include "list.h"
#include "mmalloc.h"

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
  List *list;             /* The list we are iterating. */
  union {
    ListEntry *next;      /* For a linked list, the next entry in the iterator. */
    int index;            /* For an array list, the index of the next element. */
  };
  bool reverse;           /* Are we iterating in reverse? */
};

/*
 * A generic doubly linked list.
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
  void **entries;         /* The array of entries. */
} ArrayList;

/*
 * A list object, which can be either array or link based.
 *
 * If the value being inserted is not a primitive type, the caller should supply
 * methods to free the value.
 */
struct List {
  bool linked;          /* True if linked list, false if array list. */
  unsigned int length;  /* The number of elements currently stored. */
  union {
    LinkedList *llist;  /* Pointer to the underlying array list. */
    ArrayList *alist;   /* Pointer to the underlying linked list. */
  };
  void (*free)(void *value);
};


/********************************************************************************
 *                         List iteration.
 *******************************************************************************/

/*
 * Get an iterator to traverse the list.
 *
 * The iterator should be freed by the caller.
 * The next value can be retrieved by calling `listIterNext`, which will return NULL on completion.
 *
 * @param list: The list to iterate.
 * @return An iterator for the list.
 */
ListIter *listIter(List *list) {
  assert(list != NULL);

  ListIter *iter = mmalloc(sizeof(ListIter));
  iter->list = list;
  iter->reverse = false;
  if (list->linked)
    iter->next = list->llist->head;
  else
    iter->index = 0;

  return iter;
}

/*
 * A list iterator that runs in the opposite direction.
 *
 * @param list: The list to iterate.
 * @return A reverse iterator for the list.
 */
ListIter *listIterReverse(List *list) {
  assert(list != NULL);

  ListIter *iter = mmalloc(sizeof(ListIter));
  iter->list = list;
  iter->reverse = true;
  if (list->linked)
    iter->next = list->llist->tail;
  else
    iter->index = list->length - 1;

  return iter;
}

/*
 * Next method for array lists.
 */
static void *listIterNextArray(ListIter *iter) {
  if (iter->index < 0 || iter->index >= iter->list->length)
    return NULL;
  if (iter->reverse)
    return iter->list->alist->entries[iter->index--];
  else
    return iter->list->alist->entries[iter->index++];
}

/*
 * Get the value of a list entry.
 */
static void *listEntryValue(ListEntry *entry) {
  return entry == NULL ? NULL : entry->value;
}

/*
 * Get the neighboring list entry.
 */
static void *listEntryNext(ListEntry *entry, bool reverse) {
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
static void *listIterNextLinked(ListIter *iter) {
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

/*
 * Free a list iter.
 *
 * @param iter: The iterator to free.
 */
void listIterFree(ListIter *iter) {
  assert(iter != NULL);
  mfree(iter);
}


/********************************************************************************
 *                              Memory management.
 *******************************************************************************/

/*
 * Allocate an array list.
 */
static ArrayList *listCreateArray(void) {
  ArrayList *list = mmalloc(sizeof(ArrayList));
  list->capacity = ARRAY_LIST_INITIAL_CAPACITY;
  list->entries = mmalloc(sizeof(void*) * list->capacity);
  return list;
}

/*
 * Allocate a linked list.
 */
static LinkedList *listCreateLinked(void) {
  return mcalloc(sizeof(LinkedList));
}

/*
 * Get a value from an array list at an index.
 */
static void *listGetArray(const List *list, unsigned int index) {
  return list->alist->entries[index];
}

/*
 * Get an entry at a given index.
 */
static ListEntry *listGetEntry(const List *list, unsigned int index) {
  ListIter *iter;
  ListEntry *entry;

  if (index < list->length / 2) {
    iter = listIter((List*) list);
  } else {
    iter = listIterReverse((List*) list);
    index = list->length - index - 1;
  }

  for (int i = 0; i <= index; i++)
    entry = listIterNextEntry(iter);
  listIterFree(iter);

  return entry;
}

/*
 * Get a value from a linked list at an index.
 */
static void *listGetLinked(const List *list, unsigned int index) {
  return listEntryValue(listGetEntry(list, index));
}

/*
 * Create a new list, returning a NULL pointer on error.
 *
 * @param type: The type of list to create (either `LIST_TYPE_ARRAY` or `LIST_TYPE_LINKED`).
 * @param free: Function to free the list value.
 * @return The newly created list.
 */
List *listCreate(bool linked, void (*freeFn)(void *value)) {
  List *list = mcalloc(sizeof(List));
  list->linked = linked;

  if (list->linked)
    list->llist = listCreateLinked();
  else
    list->alist = listCreateArray();

  list->free = freeFn != NULL ? freeFn : &free;

  return list;
}

/*
 * Free an array list.
 */
static void listFreeArray(List *list) {
  void *entry;
  ListIter *iter = listIter(list);

  while ((entry = listIterNext(iter)) != NULL)
    list->free(entry);
  listIterFree(iter);
  mfree(list->alist->entries);
  mfree(list->alist);
}

/*
 * Free a linked list
 */
static void listFreeLinked(List *list) {
  ListEntry *entry;
  ListIter *iter = listIter(list);

  while ((entry = listIterNextEntry(iter)) != NULL) {
    list->free(entry->value);
    mfree(entry);
  }
  listIterFree(iter);
  mfree(list->llist);
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
  mfree(list);
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


/********************************************************************************
 *                              List mutations.
 *******************************************************************************/

/*
 * Insert a value into an array list.
 */
static List *listInsertArray(List *list, int index, void *value) {
  if (list->length == list->alist->capacity) {
    /* Reallocate array. */
    list->alist->capacity = list->alist->capacity * 2 + 1;
    list->alist->entries = mrealloc(list->alist->entries, sizeof(void*) * list->alist->capacity);
    return listInsertArray(list, index, value);
  }

  if (index < 0) {
    /* Append to end of list. */
    list->alist->entries[list->length] = value;
  } else {
    /* Insert into list. */
    for (int i = list->length; i > index; i--)
      list->alist->entries[i] = list->alist->entries[i-1];
    list->alist->entries[index] = value;
  }

  return list;
}

/*
 * Insert a value into a linked list.
 */
static List *listInsertLinked(List *list, int index, void *value) {
  ListEntry *current, *entry = mcalloc(sizeof(ListEntry));
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
    if (index == 0)
      list->llist->head = entry;
  }
  if (entry->prev != NULL)
    entry->prev->next = entry;

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
  assert(index < list->length || index == -1);

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
List *listAppend(List *list, void *value) {
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
List *listPrepend(List *list, void *value) {
  int index = listLength(list) ? 0 : -1;
  return listInsert(list, index, value);
}

/*
 * Remove a value from an array list.
 */
static List *listRemoveArray(List *list, unsigned int index) {
  list->free(list->alist->entries[index]);

  for (int i = index; i < list->length - 1; i++)
    list->alist->entries[i] = list->alist->entries[i+1];

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
  mfree(entry);

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
    list = listRemoveLinked(list, index);
  else
    list = listRemoveArray(list, index);
  list->length--;

  return list;
}
