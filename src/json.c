#include "dict.h"
#include "json.h"
#include "list.h"
#include "mmalloc.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>


/**********************************************************************
 *                        Memory management.
 **********************************************************************/

/*
 * @return An allocated Json object.
 */
Json *JsonCreate(void) {
  Json *json = mmalloc(sizeof(Json));
  return json;
}

/*
 * @return A representation of the NULL Json object.
 */
Json *JsonCreateNull(void) {
  Json *json = JsonCreate();
  json->type = JSON_NULL;
  return json;
}

/*
 * @param value: The truthiness of the boolean.
 * @return A Json boolean value.
 */
Json *JsonCreateBool(bool value) {
  Json *json = JsonCreate();
  json->type = JSON_BOOL;
  json->boolValue = value;
  return json;
}

/*
 * @return A Json boolean object with a true value.
 */
Json *JsonCreateTrue(void) {
  return JsonCreateBool(true);
}

/*
 * @return A Json boolean object with a false value.
 */
Json *JsonCreateFalse(void) {
  return JsonCreateBool(false);
}

/*
 * @param value: The integer value of the object.
 * @return A Json integer.
 */
Json *JsonCreateInt(int value) {
  Json *json = JsonCreate();
  json->type = JSON_INT;
  json->intValue = value;
  return json;
}

/*
 * @param value: The double value of the object.
 * @return A Json double.
 */
Json *JsonCreateDouble(double value) {
  Json *json = JsonCreate();
  json->type = JSON_DOUBLE;
  json->doubleValue = value;
  return json;
}

/*
 * @param value: The string value of the object.
 * @return A Json string.
 */
Json *JsonCreateString(char *value) {
  Json *json = JsonCreate();
  json->type = JSON_STRING;
  json->stringValue = value;
  return json;
}

/*
 * @param list: The list representing the Json array.
 * @return A Json array.
 */
Json *JsonCreateArray(List *list) {
  Json *json = JsonCreate();
  json->type = JSON_ARRAY;
  json->arrayValue = list;
  return json;
}

/*
 * @param dict: The dictionary representing the Json object.
 * @return A Json object.
 */
Json *JsonCreateObject(Dict *dict) {
  Json *json = JsonCreate();
  json->type = JSON_OBJECT;
  json->objectValue = dict;
  return json;
}

/*
 * Free the memory of an existing Json object.
 *
 * @param json: The object to free.
 */
void JsonFree(Json *json) {
  assert(json != NULL);

  switch (json->type) {
    case JSON_STRING: mfree(json->stringValue); break;
    case JSON_ARRAY: mfree(json->arrayValue); break;
    case JSON_OBJECT: mfree(json->objectValue); break;
    default: break;
  }

  mfree(json);
}


/**********************************************************************
 *             Parsing to and from string representation.
 **********************************************************************/

/* Json literal values. */
#define ARRAY_BEGIN   '['
#define ARRAY_END     ']'
#define OBJECT_BEGIN  '{'
#define OBJECT_END    '}'
#define KEY_SEP       ':'
#define VALUE_SEP     ','
#define STRING_SEP    '\"'
#define ESCAPE        '\\'
#define NULL_LITERAL  "null"
#define TRUE_LITERAL  "true"
#define FALSE_LITERAL "false"
#define NULL_LEN      4
#define TRUE_LEN      4
#define FALSE_LEN     5
#define NEGATIVE      '-'
#define WS_LIMIT      ' '

/* Utility macros. */
#define inc(p)        (p + 1)

/*
 * Skip all the beginning whitespace of a string.
 */
static const char *skip(const char *content) {
  for (; content && *content && *content <= WS_LIMIT; content++);
  return content;
}


static const char *parseNext(Json *json, const char *content);

static const char *parseNumber(Json *json, const char *content) {
  return false;
}

static const char *parseString(Json *json, const char *content) {
  return false;
}

static const char *parseArray(Json *json, const char *content) {
  return false;
}

static const char *parseObject(Json *json, const char *content) {
  return false;
}

static const char *parseNext(Json *json, const char *content) {
  if (!content) return false;

  /* Check what type of Json object we are dealing with. */
  switch (*content) {
    case STRING_SEP     : return parseString(json, content);
    case ARRAY_BEGIN    : return parseArray(json, content);
    case OBJECT_BEGIN   : return parseObject(json, content);
  }

  if (*content == NEGATIVE || isdigit(*content)) return parseNumber(json, content);

  /* Check for literals. */
  if (!strncmp(content, NULL_LITERAL, NULL_LEN)) {
    json->type = JSON_NULL;
    return content + NULL_LEN;
  }
  if (!strncmp(content, TRUE_LITERAL, TRUE_LEN)) {
    json->type = JSON_BOOL;
    json->boolValue = true;
  }
  if (!strncmp(content, FALSE_LITERAL, FALSE_LEN)) {
    json->type = JSON_BOOL;
    json->boolValue = false;
  }

  /* Invalid input. */
  return false;
}

/*
 * Parse a string into an object.
 *
 * @param json: The string to parse.
 * @return The parsed Json object.
 */
Json *JsonParse(const char *content, char **err) {
  Json *json = JsonCreate();
  const char *end = parseNext(json, skip(content));

  if (!end) {
    /* Parsing error. */
    JsonFree(json);
    return false;
  }
  
  /* Success! */
  return json;
}

/*
 * Convert an object into a string.
 *
 * @param json: The object to convert.
 * @return The object represented as a string.
 */
char *JsonStringify(const Json *json) {
  return NULL;
}
