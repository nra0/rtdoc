#include "document.h"

#include <stdlib.h>


/**********************************************************************
 *                         Struct definitions.
 **********************************************************************/

struct Document {
  char *key;      /* The unique identifier for the document. */
};

struct Collaborator {
  char *userId;   /* Identifier for the user. */
};


/**********************************************************************
 *                         Memory management.
 **********************************************************************/

Document *documentCreate(char *key, Json *contents) {
  return NULL;
}

void documentFree(Document *doc) {

}

Collaborator *collaboratorCreate(char *userId) {
  return NULL;
}

void collaboratorFree(Collaborator *user) {

}
