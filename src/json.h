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
Json *JsonCreate(void);
Json *JsonCreateNull(void);
Json *JsonCreateBool(bool value);
Json *JsonCreateTrue(void);
Json *JsonCreateFalse(void);
Json *JsonCreateInt(int value);
Json *JsonCreateDouble(double value);
Json *JsonCreateString(char *value);
Json *JsonCreateArray(List *list);
Json *JsonCreateObject(Dict *dict);
void JsonFree(void *json);

/* Conversions. */
Json *JsonParse(const char *content, const char **err);
char *JsonStringify(const Json *json);

#endif
