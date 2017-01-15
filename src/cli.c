#include "cli.h"
#include "mmalloc.h"

#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#define BUFFER_SIZE 4096

typedef struct sockaddr_in SockAddr;
typedef struct hostent Host;


/********************************************************************************
 *                              Struct declarations.
 *******************************************************************************/

typedef struct Server {
  char *hostName;       /* The host the server is running on. */
  unsigned int port;    /* The port number of the server. */
  SockAddr *addr;       /* The address of the server. */
  Host *host;           /* The host server. */
} Server;

typedef struct Client {
  pid_t pid;            /* The process id of the client */
  Server server;        /* The server the client is connected to. */
  int fd;               /* The client file descriptor. */
} Client;

Client client;         /* Global client struct. */


/********************************************************************************
 *                           Creation and deletion.
 *******************************************************************************/

/*
 * Initialize the global client variable.
 *
 * @param hostName: The host the client should connect to.
 * @param port: The port of the server to listen to.
 */
static void clientCreate(const char *hostName, unsigned int port) {
  client.pid = getpid();

  /* Host and port numbers. */
  client.server.hostName = mmalloc(strlen(hostName) + 1);
  strcpy(client.server.hostName, hostName);
  client.server.port = port;

  /* Socket connection. */
  if ((client.fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "Could not open socket.\n");
    abort();
  }

  /* Get the host. */
  if ((client.server.host = gethostbyname(client.server.hostName)) == NULL) {
    fprintf(stderr, "Could not connect to host %s\n", client.server.hostName);
    abort();
  }

  /* Network configuration. */
  client.server.addr = mcalloc(sizeof(SockAddr));
  client.server.addr->sin_family = AF_INET;
  bcopy((char*) client.server.host->h_addr,
        (char*) &client.server.addr->sin_addr.s_addr,
        client.server.host->h_length);
  client.server.addr->sin_port = htons(client.server.port);

  /* Connect to the server. */
  if (connect(client.fd, (struct sockaddr*) client.server.addr, sizeof(SockAddr)) < 0) {
    fprintf(stderr, "Could not connect to server.\n");
    abort();
  }

  printf("Starting RTDoc client.\nConnected to %s on port %d.\n",
          client.server.hostName, client.server.port);
}

/*
 * Free the client.
 */
static void clientFree(void) {
  close(client.fd);
  mfree(client.server.host);
  mfree(client.server.addr);
}


/********************************************************************************
 *                             Launch the client.
 *******************************************************************************/

#define CLI_PROMPT    "rtdoc> "


/*
 * Called when the client disonnects from the server.
 */
static void clientDisconnected(void) {
  fprintf(stderr, "Disconnected from server!\n");
  abort();
}

/*
 * Read a message from the server.
 *
 * @param buffer: The buffer to write the message to.
 * @return The number of bytes received.
 */
static int clientRead(char *buffer) {
  assert(buffer != NULL);
  memset(buffer, 0, BUFFER_SIZE);
  return read(client.fd, buffer, BUFFER_SIZE);
}

/*
 * Write a message to the server and clear the buffer.
 *
 * @param message: The message to send.
 * @return The number of bytes sent.
 */
static int clientWrite(char *message) {
  assert(message != NULL);
  return write(client.fd, message, strlen(message));
}

/*
 * Client main method.
 * Starts a client instance that interacts with a server at a given port.
 *
 * @param port: The port the server is running on.
 */
void clientStart(const char *host, unsigned int port) {
  assert(host != NULL);

  clientCreate(host, port);

  char buffer[BUFFER_SIZE];

  while (true) {
    printf(CLI_PROMPT);
    fgets(buffer, sizeof(buffer), stdin);
    if (clientWrite(buffer) < 0 || clientRead(buffer) < 0)
      clientDisconnected();
    printf("%s", buffer);
  }

  clientFree();
}
