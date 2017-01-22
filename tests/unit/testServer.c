#include "../lib.h"
#include "testServer.h"
#include "../../src/server.h"
#include "../../src/mmalloc.h"


#define TEST_PORT 9876


char *output;


static void setup(void) {
  serverCreate(TEST_PORT, LOG_LEVEL_OFF, "", 1);
}

static void teardown(void) {
  serverFree();
  assertEqual(0, memoryUsage());
}


static void testServerPing(void) {
  output = serverRunCommand("ping");
  assertStringEqual("pong\n", output);
  mfree(output);
}

static void testServerInvalidCommand(void) {
  char *invalidCommands[] = {
    "abc",
    "invalid-command",
    "f",
    "235.2"
  };
  char error[256];
  for (int i = 0; i < arraySize(invalidCommands); i++) {
    sprintf(error, "Invalid command %s\n", invalidCommands[i]);
    output = serverRunCommand(invalidCommands[i]);
    assertStringEqual(error, output);
    mfree(output);
  }
}


TestSuite *serverTestSuite() {
  TestSuite *suite = testSuiteCreate("server operations", &setup, &teardown);
  testSuiteAdd(suite, "basic ping", &testServerPing);
  testSuiteAdd(suite, "invalid command", &testServerInvalidCommand);
  return suite;
}
