#ifndef __JSON_H__
#define __JSON_H__

#include "list.h"
#include "dict.h"


#define JSON_OBJECT_KEY_LIMIT 256

/*
 * Converting to and from JSON strings and C objects.
 */

typedef enum JsonType {
  JSON_NULL,
  JSON_BOOL,
  JSON_INT,
  JSON_DOUBLE,
  JSON_STRING,
  JSON_ARRAY,
  JSON_OBJECT
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
Json *jsonCreate(void);
Json *jsonCreateNull(void);
Json *jsonCreateBool(bool value);
Json *jsonCreateTrue(void);
Json *jsonCreateFalse(void);
Json *jsonCreateInt(int value);
Json *jsonCreateDouble(double value);
Json *jsonCreateString(char *value);
Json *jsonCreateArray(List *list);
Json *jsonCreateObject(Dict *dict);
void jsonFree(void *json);

/* Conversions. */
Json *jsonParse(const char *content, char **err);
char *jsonStringify(const Json *json);

#endif
