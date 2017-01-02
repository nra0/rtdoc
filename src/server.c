#include "mmalloc.h"
#include "server.h"

#include <assert.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#define DEFAULT_PORT 7890
#define BUFFER_SIZE  1024
#define BACKLOG_SIZE 128

typedef struct sockaddr_in SockAddr;


/********************************************************************************
 *                              Struct declarations
 *******************************************************************************/

struct RTServer {
  unsigned int port;  /* The port the server listens to. */
  int fd;             /* The file descriptor of the server. */
  SockAddr *addr;     /* The address of the server. */
  bool verbose;       /* If set, the will output each request made to it. */
};

/********************************************************************************
 *                               Server creation
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

  server->port = DEFAULT_PORT;
  server->fd = socket(AF_INET, SOCK_STREAM, 0);
  server->addr = serverAddr;
  server->verbose = false;

  serverAddr->sin_family = AF_INET;
  serverAddr->sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddr->sin_port = htons(server->port);
  return server;
}

/*
 * Set the port that the server should listen to.
 *
 * @param server: The server to modify.
 * @param port: The port the server should listen to.
 */
void serverSetPort(RTServer *server, unsigned int port) {
  assert(server != NULL);
  server->port = port != 0 ? port : DEFAULT_PORT;
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
