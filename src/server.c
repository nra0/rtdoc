#include "dict.h"
#include "document.h"
#include "mmalloc.h"
#include "server.h"

#include <assert.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#define BUFFER_SIZE  1024
#define BACKLOG_SIZE 128

typedef struct sockaddr_in SockAddr;


/********************************************************************************
 *                              Struct declarations.
 *******************************************************************************/

struct RTServer {
  unsigned int port;  /* The port the server listens to. */
  int fd;             /* The file descriptor of the server. */
  SockAddr *addr;     /* The address of the server. */
  bool verbose;       /* If set, the will output each request made to it. */
  Dict *documents;    /* The hashmap of keys to documents. */
};

/********************************************************************************
 *                               Server creation.
 *******************************************************************************/

/*
 * Create a new server instance.
 *
 * @return A server instance, ready to be started.
 */
RTServer *serverCreate(void) {
  RTServer *server = mmalloc(sizeof(RTServer));
  SockAddr *serverAddr = mcalloc(sizeof(SockAddr));
  char buffer[BUFFER_SIZE];

  server->fd = socket(AF_INET, SOCK_STREAM, 0);
  server->addr = serverAddr;
  server->verbose = false;

  /* Networking setup. */
  serverAddr->sin_family = AF_INET;
  serverAddr->sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddr->sin_port = htons(server->port);

  /* Key value store setup. */
  server->documents = dictCreate(&documentFree);

  return server;
}

/*
 * Free an existing server instance.
 *
 * @param server: The server to free.
 */
void serverFree(RTServer *server) {
  assert(server != NULL);

  mfree(server->addr);
  dictFree(server->documents);
  mfree(server);
}

/*
 * Set the port that the server should listen to.
 *
 * @param server: The server to modify.
 * @param port: The port the server should listen to.
 */
void serverSetPort(RTServer *server, unsigned int port) {
  assert(server != NULL);
  server->port = port;
}

/*
 * Set the verbosity output of the running server.
 *
 * @param server: The server to modify.
 * @param verbose: Whether the server should print all its outputs.
 */
void serverSetVerbosity(RTServer *server, bool verbose) {
  assert(server != NULL);
  server->verbose = verbose;
}

/*
 * Start an existing server.
 *
 * @param server: The server to start.
 */
void serverStart(RTServer *server) {
  assert(server != NULL);

  int clientfd;

  if (server->verbose)
    printf("Starting RTDoc server on port %d\n", server->port);

  listen(server->fd, BACKLOG_SIZE);
  while (true) {
    clientfd = accept(server->fd, NULL, NULL);
    write(clientfd, "hello there", strlen("hello there"));
  }
}


/********************************************************************************
 *                            Modify the store.
 *******************************************************************************/

/*
* Add a new document to the key/value store.
*
* @param server: The server to modify.
* @param key: The key to set or update.
* @param contents: The Json contents to set for the key.
* @return `0` on success, `-1` on an error.
*/
int serverAddDocument(RTServer *server, char *key, char *contents) {
 assert(server != NULL);
 assert(key != NULL);
 assert(contents != NULL);

 char *err = mcalloc(128);
 Json *json = jsonParse(contents, &err);

 if (strlen(err)) {
   /* The parsing failed. */
   mfree(err);
   return -1;
 }

 Document *doc = documentCreate(key, json);
 dictSet(server->documents, key, doc);

 return 0;
}

static Document *serverGetDoc(RTServer *server, char *key) {
  Document *doc;

  if ((doc = dictGet(server->documents, key)) == NULL)
    return NULL;

  return doc;
}

/*
* Get the Json contents of a given document as a string.
*
* @param server: The server to lookup.
* @param key: The identifier for the document.
* @return The contents of the document.
*/
char *serverGetDocument(RTServer *server, char *key) {
 assert(server != NULL);
 assert(key != NULL);

 Document *doc;

 if ((doc = serverGetDoc(server, key)) == NULL)
  return NULL;

 return jsonStringify(documentGetContents(doc));
}

/*
 * Begin a new editing session by adding a user to a document.
 *
 * @param server: The server containing the document.
 * @param key: The document identifier.
 * @param userId: The identifier of the user to add.
 * @return `0` on success, `-1` on an error.
 */
int serverAddCollaborator(RTServer *server, char *key, char *userId) {
  assert(server != NULL);
  assert(key != NULL);
  assert(userId != NULL);

  Document *doc;

  if ((doc = serverGetDoc(server, key)) == NULL)
    return -1;

  documentAddCollaborator(doc, collaboratorCreate(userId));
  return 0;
}

/*
 * Remove a collaborator from a document and end the editing session.
 *
 * @param server: The server containing the document.
 * @param key: The document identifier.
 * @param userId: The identifier of the user to remove.
 * @return `0` on success, `-1` on an error.
 */
int serverRemoveCollaborator(RTServer *server, char *key, char *userId) {
  assert(server != NULL);
  assert(key != NULL);
  assert(userId != NULL);

  Document *doc;

  if ((doc = serverGetDoc(server, key)) == NULL)
    return -1;

  documentRemoveCollaborator(doc, userId);
  return 0;
}

/*
 * Modify a document with a transform.
 *
 * @param server: The server containing the document.
 * @param key: The document identifier.
 * @param userId: The identifier of the user updating the document.
 * @param change: The transform to apply to the document.
 * @return `0` on success, `-1` on an error.
 */
int serverModifyDocument(RTServer *server, char *key, char *userId, char *change) {
  return 0;
}
