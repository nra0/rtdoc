#include "../lib.h"
#include "testServer.h"
#include "../../src/server.h"


static void teardown(void) {
  assertEqual(0, memoryUsage());
}


TestSuite *serverTestSuite() {
  TestSuite *suite = testSuiteCreate("server operations", NULL, &teardown);
  return suite;
}
