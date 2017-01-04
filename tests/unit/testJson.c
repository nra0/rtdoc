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


static void testjsonParseLiteral(void) {
  json = jsonParse("null", &err);
  assertEqual(JSON_NULL, json->type);
  jsonFree(json);

  json = jsonParse("true", &err);
  assertEqual(JSON_BOOL, json->type);
  assertEqual(true, json->boolValue);
  jsonFree(json);

  json = jsonParse("false", &err);
  assertEqual(JSON_BOOL, json->type);
  assertEqual(false, json->boolValue);
  jsonFree(json);
}

static void testjsonParseInt(void) {
  char *inputs[] = {"0", "42", "-1", "-123", "980", "0023", "1E9", "-4e0"};
  int values[] = {0, 42, -1, -123, 980, 23, 1000000000, -4};
  for (int i = 0; i < arraySize(inputs); i++) {
    json = jsonParse(inputs[i], &err);
    assertEqual(JSON_INT, json->type);
    assertEqual(values[i], json->intValue);
    jsonFree(json);
  }
}

static void testjsonParseDouble(void) {
  char *inputs[] = {"0.1", "4.232", "-23.342"};
  double values[] = {0.1, 4.232, -23.342};
  for (int i = 0; i < arraySize(inputs); i++) {
    json = jsonParse(inputs[i], &err);
    assertEqual(JSON_DOUBLE, json->type);
    assertEqual(values[i], json->doubleValue);
    jsonFree(json);
  }
}

static void testjsonParseString(void) {
  char *inputs[] = {"\"hello\", \"i am foo\", \"many\n lines\n\r are here\t\n\t\n\n oh yes\""};
  char *values[] = {"hello", "i am foo", "many\n lines\n\r are here\t\n\t\n\n oh yes"};
  for (int i = 0; i < arraySize(inputs); i++) {
    json = jsonParse(inputs[i], &err);
    assertEqual(JSON_STRING, json->type);
    assertStringEqual(values[i], json->stringValue);
    jsonFree(json);
  }
}

static void testjsonParseArray(void) {

}

static void testjsonParseObject(void) {

}

static void testjsonParseComplex(void) {

}


TestSuite *jsonTestSuite() {
  TestSuite *suite = testSuiteCreate("JSON", &setup, &teardown);
  testSuiteAdd(suite, "parse literals", &testjsonParseLiteral);
  testSuiteAdd(suite, "parse integers", &testjsonParseInt);
  testSuiteAdd(suite, "parse doubles", &testjsonParseDouble);
  testSuiteAdd(suite, "parse strings", &testjsonParseString);
  testSuiteAdd(suite, "parse arrays", &testjsonParseArray);
  testSuiteAdd(suite, "parse objects", &testjsonParseObject);
  testSuiteAdd(suite, "parse complex objects", &testjsonParseComplex);
  return suite;
}
