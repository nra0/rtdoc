#include "dict.h"
#include "doc.h"
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

typedef struct Server {
  unsigned int port;  /* The port the server listens to. */
  int fd;             /* The file descriptor of the server. */
  SockAddr *addr;     /* The address of the server. */
  bool verbose;       /* If set, the will output each request made to it. */
  Dict *documents;    /* The hashmap of keys to documents. */
} Server;

Server *server;       /* Global server pointer. */

/********************************************************************************
 *                               Server creation.
 *******************************************************************************/

/*
 * Free an existing server instance.
 *
 * @param server: The server to free.
 */
void serverFree(void) {
  assert(server != NULL);

  mfree(server->addr);
  dictFree(server->documents);
  mfree(server);
}


/********************************************************************************
 *                            Modify the store.
 *******************************************************************************/

static int serverAddDocument(char *key, char *contents) {
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

static Document *serverGetDocument(char *key) {
  Document *doc;

  if ((doc = dictGet(server->documents, key)) == NULL)
    return NULL;

  return doc;
}

static char *serverGetDocumentContents(char *key) {
 assert(server != NULL);
 assert(key != NULL);

 Document *doc;

 if ((doc = serverGetDocument(key)) == NULL)
  return NULL;

 return jsonStringify(documentGetContents(doc));
}

static int serverAddCollaborator(char *key, char *userId) {
  assert(server != NULL);
  assert(key != NULL);
  assert(userId != NULL);

  Document *doc;

  if ((doc = serverGetDocument(key)) == NULL)
    return -1;

  documentAddCollaborator(doc, collaboratorCreate(userId));
  return 0;
}

static int serverRemoveCollaborator(char *key, char *userId) {
  assert(server != NULL);
  assert(key != NULL);
  assert(userId != NULL);

  Document *doc;

  if ((doc = serverGetDocument(key)) == NULL)
    return -1;

  documentRemoveCollaborator(doc, userId);
  return 0;
}

static int serverModifyDocument(char *key, char *userId, char *change) {
  return 0;
}


/********************************************************************************
 *                           Run the server instance.
 *******************************************************************************/

/*
* Initialize and start the server.
*
* @param port: The port to run the server on.
* @parm verbose: Whether to output activity to a log.
*/
void serverStart(unsigned int port, bool verbose) {
  assert(server == NULL);

  server = mmalloc(sizeof(Server));

  /* Initialize socket. */
  server->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->fd < 0) {
    fprintf(stderr, "Could not open socket connection.\n");
    abort();
  }
  server->addr = mcalloc(sizeof(SockAddr));
  server->addr->sin_family = AF_INET;
  server->addr->sin_addr.s_addr = htonl(INADDR_ANY);
  server->addr->sin_port = htons(server->port);

  /* Other configurations. */
  server->port = port;
  server->verbose = verbose;

  /* Key value store setup. */
  server->documents = dictCreate(&documentFree);

  int clientfd;

  if (server->verbose)
    printf("Starting RTDoc server on port %d\n", server->port);

  /* Start accepting connections. */
  if ((bind(server->fd, (struct sockaddr*) server->addr, sizeof(SockAddr))) < 0) {
    fprintf(stderr, "Could not bind to socket.\n");
    abort();
  }

  listen(server->fd, BACKLOG_SIZE);
  while (true) {
    clientfd = accept(server->fd, NULL, NULL);
    write(clientfd, "hello there", strlen("hello there"));
  }
}
