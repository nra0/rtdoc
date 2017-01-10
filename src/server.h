#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdbool.h>


/*
 * Database server; handles all reads and writes from clients.
 */

void serverStart(unsigned int port, bool verbose);
void serverFree(void);

#endif
