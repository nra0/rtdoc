#include "cli.h"
#include "server.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define SERVER_DEFAULT_PORT 7890
#define SERVER_MAX_CLIENTS  16


int main(int argc, char **argv) {
  /* Default settings. */
  bool client;
  int port = SERVER_DEFAULT_PORT,
      maxClients = SERVER_MAX_CLIENTS;
  char logFile[128] = "";
  char host[64] = "localhost";
  LogLevel verbosity = LOG_LEVEL_INFO;
  char opt;

  /* Custom command line options. */
  while ((opt = getopt(argc, argv, "cdh:l:n:p:")) != -1) {
    switch (opt) {
      case 'c': client = true; break;
      case 'd': verbosity = LOG_LEVEL_DEBUG; break;
      case 'h': strcpy(host, optarg); break;
      case 'l': strcpy(logFile, optarg); break;
      case 'n': maxClients = atoi(optarg); break;
      case 'p': port = atoi(optarg); break;
    }
  }

  /* Run the program. */
  if (client)
    clientStart(host, port);
  else
    serverStart(port, verbosity, logFile, maxClients);
}
