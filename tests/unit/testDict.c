#include "../lib.h"
#include "testDict.h"
#include "../../src/dict.h"
#include "../../src/mmalloc.h"

#include <stdio.h>


Dict *dict;


static void setup(void) {
  dict = dictCreate(&boxFree);
}

static void teardown(void) {
  dictFree(dict);
  assertEqual(0, memoryUsage());
}


static void testDictSingleValue(void) {
  dictSet(dict, "key", boxCreate(42));
  assertEqual(1, dictSize(dict));
  assertEqual(42, boxValue(dictGet(dict, "key")));
}

static void testDictManyValues(void) {
  int numValues = 8192;
  char *key = mmalloc(128);
  for (int i = 0; i < numValues; i++) {
    sprintf(key, "key%d", i);
    dict = dictSet(dict, key, boxCreate(i));
  }
  mfree(key);
  assertEqual(numValues, dictSize(dict));
  assertEqual(7182, boxValue(dictGet(dict, "key7182")));
}

static void testDictRemove(void) {
  int numValues = 32;
  char *key = mmalloc(128);
  for (int i = 0; i < numValues; i++) {
    sprintf(key, "key%d", i);
    dict = dictSet(dict, key, boxCreate(i));
  }
  for (int i = 0; i < numValues; i++) {
    sprintf(key, "key%d", i);
    dictRemove(dict, key);
    assertEqual(numValues - i - 1, dictSize(dict));
    assertNull(dictGet(dict, key));
  }
  mfree(key);
}

static void testDictMutationKey(void) {
  char key[] = "key";
  dictSet(dict, key, boxCreate(1));
  key[1] = 'a';
  assertStringEqual("kay", key);
  assertNotNull(dictGet(dict, "key"));
}


TestSuite *dictTestSuite() {
  TestSuite *suite = testSuiteCreate("hash map", &setup, &teardown);
  testSuiteAdd(suite, "set and get single value", &testDictSingleValue);
  testSuiteAdd(suite, "set many values", &testDictManyValues);
  testSuiteAdd(suite, "remove key", &testDictRemove);
  testSuiteAdd(suite, "key mutation", &testDictMutationKey);
  return suite;
}
