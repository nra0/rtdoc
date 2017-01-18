#include "../lib.h"
#include "testOt.h"
#include "../../src/ot.h"
#include "../../src/mmalloc.h"


static void teardown(void) {
  assertEqual(0, memoryUsage());
}


TestSuite *otTestSuite() {
  TestSuite *suite = testSuiteCreate("operational transforms", NULL, &teardown);
  return suite;
}
