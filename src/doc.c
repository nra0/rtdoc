#include "doc.h"
#include "json.h"
#include "list.h"
#include "mmalloc.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>


#define mutexInit(x,y)   (pthread_mutex_init(x,y))
#define mutexLock(x)     (pthread_mutex_lock(x))
#define mutexUnlock(x)   (pthread_mutex_unlock(x))

typedef pthread_mutex_t   Mutex;


/**********************************************************************
 *                         Struct definitions.
 **********************************************************************/

struct Document {
  char *key;            /* The unique identifier for the document. */
  Json *contents;       /* The contents of the document. */
  List *collaborators;  /* All users working currently modifying the document. */
  Mutex mutex;          /* Lock to access the document. */
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
  mutexInit(&doc->mutex, NULL);

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

  Collaborator *user = mmalloc(sizeof(Collaborator));
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
  mutexLock(&doc->mutex);
  listAppend(doc->collaborators, user);
  mutexUnlock(&doc->mutex);
}

static int getCollaborator(List *list, char *key, Collaborator **user) {
  int length = listLength(list);

  if (length == 0)
    return -1;

  if (length == 1) {
    /* Common case. */
    *user = listGet(list, 0);
    if (strcmp(key, (*user)->userId)) {
      /* Wrong key. */
      *user = NULL;
      return -1;
    }
    return 0;
  }

  /* Multiple entries in the list. */
  int index = 0;
  ListIter *iter = listIter(list);

  while ((*user = listIterNext(iter)) != NULL) {
    if (!strcmp(key, (*user)->userId)) {
      /* Found a match! */
      listIterFree(iter);
      return index;
    }
    index++;
  }
  /* Did not find the key. */
  *user = NULL;
  listIterFree(iter);
  return -1;
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
  mutexLock(&doc->mutex);

  Collaborator *user;
  int index;

  if ((index = getCollaborator(doc->collaborators, userId, &user)) >= 0)
    listRemove(doc->collaborators, index);

  mutexUnlock(&doc->mutex);
}
