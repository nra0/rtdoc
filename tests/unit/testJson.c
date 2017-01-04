#include "../lib.h"
#include "testJson.h"
#include "../../src/json.h"
#include "../../src/mmalloc.h"


Json *json;
char *err;


static void setup(void) {
  err = mmalloc(256);
}

static void teardown(void) {
  mfree(err);
  assertEqual(0, memoryUsage());
}


static void testJsonParseLiteral(void) {
  json = JsonParse("null", &err);
  assertEqual(JSON_NULL, json->type);
  JsonFree(json);

  json = JsonParse("true", &err);
  assertEqual(JSON_BOOL, json->type);
  assertEqual(true, json->boolValue);
  JsonFree(json);

  json = JsonParse("false", &err);
  assertEqual(JSON_BOOL, json->type);
  assertEqual(false, json->boolValue);
  JsonFree(json);
}

static void testJsonParseInt(void) {
  char *inputs[] = {"0", "42", "-1", "-123", "980", "0023", "1E9", "-4e0"};
  int values[] = {0, 42, -1, -123, 980, 23, 1000000000, -4};
  for (int i = 0; i < arraySize(inputs); i++) {
    json = JsonParse(inputs[i], &err);
    assertEqual(JSON_INT, json->type);
    assertEqual(values[i], json->intValue);
    JsonFree(json);
  }
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
