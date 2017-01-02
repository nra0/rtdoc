#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdbool.h>


/*
 * Database server; handles all reads and writes from clients.
 */

typedef struct RTServer RTServer;


/* Configuration methods. */
RTServer *serverCreate(void);
void serverSetPort(RTServer *server, unsigned int port);
void serverSetVerbosity(RTServer *server, bool verbose);
void serverStart(RTServer *server);

/* Database methods. */

/* Set and get keys. */
int serverAddDocument(RTServer *server, char *key, char *contents);
char *serverGetDocument(RTServer *server, char *key);

/* Modify collaborators. */
int serverAddCollaborator(RTServer *server, char *key, char *userId);
int serverRemoveCollaborator(RTServer *server, char *key, char *userId);

/* Modify documents. */
int serverModifyDocument(RTServer, char *key, char *userId, char *change);

#endif
