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
void documentFree(Document *doc);

Collaborator *collaboratorCreate(char *userId);
void collaboratorFree(Collaborator *user);

#endif
