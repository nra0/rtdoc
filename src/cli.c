#include "cli.h"
#include "mmalloc.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


/********************************************************************************
 *                              Struct declarations.
 *******************************************************************************/

typedef struct Server {
  char *host;           /* The host the server is running on. */
  unsigned int port;    /* The port number of the server. */
} Server;

typedef struct Client {
  pid_t pid;            /* The process id of the client */
  Server *server;       /* The server the client is connected to. */
} Client;

Client *client;         /* Global client struct. */


/********************************************************************************
 *                           Creation and deletion.
 *******************************************************************************/

/*
 * Initialize the global client variable.
 *
 * @param host: The host the client should connect to.
 * @param port: The port of the server to listen to.
 */
static void clientCreate(const char *host, unsigned int port) {
  client = mmalloc(sizeof(Client));
  client->pid = getpid();
  client->server = mmalloc(sizeof(Server));
  client->server->host = mmalloc(strlen(host) + 1);
  strcpy(client->server->host, host);
  client->server->port = port;
}

/*
 * Free the client.
 */
static void clientFree(void) {
  assert(client != NULL);
  mfree(client->server->host);
  mfree(client->server);
  mfree(client);
}


/********************************************************************************
 *              Interface for the user to interact with the server.
 *******************************************************************************/

/*
 * Pint the client's server.
 */
static void clientPing(void) {
  assert(client != NULL);
}


/********************************************************************************
 *                             Launch the client.
 *******************************************************************************/

/*
 * Client main method.
 * Starts a client instance that interacts with a server at a given port.
 *
 * @param port: The port the server is running on.
 */
void clientStart(const char *host, unsigned int port) {
  assert(client == NULL);
  assert(host != NULL);

  clientCreate(host, port);

  char buffer[4096];

  while (true) {
    printf("rtdoc> ");
    fgets(buffer, sizeof(buffer), stdin);
    printf("%s", buffer);
  }
}
