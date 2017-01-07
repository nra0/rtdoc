#include "dict.h"
#include "json.h"
#include "list.h"
#include "mmalloc.h"

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>


/**********************************************************************
 *                        Memory management.
 **********************************************************************/

/*
 * @return An allocated Json object.
 */
Json *jsonCreate(void) {
  Json *json = mmalloc(sizeof(Json));
  return json;
}

/*
 * @return A representation of the NULL Json object.
 */
Json *jsonCreateNull(void) {
  Json *json = jsonCreate();
  json->type = JSON_NULL;
  return json;
}

/*
 * @param value: The truthiness of the boolean.
 * @return A Json boolean value.
 */
Json *jsonCreateBool(bool value) {
  Json *json = jsonCreate();
  json->type = JSON_BOOL;
  json->boolValue = value;
  return json;
}

/*
 * @return A Json boolean object with a true value.
 */
Json *jsonCreateTrue(void) {
  return jsonCreateBool(true);
}

/*
 * @return A Json boolean object with a false value.
 */
Json *jsonCreateFalse(void) {
  return jsonCreateBool(false);
}

/*
 * @param value: The integer value of the object.
 * @return A Json integer.
 */
Json *jsonCreateInt(int value) {
  Json *json = jsonCreate();
  json->type = JSON_INT;
  json->intValue = value;
  return json;
}

/*
 * @param value: The double value of the object.
 * @return A Json double.
 */
Json *jsonCreateDouble(double value) {
  Json *json = jsonCreate();
  json->type = JSON_DOUBLE;
  json->doubleValue = value;
  return json;
}

/*
 * @param value: The string value of the object.
 * @return A Json string.
 */
Json *jsonCreateString(char *value) {
  Json *json = jsonCreate();
  json->stringValue = mmalloc(strlen(value) + 1);
  json->type = JSON_STRING;
  strcpy(json->stringValue, value);
  return json;
}

/*
 * @param list: The list representing the Json array.
 * @return A Json array.
 */
Json *jsonCreateArray(List *list) {
  Json *json = jsonCreate();
  json->type = JSON_ARRAY;
  json->arrayValue = list;
  return json;
}

/*
 * @param dict: The dictionary representing the Json object.
 * @return A Json object.
 */
Json *jsonCreateObject(Dict *dict) {
  Json *json = jsonCreate();
  json->type = JSON_OBJECT;
  json->objectValue = dict;
  return json;
}

/*
 * Free the memory of an existing Json object.
 *
 * @param json: The object to free.
 */
void jsonFree(void *json) {
  assert(json != NULL);

  Json *js = (Json*) json;
  switch (js->type) {
    case JSON_STRING:   mfree(js->stringValue); break;
    case JSON_ARRAY:    listFree(js->arrayValue); break;
    case JSON_OBJECT:   dictFree(js->objectValue); break;
    default: break;
  }

  mfree(js);
}


/**********************************************************************
 *               Parse Json string to binary struct.
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
#define POSITIVE      '+'
#define NEGATIVE      '-'
#define DECIMAL       '.'
#define ZERO          '0'
#define BASE          10
#define EXPONENT      'e'
#define WS_LIMIT      ' '

/* Utility macros. */
#define inc(p)        (p + 1)
#define ctoi(n)       (n - '0')
#define min(a, b)      (a < b ? a : b)

/*
 * Parsing failure.
 */
static const char *fail(const char *content, char **err) {
  strcpy(*err, content);
  return false;
}

/*
 * Skip all the beginning whitespace of a string.
 */
static const char *skip(const char *content) {
  for (; content && *content && *content <= WS_LIMIT; content++);
  return content;
}


static const char *parseNext(Json *json, const char *content, char **err);

static const char *parseNumber(Json *json, const char *content, char **err) {
  double n = 0;
  int sign = 1, expSign = 1, exp = 0, scale = 0;

  /* Check for negative numbers. */
  if (*content == NEGATIVE)
    sign = -1, content++;

  /* Make sure we have a number. */
  if (!isdigit(*content))
    return fail(content, err);

  /* Skip leading zeros. */
  while (*content == ZERO)
    content++;

  /* Numbers up to the decimal. */
  while (isdigit(*content))
    n = (n * BASE) + ctoi(*content++);

  /* After the decimal. */
  if (*content == DECIMAL)
    while (isdigit(*++content))
      n = (n * BASE) + ctoi(*content), scale--;

  /* Handle exponenents. */
  if (tolower(*content) == EXPONENT) {
    content++;
    if (*content == POSITIVE)
      content++;
    else if (*content == NEGATIVE)
      expSign = -1, content++;
    while (isdigit(*content))
      exp = (exp * BASE) + ctoi(*content++);
  }

  n = sign * n * pow(BASE, scale + expSign * exp);

  int m = (int) n;
  if (n - (double) m == 0.0) {
    /* It is an integer. */
    json->type = JSON_INT;
    json->intValue = m;
  } else {
    /* It is a double. */
    json->type = JSON_DOUBLE;
    json->doubleValue = n;
  }

  return content;
}

static const char *parseString(Json *json, const char *content, char **err) {
  if (*content != STRING_SEP)
    return fail(content, err);

  int len = 0;
  char *value;
  const char *end;

  /* Find the length of the string. */
  for (end = ++content; *end != STRING_SEP; len++, end++)
    if (*end == ESCAPE)
      end++;

  if (*end != STRING_SEP)
    return fail(end, err);

  value = mcalloc(len + 1);
  json->type = JSON_STRING;
  json->stringValue = value;

  /* Copy the string. */
  while (content < end) {
    if (*content == ESCAPE) {
      /* Escaped character. */
      content++;
      switch (*content) {
        case 'b': *value = '\b'; break;
        case 'f': *value = '\f'; break;
        case 'n': *value = '\n'; break;
        case 'r': *value = '\r'; break;
        case 't': *value = '\t'; break;
        default: *value = *content;
      }
    } else {
      *value = *content;
    }
    value++, content++;
  }

  return inc(content);
}

static const char *parseArray(Json *json, const char *content, char **err) {
  if (*content != ARRAY_BEGIN)
    return fail(content, err);

  json->type = JSON_ARRAY;
  json->arrayValue = listCreate(LIST_TYPE_ARRAY, &jsonFree);

  content = skip(inc(content));
  if (*content == ARRAY_END)
    return inc(content);

  /* The list is not empty. */
  do {
    Json *element = jsonCreate();
    if ((content = skip(parseNext(element, content, err))) == NULL) {
      /* Parsing error. */
      jsonFree(element);
      return content;
    }
    listAppend(json->arrayValue, element);

    if (*content != VALUE_SEP)
      break;
    content = skip(inc(content));
  } while (*content != ARRAY_END);

  /* End of the list. */
  if (*content != ARRAY_END)
    return fail(content, err);
  return inc(content);
}

static const char *parseObject(Json *json, const char *content, char **err) {
  if (*content != OBJECT_BEGIN)
    return fail(content, err);

  char *key;
  Json *element;

  json->type = JSON_OBJECT;
  json->objectValue = dictCreate(&jsonFree);

  content = skip(inc(content));
  if (*content == OBJECT_END)
    return inc(content);
  if (*content == VALUE_SEP)
    return fail(content, err);

  /* Object is not empty. */
  do {
    if (*content == VALUE_SEP)
      content = skip(inc(content));

    element = jsonCreate();

    /* Get the key. */
    if ((content = skip(parseString(element, content, err))) == NULL) {
      jsonFree(element);
      return content;
    }

    key = element->stringValue;

    /* Check for colon. */
    if (*content != KEY_SEP) {
      mfree(key);
      return fail(content, err);
    }

    /* Get the value. */
    if ((content = skip(parseNext(element, skip(inc(content)), err))) == NULL) {
      jsonFree(element);
      mfree(key);
      return content;
    }

    dictSet(json->objectValue, key, element);
    mfree(key);
  } while (*content == VALUE_SEP);

  /* End of the object. */
  if (*content != OBJECT_END)
    return fail(content, err);
  return content;
}

static const char *parseNext(Json *json, const char *content, char **err) {
  if (!content)
    return false;

  /* Check what type of Json object we are dealing with. */
  switch (*content) {
    case STRING_SEP     : return parseString(json, content, err);
    case ARRAY_BEGIN    : return parseArray(json, content, err);
    case OBJECT_BEGIN   : return parseObject(json, content, err);
  }

  if (*content == NEGATIVE || isdigit(*content)) return parseNumber(json, content, err);

  /* Check for literals. */
  if (!strncmp(content, NULL_LITERAL, NULL_LEN)) {
    json->type = JSON_NULL;
    return content + NULL_LEN;
  }
  if (!strncmp(content, TRUE_LITERAL, TRUE_LEN)) {
    json->type = JSON_BOOL;
    json->boolValue = true;
    return content + TRUE_LEN;
  }
  if (!strncmp(content, FALSE_LITERAL, FALSE_LEN)) {
    json->type = JSON_BOOL;
    json->boolValue = false;
    return content + FALSE_LEN;
  }

  /* Invalid input. */
  return fail(content, err);
}

/*
 * Parse a string into an object.
 *
 * @param json: The string to parse.
 * @return The parsed Json object.
 */
Json *jsonParse(const char *content, char **err) {
  Json *json = jsonCreate();
  const char *end = parseNext(json, skip(content), err);

  if (!end) {
    /* Parsing error. */
    jsonFree(json);
    return false;
  }

  /* Success! */
  return json;
}


/**********************************************************************
 *                    Stringify a Json object
 **********************************************************************/

#define JSON_STRING_INITIAL_SIZE   256


static char *reallocContent(char *content) {
  return mrealloc(content, msize(content) * 2 + 1);
}

static int stringifyNull(const Json *json, char *content, unsigned int offset) {
  assert(json->type == JSON_NULL);

  if (offset + NULL_LEN > msize(content))
    content = reallocContent(content);

  strcpy(content + offset, NULL_LITERAL);
  return NULL_LEN;
}

static int stringifyBool(const Json *json, char *content, unsigned int offset) {
  assert(json->type == JSON_BOOL);

  if (offset + FALSE_LEN > msize(content))
    content = reallocContent(content);

  if (json->boolValue) {
    strcpy(content + offset, TRUE_LITERAL);
    return TRUE_LEN;
  } else {
    strcpy(content + offset, FALSE_LITERAL);
    return FALSE_LEN;
  }
}

static int stringifyInt(const Json *json, char *content, unsigned int offset) {
  assert(json->type == JSON_INT);

  int length = snprintf(NULL, 0, "%d", json->intValue);
  if (offset + length > msize(content))
    content = reallocContent(content);

  sprintf(content + offset, "%d", json->intValue);
  return length;
}

static int stringifyDouble(const Json *json, char *content, unsigned int offset) {
  assert(json->type == JSON_DOUBLE);

  int length = snprintf(NULL, 0, "%8g", json->doubleValue);

  if (offset + length > msize(content))
    content = reallocContent(content);

  sprintf(content + offset, "%.8g", json->doubleValue);
  return length;
}

static int stringifyString(const Json *json, char *content, unsigned int offset) {
  assert(json->type == JSON_STRING);

  int length = strlen(json->stringValue) + 2;

  if (offset + length > msize(content))
    content = reallocContent(content);

  sprintf(content + offset, "\"%s\"", json->stringValue);
  return length;
}

static int stringifyArray(const Json *json, char *content, unsigned int offset) {
  assert(json->type == JSON_ARRAY);

  if (offset + 1 > msize(content))
    content = reallocContent(content);

  *++content = '[';
}

static int stringifyObject(const Json *json, char *content, unsigned int offset) {
  return 0;
}

int stringifyNext(const Json *json, char *content, unsigned int offset) {
  switch (json->type) {
    case JSON_NULL:   return stringifyNull(json, content, offset);
    case JSON_BOOL:   return stringifyBool(json, content, offset);
    case JSON_INT:    return stringifyInt(json, content, offset);
    case JSON_DOUBLE: return stringifyDouble(json, content, offset);
    case JSON_STRING: return stringifyString(json, content, offset);
    case JSON_ARRAY:  return stringifyArray(json, content, offset);
    case JSON_OBJECT: return stringifyObject(json, content, offset);
  }
}

/*
 * Convert an object into a string.
 *
 * @param json: The object to convert.
 * @return The object represented as a string.
 */
char *jsonStringify(const Json *json) {
  assert(json != NULL);

  char *content = mcalloc(JSON_STRING_INITIAL_SIZE);
  stringifyNext(json, content, 0);
  return content;
}
