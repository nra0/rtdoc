#include "../lib.h"
#include "testList.h"
#include "../../src/list.h"
#include "../../src/mmalloc.h"


#define DEFAULT_LIST_SIZE 64

List *alist;
List *llist;

static void setup(void) {
  alist = listCreate(LIST_TYPE_ARRAY, &boxFree);
  llist = listCreate(LIST_TYPE_LINKED, &boxFree);
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
  ListIter *iter = reverse ? listIterReverse(list) : listIter(list);
  for (int i = 0; i < DEFAULT_LIST_SIZE; i++) {
    int value = reverse ? DEFAULT_LIST_SIZE - i - 1 : i;
    assertEqual(value, boxValue(listIterNext(iter)));
  }
  assertNull(listIterNext(iter));
  listIterFree(iter);
}

static void testListIterForward(void) {
  testListIterHelper(alist, true);
  testListIterHelper(llist, true);
}

static void testListIterReverse(void) {
  testListIterHelper(alist, false);
  testListIterHelper(llist, false);
}


TestSuite *listTestSuite() {
  TestSuite *suite = testSuiteCreate("array and linked lists", &setup, &teardown);
  testSuiteAdd(suite, "list append", &testListAppend);
  testSuiteAdd(suite, "list prepend", &testListPrepend);
  testSuiteAdd(suite, "list insert", &testListInsert);
  testSuiteAdd(suite, "list remove", &testListRemove);
  testSuiteAdd(suite, "list iter forward", &testListIterForward);
  testSuiteAdd(suite, "list iter reverse", &testListIterReverse);
  return suite;
}
