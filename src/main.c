#include "cli.h"
#include "server.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define SERVER_DEFAULT_PORT 7890


int main(int argc, char **argv) {
  bool client;
  int port = SERVER_DEFAULT_PORT;
  FILE *logFile = stdout;
  char host[64] = "localhost";
  LogLevel verbosity = LOG_LEVEL_ERROR;
  char opt;

  while ((opt = getopt(argc, argv, "cdh:l:p:")) != -1) {
    switch (opt) {
      case 'c': client = true; break;
      case 'd': verbosity = LOG_LEVEL_DEBUG; break;
      case 'h': strcpy(host, optarg); break;
      case 'l': logFile = fopen(optarg, "a"); break;
      case 'p': port = atoi(optarg); break;
    }
  }

  if (client)
    clientStart(host, port);
  else
    serverStart(port, verbosity, logFile);
}
