#include "lib.h"
#include "unit/testDict.h"
#include "unit/testJson.h"
#include "unit/testList.h"
#include "unit/testMmalloc.h"

#include <stdio.h>


TestCase *findTestCase(TestSuite **suites, char *name, unsigned int numSuites) {
  TestCase *tc;
  for (int i = 0; i < numSuites; i++)
    if ((tc = testSuiteGet(suites[i], name)) != NULL)
      return tc;
  return NULL;
}

/*
 * Procedure to import and run test suites.
 */
int main(int argc, char **argv) {
  TestSuite *suites[] = {
    mmallocTestSuite(),
    listTestSuite(),
    dictTestSuite(),
    jsonTestSuite()
  };

  if (argc > 1) {
    /* Run a specific test. */
    TestCase *tc = findTestCase(suites, argv[1], arraySize(suites));
    if (tc == NULL) {
      printf("Could not load test case %s\n", argv[1]);
      return 1;
    }
    testCaseRun(tc);
    return 0;
  }

  /* Run all the tests. */
  int numRun = 0, numFailed = 0;

  for (int i = 0; i < arraySize(suites); i++) {
    printf("SUITE: %s\n", testSuiteName(suites[i]));
    numRun += testSuiteNumTests(suites[i]);
    numFailed += testSuiteRun(suites[i]);
    testSuiteFree(suites[i]);
  }

  printf("Ran %d tests. %d failed.\n", numRun, numFailed);

  return 0;
}
