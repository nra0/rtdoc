#include "../lib.h"
#include "testDoc.h"
#include "../../src/doc.h"
#include "../../src/mmalloc.h"


Document *doc;
Collaborator *user;


static void setup(void) {

}

static void teardown(void) {
  assertEqual(0, memoryUsage());
}


TestSuite *documentTestSuite() {
  TestSuite *suite = testSuiteCreate("Document and collaborators", &setup, &teardown);
  return suite;
}
