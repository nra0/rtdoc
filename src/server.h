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
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_ALL
} LogLevel;

void serverStart(unsigned int port, LogLevel verbosity, FILE *logFile);

#endif
