#include "../lib.h"
#include "testList.h"
#include "../../src/list.h"

#include <stdio.h>


List *alist;
List *llist;

void setup(void) {
  alist = listCreate(false, &boxCopy, &boxFree, &boxEquals);
  llist = listCreate(true, &boxCopy, &boxFree, &boxEquals);
}

void teardown(void) {
  listFree(alist);
  listFree(llist);
}


void testAlistIterBasic(void) {
  int length = 3;
  for (int i = 0; i < length; i++)
    listAppend(alist, boxCreate(i));
  ListIter *iter = listIter(alist, false);
  for (int i = 0; i < length; i++)
    assertPointerNotNull(listIterNext(iter));
  assertPointerNull(listIterNext(iter));
}

void testLlistIterBasic(void) {
  int length = 3;
  for (int i = 0; i < length; i++)
    listAppend(llist, boxCreate(i));
  ListIter *iter = listIter(llist, false);
  for (int i = 0; i < length; i++)
    assertPointerNotNull(listIterNext(iter));
  assertPointerNull(listIterNext(iter));
}

void testListIterReverse(void) {

}

void testListLengthShort(void) {
  int length = 4;
  for (int i = 0; i < length; i++) {
    listAppend(llist, boxCreate(i));
    listAppend(alist, boxCreate(i));
  }
  assertEqual(length, listLength(llist));
  assertEqual(length, listLength(alist));
}


TestSuite *listTestSuite() {
  TestSuite *suite = testSuiteCreate("array and linked lists", &setup, &teardown);
  testSuiteAdd(suite, "array list basic iteration", &testAlistIterBasic);
  testSuiteAdd(suite, "linked list basic iteration", &testLlistIterBasic);
  testSuiteAdd(suite, "reverse list iteration", &testListIterReverse);
  testSuiteAdd(suite, "list length short", &testListLengthShort);
  return suite;
}
