#include "doc.h"
#include "json.h"
#include "list.h"
#include "mmalloc.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


/**********************************************************************
 *                         Struct definitions.
 **********************************************************************/

struct Document {
  char *key;            /* The unique identifier for the document. */
  Json *contents;       /* The contents of the document. */
  List *collaborators;  /* All users working currently modifying the document. */
};

struct Collaborator {
  char *userId;         /* Identifier for the user. */
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
  doc->collaborators = listCreate(LIST_TYPE_ARRAY, &collaboratorFree);

  return doc;
}

/*
 * Free an existing document.
 *
 * @param doc: The document to free.
 */
void documentFree(void *doc) {
  assert(doc != NULL);

  Document *document = (Document*) doc;
  mfree(document->key);
  jsonFree(document->contents);
  listFree(document->collaborators);
  mfree(document);
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
  user->userId = mmalloc(strlen(userId) + 1);
  strcpy(user->userId, userId);

  return user;
}

/*
 * Free an existing collaborator object.
 *
 * @param user: The user to free.
 */
void collaboratorFree(void *user) {
  assert(user != NULL);

  Collaborator *us = (Collaborator*) user;
  mfree(us->userId);
  mfree(us);
}


/********************************************************************************
 *                         Get information on documents.
 *******************************************************************************/

/*
 * Get the Json contents of a document.
 *
 * @param doc: The document to get the contents for.
 * @return The contents of the document.
 */
Json *documentGetContents(Document *doc) {
  assert(doc != NULL);
  return doc->contents;
}

/*
 * Get the collaborators currently working on a document.
 *
 * @param doc: the document to get the collaborators for.
 * @return The users working on the document.
 */
List *documentGetCollaborators(Document *doc) {
  assert(doc != NULL);
  return doc->collaborators;
}

/*
 * Get the username for a collaborator.
 *
 * @param user: The user to get the key for.
 * @return The unique identifier for the user.
 */
char *collaboratorGetKey(Collaborator *user) {
  assert(user != NULL);
  return user->userId;
}


/********************************************************************************
 *                           Update documents.
 *******************************************************************************/

/*
 * Add a new collaborator to a document.
 *
 * @param doc: The document being modified.
 * @param user: The user beginning the editing session.
 */
void documentAddCollaborator(Document *doc, Collaborator *user) {
  assert(doc != NULL);
  assert(user != NULL);
  listAppend(doc->collaborators, user);
}

/*
 * Remove a collaborator from the document, if it a matching one exists.
 *
 * @param doc: The document being modified.
 * @param userId: The identifier for the user to remove.
 */
void documentRemoveCollaborator(Document *doc, char *userId) {
  assert(doc != NULL);
  assert(userId != NULL);

  ListIter *iter = listIter(doc->collaborators);
  Collaborator *user;
  int index = 0;

  while ((user = listIterNext(iter)) != NULL) {
    if (!strcmp(user->userId, userId)) {
      /* This is the user to remove. */
      listRemove(doc->collaborators, index);
      break;
    }
    index++;
  }

  mfree(iter);
}
