#include "lib.h"
#include "unit/testDict.h"
#include "unit/testJson.h"
#include "unit/testList.h"
#include "unit/testMmalloc.h"

#include <stdio.h>


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

  unsigned int numRun = 0, numFailed = 0;

  for (int i = 0; i < arraySize(suites); i++) {
    printf("SUITE: %s\n", testSuiteName(suites[i]));
    numRun += testSuiteNumTests(suites[i]);
    numFailed += testSuiteRun(suites[i]);
    testSuiteFree(suites[i]);
  }

  printf("Ran %d tests. %d failed.\n", numRun, numFailed);

  return 0;
}
