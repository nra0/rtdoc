#include "dict.h"
#include "json.h"
#include "list.h"
#include "mmalloc.h"

#include <assert.h>


/**********************************************************************
 *                        Memory management.
 **********************************************************************/

/*
 * @return A representation of the NULL Json object.
 */
Json *JsonCreateNull(void) {
  Json *json = mmalloc(sizeof(Json));
  json->type = JsonNull;
  return json;
}

/*
 * @param value: The truthiness of the boolean.
 * @return A Json boolean value.
 */
Json *JsonCreateBool(bool value) {
  Json *json = mmalloc(sizeof(Json));
  json->type = JsonBool;
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
  Json *json = mmalloc(sizeof(Json));
  json->type = JsonInt;
  json->intValue = value;
  return json;
}

/*
 * @param value: The double value of the object.
 * @return A Json double.
 */
Json *JsonCreateDouble(double value) {
  Json *json = mmalloc(sizeof(Json));
  json->type = JsonDouble;
  json->doubleValue = value;
  return json;
}

/*
 * @param value: The string value of the object.
 * @return A Json string.
 */
Json *JsonCreateString(char *value) {
  Json *json = mmalloc(sizeof(Json));
  json->type = JsonString;
  json->stringValue = value;
  return json;
}

/*
 * @param list: The list representing the Json array.
 * @return A Json array.
 */
Json *JsonCreateArray(List *list) {
  Json *json = mmalloc(sizeof(Json));
  json->type = JsonArray;
  json->arrayValue = list;
  return json;
}

/*
 * @param dict: The dictionary representing the Json object.
 * @return A Json object.
 */
Json *JsonCreateObject(Dict *dict) {
  Json *json = mmalloc(sizeof(Json));
  json->type = JsonObject;
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
    case JsonString: mfree(json->stringValue); break;
    case JsonArray: mfree(json->arrayValue); break;
    case JsonObject: mfree(json->objectValue); break;
    default: break;
  }

  mfree(json);
}

/*
 * Parse a string into an object.
 *
 * @param json: The string to parse.
 * @return The parsed Json object.
 */
Json *JsonParse(const char *json) {
  return NULL;
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
