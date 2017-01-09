#include "lib.h"
#include "unit/testDict.h"
#include "unit/testDoc.h"
#include "unit/testJson.h"
#include "unit/testList.h"
#include "unit/testMemory.h"

#include <stdio.h>
#include <string.h>


/*
 * Find a test case (or suite) with the given name and run it.
 */
static void runTestCase(TestSuite **suites, char *name, unsigned int numSuites) {
  TestCase *tc;
  for (int i = 0; i < numSuites; i++) {
    if (!strcmp(name, testSuiteName(suites[i]))) {
      testSuiteRun(suites[i]);
      return;
    }
    if ((tc = testSuiteGet(suites[i], name)) != NULL) {
      testCaseRun(tc);
      return;
    }
  }
  printf("Could not load test case %s\n", name);
}

/*
 * Run all test suites.
 */
static void runTestSuites(TestSuite **suites, unsigned int numSuites) {
  int numRun = 0, numFailed = 0;

  for (int i = 0; i < numSuites; i++) {
    printf("SUITE: %s\n", testSuiteName(suites[i]));
    numRun += testSuiteNumTests(suites[i]);
    numFailed += testSuiteRun(suites[i]);
  }

  printf("Ran %d tests. %d failed.\n", numRun, numFailed);
}

/*
 * Procedure to import and run test suites.
 */
int main(int argc, char **argv) {
  TestSuite *suites[] = {
    memoryTestSuite(),
    listTestSuite(),
    dictTestSuite(),
    jsonTestSuite(),
    documentTestSuite()
  };

  if (argc > 1)
    runTestCase(suites, argv[1], arraySize(suites));
  else
    runTestSuites(suites, arraySize(suites));

  for (int i = 0; i < arraySize(suites); i++)
    testSuiteFree(suites[i]);

  return 0;
}
