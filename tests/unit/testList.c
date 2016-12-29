#include "../lib.h"
#include "testList.h"
#include "../../src/list.h"
#include "../../src/mmalloc.h"


#define DEFAULT_LIST_SIZE 64

List *alist;
List *llist;

static void setup(void) {
  alist = listCreate(LIST_TYPE_ARRAY, &boxCopy, &boxFree, &boxEquals);
  llist = listCreate(LIST_TYPE_LINKED, &boxCopy, &boxFree, &boxEquals);
}

static void teardown(void) {
  listFree(alist);
  listFree(llist);
  assertEqual(0, memoryUsage());
}

static void testListAppendHelper(List *list) {
  for (int i = 0; i < DEFAULT_LIST_SIZE; i++)
    listAppend(list, boxCreate(i));
  assertEqual(DEFAULT_LIST_SIZE, listLength(list));
  for (int i = 0; i < DEFAULT_LIST_SIZE; i++)
    assertEqual(i, boxValue(listGet(list, i)));
}

static void testListAppend(void) {
  testListAppendHelper(alist);
  testListAppendHelper(llist);
}

static void testListPrependHelper(List *list) {
  for (int i = 0; i < DEFAULT_LIST_SIZE; i++)
    listPrepend(list, boxCreate(i));
  assertEqual(DEFAULT_LIST_SIZE, listLength(list));
  for (int i = 0; i < DEFAULT_LIST_SIZE; i++)
    assertEqual(DEFAULT_LIST_SIZE - i - 1, boxValue(listGet(list, i)));
}

static void testListPrepend(void) {
  testListPrependHelper(alist);
  testListPrependHelper(llist);
}

static void testListInsertHelper(List *list) {
  int vals[] = {5, 4, 3, 2, 1};
  int insertOrder[] = {-1, 0, 1, -1, 2};
  for (int i = 0; i < arraySize(vals); i++)
    listInsert(list, insertOrder[i], boxCreate(vals[i]));
  int order[] = {4, 3, 1, 5, 2};
  for (int i = 0; i < arraySize(vals); i++)
    assertEqual(order[i], boxValue(listGet(list, i)));
}

static void testListInsert(void) {
  testListInsertHelper(alist);
  testListInsertHelper(llist);
}

static void testListIndexHelper(List *list) {
  listAppend(list, boxCreate(0));
  listAppend(list, boxCreate(1));
  Box *key0 = boxCreate(0), *key1 = boxCreate(1);
  assertEqual(0, listIndex(list, key0));
  assertEqual(1, listIndex(list, key1));
  boxFree(key0); boxFree(key1);
}

static void testListIndex(void) {
  testListIndexHelper(alist);
  testListIndexHelper(llist);
}

static void testListRemoveHelper(List *list) {
  for (int i = 0; i < DEFAULT_LIST_SIZE; i++)
    listAppend(list, boxCreate(i));
  for (int i = 0; i < DEFAULT_LIST_SIZE; i++) {
    listRemove(list, listLength(list) / 2);
    assertEqual(DEFAULT_LIST_SIZE - i - 1, listLength(list));
  }
}

static void testListRemove(void) {
  testListRemoveHelper(alist);
  testListRemoveHelper(llist);
}

static void testListIterHelper(List *list, bool reverse) {
  for (int i = 0; i < DEFAULT_LIST_SIZE; i++)
    listAppend(list, boxCreate(i));
  ListIter *iter = listIter(list, reverse);
  for (int i = 0; i < DEFAULT_LIST_SIZE; i++) {
    int value = reverse ? DEFAULT_LIST_SIZE - i - 1 : i;
    assertEqual(value, boxValue(listIterNext(iter)));
  }
  assertNull(listIterNext(iter));
  listIterFree(iter);
}

static void testListIterForward(void) {
  testListIterHelper(alist, LIST_ITER_FORWARD);
  testListIterHelper(llist, LIST_ITER_FORWARD);
}

static void testListIterReverse(void) {
  testListIterHelper(alist, LIST_ITER_REVERSE);
  testListIterHelper(llist, LIST_ITER_REVERSE);
}


TestSuite *listTestSuite() {
  TestSuite *suite = testSuiteCreate("array and linked lists", &setup, &teardown);
  testSuiteAdd(suite, "list append", &testListAppend);
  testSuiteAdd(suite, "list prepend", &testListPrepend);
  testSuiteAdd(suite, "list insert", &testListInsert);
  testSuiteAdd(suite, "list index lookup", &testListIndex);
  testSuiteAdd(suite, "list remove", &testListRemove);
  testSuiteAdd(suite, "list iter forward", &testListIterForward);
  testSuiteAdd(suite, "list iter reverse", &testListIterReverse);
  return suite;
}
