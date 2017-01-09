#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdbool.h>


/*
 * Database server; handles all reads and writes from clients.
 */

typedef struct RTServer RTServer;


/* Configuration methods. */
RTServer *serverCreate(void);
void serverFree(RTServer *server);
void serverSetPort(RTServer *server, unsigned int port);
void serverSetVerbosity(RTServer *server, bool verbose);
void serverStart(RTServer *server);

#endif
