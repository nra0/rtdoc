#include "../lib.h"
#include "testServer.h"
#include "../../src/server.h"
#include "../../src/mmalloc.h"


static void teardown(void) {
  assertEqual(0, memoryUsage());
}


TestSuite *serverTestSuite() {
  TestSuite *suite = testSuiteCreate("server operations", NULL, &teardown);
  return suite;
}
