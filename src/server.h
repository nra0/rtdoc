#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdbool.h>
#include <stdio.h>


/*
 * Database server; handles all reads and writes from clients.
 */

typedef enum LogLevel {
  LOG_LEVEL_OFF,
  LOG_LEVEL_FATAL,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_INFO,
  LOG_LEVEL_DEBUG
} LogLevel;

void serverCreate(unsigned int port, LogLevel verbosity, char *logFile, unsigned int maxClients);
void serverStart(unsigned int port, LogLevel verbosity, char *logFile, unsigned int maxClients);
char *serverRunCommand(char *command);
void serverFree(void);

#endif
