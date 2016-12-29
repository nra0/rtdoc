#include "../lib.h"
#include "testDict.h"
#include "../../src/dict.h"
#include "../../src/mmalloc.h"


Dict *dict;


static void setup(void) {
  dict = dictCreate(&boxCopy, &boxFree, &boxEquals);
}

static void teardown(void) {
  dictFree(dict);
  assertEqual(0, memoryUsage());
}


static void testSetSingleValue(void) {
  dictSet(dict, "hello", boxCreate(42));
  assertEqual(1, dictSize(dict));
}


TestSuite *dictTestSuite() {
  TestSuite *suite = testSuiteCreate("hash map", &setup, &teardown);
  testSuiteAdd(suite, "set single value", &testSetSingleValue);
  return suite;
}
