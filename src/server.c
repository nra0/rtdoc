#include "dict.h"
#include "doc.h"
#include "list.h"
#include "mmalloc.h"
#include "server.h"

#include <assert.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#define BUFFER_SIZE         4096
#define MAX_CLIENTS         8
#define UNUSED(x)           (void)(x)

typedef struct sockaddr_in  SockAddr;
typedef pthread_mutex_t     Mutex;
typedef pthread_cond_t      CondVar;


/********************************************************************************
 *                              Struct declarations.
 *******************************************************************************/

typedef struct Client {
  int fd;                                     /* The file descriptor of the client. */
} Client;

typedef struct WorkQueue {
  List *clients;                              /* Clients waiting to be run. */
  Mutex mutex;                                /* Lock to access the queue. */
  CondVar cv;                                 /* Condition variable to wait for list to fill. */
} WorkQueue;

typedef struct Server {
  pid_t pid;                                  /* The id of the main server process. */
  unsigned int port;                          /* The port the server listens to. */
  int fd;                                     /* The file descriptor of the server. */
  SockAddr *addr;                             /* The address of the server. */
  LogLevel verbosity;                         /* If set, the will output each request made to it. */
  FILE *logFile;                              /* The file descriptor to log activity to. */
  Dict *documents;                            /* The hashmap of keys to documents. */
  WorkQueue *workQueue;                       /* The list of workers waiting to be run. */
} Server;

typedef struct Command {
  char *name;                                 /* The name of the command. */
  unsigned int argc;                          /* The number of arguments the command takes. */
  char *(*fn)(char *a1, char *a2, char *a3);  /* The function that implements the command. */
} Command;

Server *server;                               /* Global server pointer. */


/********************************************************************************
 *                                Worker queue.
 *******************************************************************************/

/*
 * Create a new client.
 *
 * @param fd: The file descriptor of the client.
 * @return The newly created client instance.
 */
static Client *clientCreate(int fd) {
  Client *client = mmalloc(sizeof(Client));
  client->fd = fd;
  return client;
}

/*
 * Free an existing client.
 *
 * @param client: The client to free.
 */
static void clientFree(void *client) {
  assert(client != NULL);
  Client *cli = (Client*) client;
  mfree(cli);
}

/*
 * Create a copy of a client.
 *
 * @param client: The client to copy.
 */
static Client *clientCopy(Client *client) {
  assert(client != NULL);
  return clientCreate(client->fd);
}

/*
 * Initialize the server's work queue.
 */
static void workQueueCreate(void) {
  assert(server != NULL);
  server->workQueue = mmalloc(sizeof(WorkQueue));
  server->workQueue->clients = listCreate(LIST_TYPE_LINKED, &clientFree);
}

/*
 * Free the server's work queue.
 */
static void workQueueFree(void) {
  assert(server != NULL);
  assert(server->workQueue != NULL);
  listFree(server->workQueue->clients);
  mfree(server->workQueue);
}

/*
 * Add a new client to the work queue.
 *
 * @param client: The client to add.
 */
static void workQueuePush(Client *client) {
  assert(server != NULL);
  assert(server->workQueue != NULL);

  WorkQueue *queue = server->workQueue;
  pthread_mutex_lock(&queue->mutex);

  listAppend(queue->clients, client);
  pthread_cond_signal(&queue->cv);
  pthread_mutex_unlock(&queue->mutex);
}

/*
 * Get the next client to service.
 *
 * @return The next client from the queue.
 */
static Client *workQueuePop(void) {
  assert(server != NULL);
  assert(server->workQueue != NULL);

  WorkQueue *queue = server->workQueue;
  pthread_mutex_lock(&queue->mutex);

  while (listLength(queue->clients) == 0)
    pthread_cond_wait(&queue->cv, &queue->mutex);

  Client *client = clientCopy(listGet(queue->clients, 0));
  listRemove(queue->clients, 0);
  pthread_mutex_unlock(&queue->mutex);

  return client;
}


/********************************************************************************
 *                               Server creation.
 *******************************************************************************/

/*
 * Log the message to the server's log file,
 * if its level is above the server's verbosity level.
 *
 * @param level: The debug level of the message.
 * @param message: The message to log.
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

  /* Work queue. */
  workQueueCreate();

  /* Start accepting connections. */
  if ((bind(server->fd, (struct sockaddr*) server->addr, sizeof(SockAddr))) < 0) {
    fprintf(stderr, "Could not bind to socket.\n");
    abort();
  }
  listen(server->fd, MAX_CLIENTS);

  serverLog(LOG_LEVEL_INFO, "Starting RTDoc server on port %d\n", server->port);
  serverLog(LOG_LEVEL_DEBUG, "Debug mode on\n");
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
  workQueueFree();
  mfree(server);
}


/********************************************************************************
 *                            Store actions.
 *******************************************************************************/

#define CHAR_LIMIT  ' '
#define OK          "ok"
#define NIL         "nil"


/*
 * Called on an invalid command to the server.
 *
 * @param command: The command attempted to be executed.
 * @return A message indicating the command is invalid.
 */
static char *serverInvalidCommand(char *command, char *unused1, char *unused2) {
  assert(command != NULL);
  UNUSED(unused1); UNUSED(unused2);

  int i = 0;
  while (command[i] > CHAR_LIMIT)
    i++;

  char *output = mmalloc(20 + i);
  sprintf(output, "Invalid command ");
  strncat(output, command, i);
  strcat(output, "\n");
  return output;
}

/*
 * Simple ping to the server.
 *
 * @return 'pong' on success.
 */
static char *serverPing(char *unused1, char *unused2, char *unused3) {
  UNUSED(unused1); UNUSED(unused2); UNUSED(unused3);
  char *output = mmalloc(6);
  sprintf(output, "pong\n");
  return output;
}

/*
 * Add a new document to the document store.
 *
 * @param key: The document's identifier.
 * @param contents: The initial contents of the document.
 * @return The status code of the action.
 */
static char *serverAddDocument(char *key, char *contents, char *unused) {
  assert(key != NULL);
  assert(contents != NULL);
  UNUSED(unused);

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
static char *serverGetDocumentContents(char *key, char *unused1, char *unused2) {
  assert(key != NULL);
  UNUSED(unused1); UNUSED(unused2);

  Document *doc;

  if ((doc = serverGetDocument(key)) == NULL)
    return NIL;

  return jsonStringify(documentGetContents(doc));
}

static char *serverRemoveDocument(char *key, char *unused1, char *unused2) {
  assert(key != NULL);
  UNUSED(unused1); UNUSED(unused2);
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
static char *serverAddCollaborator(char *key, char *userId, char *unused) {
  assert(key != NULL);
  assert(userId != NULL);
  UNUSED(unused);

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
static char *serverRemoveCollaborator(char *key, char *userId, char *unused) {
  assert(key != NULL);
  assert(userId != NULL);
  UNUSED(unused);

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

#define NUM_COMMANDS  7

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
 * Read bytes from a given client.
 *
 * @param fd: The file descriptor of the client.
 * @param buffer: The buffer to read into.
 * @return The number of bytes read.
 */
static int serverRead(int fd, char *buffer) {
  assert(server != NULL);
  assert(buffer != NULL);
  memset(buffer, 0, BUFFER_SIZE);
  return read(fd, buffer, BUFFER_SIZE);
}

/*
 * Write bytes to a given client and clear the buffer.
 *
 * @param fd: The file descriptor of the client.
 * @param message: The message to write.
 * @return The number of bytes written.
 */
static int serverWrite(int fd, char *message) {
  assert(server != NULL);
  assert(message != NULL);
  return write(fd, message, strlen(message));
}

/*
 * Run a command.
 *
 * @param command: The command along with its arguments.
 * @return The output of the command.
 */
static char *runCommand(char *command) {
  assert(command != NULL);

  Command comm;
  for (int i = 0; i < NUM_COMMANDS; i++) {
    comm = commandTable[i];
    if (!strncmp(command, comm.name, strlen(comm.name))) {
      if (comm.argc == 0)
        return comm.fn(NULL, NULL, NULL);
    }
  }
  return serverInvalidCommand(command, NULL, NULL);
}

/*
 * Handle a single client.
 *
 * @param client: The request to handle.
 */
static void handleClientRequest(Client *client) {
  assert(client != NULL);

  char buffer[BUFFER_SIZE], *output;

  /* Accept requests in a loop. */
  while (serverRead(client->fd, buffer) > 0) {
    serverLog(LOG_LEVEL_DEBUG, buffer);
    output = runCommand(buffer);
    if (serverWrite(client->fd, output) < 0) {
      serverLog(LOG_LEVEL_INFO, "Client disconnected.\n");
      mfree(output);
      mfree(client);
      break;
    }
    mfree(output);
  }
  close(client->fd);
  clientFree(client);
}

/*
 * The procedure each thread will run.
 * Continuously pop clients from the queue and run the operations.
 */
static void *serverThreadJob(void *unused) {
  UNUSED(unused);
  while (true)
    handleClientRequest(workQueuePop());
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

  int clientfd;

  /* Create threading system. */
  pthread_t threads[MAX_CLIENTS];
  for (int i = 0; i < MAX_CLIENTS; i++)
    if (pthread_create(&threads[i], NULL, serverThreadJob, NULL))

  /* Catch interrupts for cleanup. */
  signal(SIGINT, interruptHandler);

  /* Accept requests in a loop. */
  while (true) {
    if ((clientfd = accept(server->fd, NULL, NULL)) < 0) {
      serverLog(LOG_LEVEL_ERROR, "Error accepting client.\n");
      continue;
    }
    serverLog(LOG_LEVEL_DEBUG, "Connected %d\n", clientfd);
    workQueuePush(clientCreate(clientfd));
  }

  serverFree();
}
