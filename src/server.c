#include "dict.h"
#include "doc.h"
#include "mmalloc.h"
#include "server.h"

#include <assert.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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
  pid_t pid;            /* The id of the main server process. */
  unsigned int port;    /* The port the server listens to. */
  int fd;               /* The file descriptor of the server. */
  SockAddr *addr;       /* The address of the server. */
  LogLevel verbosity;   /* If set, the will output each request made to it. */
  FILE *logFile;        /* The file descriptor to log activity to. */
  Dict *documents;      /* The hashmap of keys to documents. */
} Server;

typedef struct Command {
  char *name;           /* The name of the command. */
  unsigned int argc;    /* The number of arguments the command takes. */
  void *fn;             /* The function that implements the command. */
} Command;

Server *server;         /* Global server pointer. */

/********************************************************************************
 *                               Server creation.
 *******************************************************************************/

/*
 * Log the message to the server's log file,
 * if its level is above the server's verbosity level.
 *
 * @param message: The message to log.
 * @param level: The debug level of the message.
 */
static void serverLog(LogLevel level, const char *message, ...) {
 va_list args;
 va_start(args, message);
 if (server->verbosity >= level)
   vfprintf(server->logFile, message, args);
 va_end(args);
}

/*
 * Initialize the server.
 *
 * @param port: The port to run the server on.
 * @param verbosity: The amount of output to the log file.
 */
static void serverCreate(unsigned int port, LogLevel verbosity, FILE *logFile) {
  server = mmalloc(sizeof(Server));

  /* Passed in properties. */
  server->pid = getpid();
  server->port = port;
  server->verbosity = verbosity;
  server->logFile = logFile;

  /* Initialize socket. */
  if ((server->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "Could not open socket connection.\n");
    abort();
  }

  /* Network configuration. */
  server->addr = mcalloc(sizeof(SockAddr));
  server->addr->sin_family = AF_INET;
  server->addr->sin_addr.s_addr = htonl(INADDR_ANY);
  server->addr->sin_port = htons(server->port);

  /* Key value store setup. */
  server->documents = dictCreate(&documentFree);

  /* Start accepting connections. */
  if ((bind(server->fd, (struct sockaddr*) server->addr, sizeof(SockAddr))) < 0) {
    fprintf(stderr, "Could not bind to socket.\n");
    abort();
  }
  listen(server->fd, BACKLOG_SIZE);

  serverLog(LOG_LEVEL_INFO, "Starting RTDoc server on port %d\n", server->port);
}

/*
 * Free an existing server instance.
 *
 * @param server: The server to free.
 */
static void serverFree(void) {
  assert(server != NULL);
  close(server->fd);
  mfree(server->addr);
  dictFree(server->documents);
  mfree(server);
}


/********************************************************************************
 *                            Store actions.
 *******************************************************************************/

#define OK  "ok"
#define NIL "nil"


/*
 * Simple ping to the server.
 *
 * @return 'pong' on success.
 */
static char *serverPing(void) {
  return "pong";
}

/*
 * Add a new document to the document store.
 *
 * @param key: The document's identifier.
 * @param contents: The initial contents of the document.
 * @return The status code of the action.
 */
static char *serverAddDocument(char *key, char *contents) {
  assert(key != NULL);
  assert(contents != NULL);

  char *err = mcalloc(128);
  Json *json = jsonParse(contents, &err);

  if (strlen(err)) {
    /* The parsing failed. */
    mfree(err);
    return NIL;
  }

  Document *doc = documentCreate(key, json);
  dictSet(server->documents, key, doc);

  return OK;
}

/*
 * Retrieve a document from the store.
 *
 * @param key: The identifier of the document to get.
 * @return The matching document if it exists.
 */
static Document *serverGetDocument(char *key) {
  Document *doc;

  if ((doc = dictGet(server->documents, key)) == NULL)
    return NULL;

  return doc;
}

/*
 * Get the JSON contents of a document.
 *
 * @param key: The document to retrieve.
 * @return The contents of the document.
 */
static char *serverGetDocumentContents(char *key) {
  assert(key != NULL);

  Document *doc;

  if ((doc = serverGetDocument(key)) == NULL)
    return NIL;

  return jsonStringify(documentGetContents(doc));
}

static char *serverRemoveDocument(char *key) {
  assert(key != NULL);
  dictRemove(server->documents, key);
  return OK;
}

/*
 * Add a new collaborator to a document, and begin an editing session.
 *
 * @param key: The document to add a user to.
 * @param userId: The id of the user modifying the document.
 * @return The status code of the operation.
 */
static char *serverAddCollaborator(char *key, char *userId) {
  assert(key != NULL);
  assert(userId != NULL);

  Document *doc;

  if ((doc = serverGetDocument(key)) == NULL)
    return NIL;

  documentAddCollaborator(doc, collaboratorCreate(userId));
  return OK;
}

/*
 * Remove a collaborator from a document and end the editing session.
 *
 * @param key: The document to remove the user from.
 * @param userId: The id of the user to remove.
 * @return The status code of the operation.
 */
static char *serverRemoveCollaborator(char *key, char *userId) {
  assert(key != NULL);
  assert(userId != NULL);

  Document *doc;

  if ((doc = serverGetDocument(key)) == NULL)
    return NIL;

  documentRemoveCollaborator(doc, userId);
  return OK;
}

static char *serverModifyDocument(char *key, char *userId, char *change) {
  return OK;
}


/********************************************************************************
 *                           Run the server instance.
 *******************************************************************************/

/* All commands supported by the server. */
Command commandTable[] = {
  {"ping", 0, &serverPing},
  {"add", 2, &serverAddDocument},
  {"get", 1, &serverGetDocumentContents},
  {"remove", 1, &serverRemoveDocument},
  {"start", 2, &serverAddCollaborator},
  {"end", 2, &serverRemoveCollaborator},
  {"update", 2, &serverModifyDocument}
};

/*
 * Interrupt handler that cleans up the open connections before exiting the program.
 */
 static void interruptHandler(int signal) {
   assert(server != NULL);
   serverFree();
   exit(0);
 }

/*
 * Initialize and start the server.
 *
 * @param port: The port to run the server on.
 * @param verbosity: The level of output to the log.
 * @param logFile: The file descriptor to output activity to.
 */
void serverStart(unsigned int port, LogLevel verbosity, FILE *logFile) {
  assert(server == NULL);
  serverCreate(port, verbosity, logFile);

  int clientfd, bytesTransferred;
  char buffer[BUFFER_SIZE];

  /* Catch interrupts for cleanup. */
  signal(SIGINT, interruptHandler);

  /* Accept requests in a loop. */
  while (true) {
    if ((clientfd = accept(server->fd, NULL, NULL)) < 0) {
      fprintf(stderr, "Error accepting client.\n");
      continue;
    }
    while ((bytesTransferred = read(clientfd, buffer, sizeof(buffer))) > 0) {
      printf("Reading %d: %s", bytesTransferred, buffer);
      if ((bytesTransferred = write(clientfd, buffer, strlen(buffer))) < 0) {
        fprintf(stderr, "Client disconnected.\n");
        break;
      }
      memset(buffer, 0, sizeof(buffer));
    }
  }

  serverFree();
}
