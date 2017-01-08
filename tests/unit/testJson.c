#include "../lib.h"
#include "testJson.h"
#include "../../src/json.h"
#include "../../src/mmalloc.h"

#include <stdio.h>


#define TEST_OBJECT_SIZE 10


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
  int values[][TEST_OBJECT_SIZE] = {
    {},
    {42},
    {1, 2, 3},
    {-3, 23, 48, 2, -4, 1, 9, 8, 4, 4}
  };
  for (int i = 0; i < arraySize(inputs); i++) {
    json = jsonParse(inputs[i], &err);
    assertEqual(JSON_ARRAY, json->type);
    for (int j = 0; j < TEST_OBJECT_SIZE; j++) {
      if (values[i][j] != 0)
        assertEqual(values[i][j], ((Json*) listGet(json->arrayValue, j))->intValue);
      else
        break;
    }
    jsonFree(json);
  }
}

static void testJsonParseObject(void) {
  char *inputs[] = {
    "{}",
    "{\"key\": \"value\"}",
    "{\"key1\": \"value1\", \"key2\": \"value2\", \"key3\": \"value1\"}"
  };
  char *values[][TEST_OBJECT_SIZE][2] = {
    {{}},
    {{"key", "value"}},
    {{"key1", "value1"}, {"key2", "value2"}, {"key3", "value1"}}
  };
  for (int i = 0; i < arraySize(inputs); i++) {
    json = jsonParse(inputs[i], &err);
    assertEqual(JSON_OBJECT, json->type);
    for (int j = 0; j < TEST_OBJECT_SIZE; j++) {
      char *key = values[i][j][0], *value = values[i][j][1];
      if (values[i][j][0] != NULL)
        assertStringEqual(value, ((Json*) dictGet(json->objectValue, key))->stringValue);
      else
        break;
    }
    jsonFree(json);
  }
}

static void testJsonParseComplex(void) {
  json = jsonParse("{\"foo\": [1, true, \"false\"], \"bar\": {\"baz\": 33.4}}", &err);
  assertEqual(JSON_OBJECT, json->type);
  assertEqual(2, dictSize(json->objectValue));

  Json *foo = dictGet(json->objectValue, "foo");
  Json *bar = dictGet(json->objectValue, "bar");

  assertEqual(JSON_ARRAY, foo->type);
  assertEqual(3, listLength(foo->arrayValue));
  assertEqual(1, ((Json*) listGet(foo->arrayValue, 0))->intValue);
  assertTrue(((Json*) listGet(foo->arrayValue, 1))->boolValue);
  assertStringEqual("false", ((Json*) listGet(foo->arrayValue, 2))->stringValue);

  assertEqual(JSON_OBJECT, bar->type);
  assertEqual(1, dictSize(bar->objectValue));
  assertDoubleEqual(33.4, ((Json*) dictGet(bar->objectValue, "baz"))->doubleValue);

  jsonFree(json);
}

static void testStringify(Json **objects, char **values, unsigned int numValues) {
  char *string;
  for (int i = 0; i < numValues; i++) {
    string = jsonStringify(objects[i]);
    assertStringEqual(values[i], string);
    mfree(string);
    jsonFree(objects[i]);
  }
}

static void testJsonStringifyLiterals(void) {
  Json *objects[] = {
    jsonCreateNull(),
    jsonCreateTrue(),
    jsonCreateFalse()
  };
  char *values[] = {
    "null",
    "true",
    "false"
  };
  testStringify(objects, values, arraySize(objects));
}

static void testJsonStringifyNumbers(void) {
  Json *objects[] = {
    jsonCreateInt(0),
    jsonCreateInt(42),
    jsonCreateInt(-3),
    jsonCreateDouble(3.4),
    jsonCreateDouble(-123.452)
  };
  char *values[] = {
    "0",
    "42",
    "-3",
    "3.4",
    "-123.452"
  };
  testStringify(objects, values, arraySize(objects));
}

static void testJsonStringifyStrings(void) {
  Json *objects[] = {
    jsonCreateString(""),
    jsonCreateString("hello"),
    jsonCreateString("blah \n next line \t\t\n\r\t \\\"hello!\"\\")
  };
  char *values[] = {
    "\"\"",
    "\"hello\"",
    "\"blah \n next line \t\t\n\r\t \\\"hello!\"\\\""
  };
  testStringify(objects, values, arraySize(objects));
}

static void testJsonStringifyArrays(void) {
  List *list1 = listCreate(LIST_TYPE_ARRAY, &jsonFree);
  List *list2 = listCreate(LIST_TYPE_ARRAY, &jsonFree);
  for (int i = 0; i < 8; i++)
    listAppend(list2, jsonCreateInt(i));
  Json *objects[] = {
    jsonCreateArray(list1),
    jsonCreateArray(list2)
  };
  char *values[] = {
    "[]",
    "[0,1,2,3,4,5,6,7]"
  };
  testStringify(objects, values, arraySize(objects));
}

static void testJsonStringifyObjects(void) {
  Dict *dict1 = dictCreate(&jsonFree);
  Dict *dict2 = dictCreate(&jsonFree);
  char key[8];
  for (int i = 0; i < 3; i++) {
    sprintf(key, "key%d", i);
    dictSet(dict2, key, jsonCreateInt(i));
  }
  Json *objects[] = {
    jsonCreateObject(dict1),
    jsonCreateObject(dict2)
  };
  char *values[] = {
    "{}",
    "{\"key2\":2,\"key0\":0,\"key1\":1}"
  };
  testStringify(objects, values, arraySize(objects));
}

static void testJsonConvertComplex(void) {
  Json *json;
  char *string;
  char *values[] = {
    "{\"foo\":[0,1,2,3.3,3,\"bar\"],\"baz\":{\"fee\":[{\"abc\":\"cde\"}]}}",
    "[\"a\",\"b\",false,false,true,null,[null,[false,true]]]",
    "[1,2.2,false,{\"foo\":3.3},{\"bar\":[null,\"hi lo\"]}]"
  };
  for (int i = 0; i < arraySize(values); i++) {
    json = jsonParse(values[i], &err);
    string = jsonStringify(json);
    assertStringEqual(values[i], string);
    mfree(string);
    jsonFree(json);
  }
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
  testSuiteAdd(suite, "stringify literals", &testJsonStringifyLiterals);
  testSuiteAdd(suite, "stringify numbers", &testJsonStringifyNumbers);
  testSuiteAdd(suite, "stringify strings", &testJsonStringifyStrings);
  testSuiteAdd(suite, "stringify arrays", &testJsonStringifyArrays);
  testSuiteAdd(suite, "stringify objects", &testJsonStringifyObjects);
  testSuiteAdd(suite, "convert complex objects", &testJsonConvertComplex);
  return suite;
}
