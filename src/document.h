#ifndef __DOCUMENT_H__
#define __DOCUMENT_H__

#include "json.h"


/*
 * Documents stored in the database.
 */

typedef struct Document Document;
typedef struct Collaborator Collaborator;

/* Memory management. */
Document *documentCreate(char *key, Json *contents);
void documentFree(void *doc);
Collaborator *collaboratorCreate(char *userId);
void collaboratorFree(void *user);

/* Get information on documents. */
Json *documentGetContents(Document *doc);
List *documentGetCollaborators(Document *doc);
char *collaboratorGetKey(Collaborator *user);

/* Modify documents. */
void documentAddCollaborator(Document *doc, Collaborator *user);
void documentRemoveCollaborator(Document *doc, char *user);

#endif
