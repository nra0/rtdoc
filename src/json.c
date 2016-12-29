#include "dict.h"
#include "json.h"
#include "list.h"
#include "mmalloc.h"

#include <assert.h>


/**********************************************************************
 *                        Memory management.
 **********************************************************************/

Json *JsonCreate(void) {
  return NULL;
}

void JsonFree(Json *json) {

}

Json *JsonParse(const char *json) {
  return NULL;
}

char *JsonStringify(const Json *json) {
  return NULL;
}
