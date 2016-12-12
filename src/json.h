#ifndef __JSON_H__
#define __JSON_H__

/*
 * Converting to and from JSON strings and C objects.
 */

typedef struct Json Json;

/* Memory management. */
Json *JsonCreate(void);
void JsonFree(Json *json);

/* Conversions. */
Json *JsonParse(const char *json);
char *JsonStringify(const Json *json);

#endif

