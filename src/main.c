#include "cli.h"
#include "server.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char **argv) {
  bool client, quiet;
  int port;
  char opt;

  while ((opt = getopt(argc, argv, "cp:s")) != -1) {
    switch (opt) {
      case 'c': client = true; break;
      case 'p': port = atoi(optarg); break;
      case 's': quiet = true; break;
    }
  }

  if (client) {
    printf("Client main method...\n");
  } else {
    RTServer *server = serverCreate();
    serverSetPort(server, port);
    serverSetVerbosity(server, !quiet);
    serverStart(server);
  }
}
