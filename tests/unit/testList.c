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

/*
 * Check that the list length function works when adding objects.
 */
void testListLengthBasic(void) {
  int length = 10;
  for (int i = 0; i < length; i++) {
    listAppend(llist, boxCreate());
    listAppend(alist, boxCreate());
  }
  assertEqual(length, listLength(llist));
  assertEqual(length, listLength(alist));
}


TestSuite *listTestSuite() {
  TestSuite *suite = testSuiteCreate("array and linked lists", &setup, &teardown);
  testSuiteAdd(suite, "list length basic", &testListLengthBasic);
  return suite;
}

