#ifndef __JSON_H__
#define __JSON_H__

#include "list.h"
#include "dict.h"

/*
 * Converting to and from JSON strings and C objects.
 */

typedef enum JsonType {
  JsonNull,
  JsonBool,
  JsonInt,
  JsonDouble,
  JsonString,
  JsonArray,
  JsonObject
} JsonType;

typedef struct Json {
  JsonType type;
  union {
    bool boolValue;
    int intValue;
    double doubleValue;
    char *stringValue;
    List *arrayValue;
    Dict *objectValue;
  };
} Json;

/* Memory management. */
Json *JsonCreateNull(void);
Json *JsonCreateBool(bool value);
Json *JsonCreateTrue(void);
Json *JsonCreateFalse(void);
Json *JsonCreateInt(int value);
Json *JsonCreateDouble(double value);
Json *JsonCreateString(char *value);
Json *JsonCreateArray(List *list);
Json *JsonCreateObject(Dict *dict);

void JsonFree(Json *json);

/* Conversions. */
Json *JsonParse(const char *json);
char *JsonStringify(const Json *json);

#endif
