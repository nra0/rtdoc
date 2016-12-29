#include "../lib.h"
#include "testDict.h"
#include "../../src/dict.h"
#include "../../src/mmalloc.h"


Dict *dict;


static void setup(void) {
  dict = dictCreate(&boxCopy, &boxFree);
}

static void teardown(void) {
  dictFree(dict);
  assertEqual(0, memoryUsage());
}


static void testSingleValue(void) {
  dictSet(dict, "key", boxCreate(42));
  assertEqual(1, dictSize(dict));
  assertEqual(42, boxValue(dictGet(dict, "key")));
}


TestSuite *dictTestSuite() {
  TestSuite *suite = testSuiteCreate("hash map", &setup, &teardown);
  testSuiteAdd(suite, "set and get single value", &testSingleValue);
  return suite;
}
