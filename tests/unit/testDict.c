#include "../lib.h"
#include "testDict.h"
#include "../../src/dict.h"
#include "../../src/mmalloc.h"

#include <stdio.h>


Dict *dict;
DictIter *iter;


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
  char key[8];
  for (int i = 0; i < numValues; i++) {
    sprintf(key, "key%d", i);
    dict = dictSet(dict, key, boxCreate(i));
  }
  assertEqual(numValues, dictSize(dict));
  assertEqual(7182, boxValue(dictGet(dict, "key7182")));
}

static void testDictRemove(void) {
  int numValues = 32;
  char key[8];
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
}

static void testDictMutationKey(void) {
  char key[] = "key";
  dictSet(dict, key, boxCreate(1));
  key[1] = 'a';
  assertStringEqual("kay", key);
  assertNotNull(dictGet(dict, "key"));
}

static void testDictIter(void) {
  int numValues = 1;
  char key[8];
  for (int i = 0; i < numValues; i++) {
    sprintf(key, "key%d", i);
    dictSet(dict, key, boxCreate(i));
  }
  iter = dictIter(dict);
  for (int i = 0; i < numValues; i++)
    assertNotNull(dictIterNext(iter));
  assertNull(dictIterNext(iter));
  dictIterFree(iter);
}


TestSuite *dictTestSuite() {
  TestSuite *suite = testSuiteCreate("hash map", &setup, &teardown);
  testSuiteAdd(suite, "set and get single value", &testDictSingleValue);
  testSuiteAdd(suite, "set many values", &testDictManyValues);
  testSuiteAdd(suite, "remove key", &testDictRemove);
  testSuiteAdd(suite, "key mutation", &testDictMutationKey);
  testSuiteAdd(suite, "key iteration", &testDictIter);
  return suite;
}
