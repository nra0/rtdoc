#include "../lib.h"
#include "testJson.h"
#include "../../src/json.h"
#include "../../src/mmalloc.h"


Json *json;


static void setup(void) {
  json = JsonCreate();
}

static void teardown(void) {
  JsonFree(json);
}


static void testJsonParseLiteral(void) {

}

static void testJsonParseInt(void) {

}

static void testJsonParseDouble(void) {

}

static void testJsonParseString(void) {

}

static void testJsonParseArray(void) {

}

static void testJsonParseObject(void) {

}

static void testJsonParseComplex(void) {

}


TestSuite *jsonTestSuite() {
  TestSuite *suite = testSuiteCreate("JSON", &setup, &teardown);
  testSuiteAdd(suite, "parse literals", &testJsonParseLiteral);
  testSuiteAdd(suite, "parse integers", &testJsonParseInt);
  testSuiteAdd(suite, "parse doubles", &testJsonParseDouble);
  testSuiteAdd(suite, "parse strings", &testJsonParseString);
  testSuiteAdd(suite, "parse arrays", &testJsonParseArray);
  testSuiteAdd(suite, "parse objects", &testJsonParseObject);
  testSuiteAdd(suite, "parse complex objects", &testJsonParseComplex);
  return suite;
}
