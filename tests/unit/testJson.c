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

static void testJsonParseInt(void) {
  char *inputs[] = {
    "0",
    "42",
    "-1",
    "-123",
    "980",
    "0023",
    "1E9",
    "-4e0"
  };
  int values[] = {
    0,
    42,
    -1,
    -123,
    980,
    23,
    1000000000,
    -4
  };
  for (int i = 0; i < arraySize(inputs); i++) {
    json = jsonParse(inputs[i], &err);
    assertEqual(JSON_INT, json->type);
    assertEqual(values[i], json->intValue);
    jsonFree(json);
  }
}

static void testJsonParseDouble(void) {
  char *inputs[] = {
    "0.1",
    "4.232",
    "-23.342"
  };
  double values[] = {
    0.1,
    4.232,
    -23.342
  };
  for (int i = 0; i < arraySize(inputs); i++) {
    json = jsonParse(inputs[i], &err);
    assertEqual(JSON_DOUBLE, json->type);
    assertEqual(values[i], json->doubleValue);
    jsonFree(json);
  }
}

static void testJsonParseString(void) {
  char *inputs[] = {
    "\"hello\"",
    "\"My name is Doo\"",
    "\"We\nhave now\n\n a multiline\n \n string!... \t \n\t With some tabs\"",
    "\"Let's try some \\\"escape sequences!\\\""
  };
  char *values[] = {
    "hello",
    "My name is Doo",
    "We\nhave now\n\n a multiline\n \n string!... \t \n\t With some tabs",
    "Let's try some \"escape sequences!\""
  };
  for (int i = 0; i < arraySize(inputs); i++) {
    json = jsonParse(inputs[i], &err);
    assertEqual(JSON_STRING, json->type);
    assertStringEqual(values[i], json->stringValue);
    jsonFree(json);
  }
}

static void testJsonParseArray(void) {
  char *inputs[] = {
    "[]",
    "[42]",
    "[1, 2, 3]",
    "[-3, 23, 48, 2, -4, 1, 9, 8, 4, 4]"
  };
  int values[][10] = {
    {},
    {42},
    {1, 2, 3},
    {-3, 23, 48, 2, -4, 1, 9, 8, 4, 4}
  };
  for (int i = 0; i < arraySize(inputs); i++) {
    json = jsonParse(inputs[i], &err);
    assertEqual(JSON_ARRAY, json->type);
    for (int j = 0; j < 10; j++) {
      if (values[i][j] != 0)
        assertEqual(values[i][j], ((Json*) listGet(json->arrayValue, j))->intValue);
      else
        break;
    }
    jsonFree(json);
  }
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
