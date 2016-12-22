#include "lib.h"
#include "unit/testList.h"

#include <stdio.h>


#define NUM_SUITES 1 


/*
 * Procedure to import and run test suites. Options can be specified with flags.
 */


int main(int argc, char **argv) {
  TestSuite *suites[NUM_SUITES];
  suites[0] = listTestSuite();

  unsigned int numRun = 0, numFailed = 0;

  for (int i = 0; i < NUM_SUITES; i++) {
    printf("SUITE: %s\n", testSuiteName(suites[i]));
    numRun += testSuiteNumTests(suites[i]);
    numFailed += testSuiteRun(suites[i]);
  }

  printf("Ran %d tests. %d failed.\n", numRun, numFailed);

  return 0;
}

