#include "document.h"
#include "json.h"
#include "mmalloc.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


/**********************************************************************
 *                         Struct definitions.
 **********************************************************************/

struct Document {
  char *key;        /* The unique identifier for the document. */
  Json *contents;   /* The contents of the document. */
};

struct Collaborator {
  char *userId;     /* Identifier for the user. */
};


/**********************************************************************
 *                         Memory management.
 **********************************************************************/

/*
 * Create a new document.
 *
 * @param key: The unique identifier for the document.
 * @param contents: The Json contents of the document.
 * @return The created document.
 */
Document *documentCreate(char *key, Json *contents) {
  assert(key != NULL);
  assert(contents != NULL);

  Document *doc = mmalloc(sizeof(Document));
  doc->key = mmalloc(strlen(key) + 1);
  strcpy(doc->key, key);
  doc->contents = contents;

  return doc;
}

/*
 * Free an existing document.
 *
 * @param doc: The document to free.
 */
void documentFree(Document *doc) {
  assert(doc != NULL);

  mfree(doc->key);
  jsonFree(doc->contents);
  mfree(doc);
}

/*
 * Create a new collaborator.
 *
 * @param userId: The unique identifier for the user.
 * @return The created collaborator.
 */
Collaborator *collaboratorCreate(char *userId) {
  assert(userId != NULL);

  Collaborator *user = malloc(sizeof(Collaborator));
  user->userId = userId;

  return user;
}

/*
 * Free an existing collaborator object.
 *
 * @param user: The user to free.
 */
void collaboratorFree(Collaborator *user) {
  assert(user != NULL);
  mfree(user);
}
