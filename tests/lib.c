#include "lib.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define SUITE_MAX_TESTS 256

char assertError[512];

/********************************************************************************
 *                            Structs for mock objects.
 *******************************************************************************/

Box *boxCreate() {
  Box *b = malloc(sizeof(Box));
  return b;
}

void *boxCopy(void *box) {
  Box *copy = boxCreate();
  copy->value = ((Box*) box)->value;
  return copy;
}

void boxFree(void *box) {
  Box *b = (Box*) box;
  free(b);
}

int boxEquals(void *box1, void *box2) {
  Box *b1 = (Box*) box1;
  Box *b2 = (Box*) box2;
  return b1->value == b2->value;
}


/********************************************************************************
 *                            Test suite functions.
 *******************************************************************************/

typedef struct TestCase {
  char *name;                 /* The name of the test. */
  void (*test)(void);         /* The pointer to the test function. */
} TestCase;

struct TestSuite {
  char *name;                             /* The name of the test suite. */
  unsigned int numTests;                  /* The number of tests in the suite. */
  TestCase *tests[SUITE_MAX_TESTS];        /* Array of test cases. */
  void (*setup)(void);                    /* The setup function. */
  void (*teardown)(void);                 /* The teardown function. */
};


/*
 * Create a new test suite.
 *
 * @param name: The name of the test suite.
 * @param setup: The function to call before each test runs.
 * @param teardown: The function to call after each test runs.
 *
 * @return The initialized test suite.
 */
TestSuite *testSuiteCreate(char *name, void (*setup)(void), void (*teardown)(void)) {
  TestSuite *suite = malloc(sizeof(TestSuite));
  suite->name = name;
  suite->numTests = 0;
  suite->setup = setup;
  suite->teardown = teardown;

  return suite;
}

/*
 * Free the memory of a test suite.
 *
 * @param suite: The suite to free.
 */
void testSuiteFree(TestSuite *suite) {
  assert(suite != NULL);
  
  for (int i = 0; i < suite->numTests; i++)
    free(suite->tests[i]);
  free(suite);
}

/*
 * Get the name of the test suite.
 *
 * @param suite: The suite to get the name of.
 * @return The suite's name.
 */
char *testSuiteName(TestSuite *suite) {
  return suite->name;
}

/*
 * Get the number of tests in the suite.
 *
 * @param suite: The suite to get the size of.
 * @return The number of tests in the suite.
 */
unsigned int testSuiteNumTests(TestSuite *suite) {
  return suite->numTests;
}

/*
 * Add a new test to the suite.
 *
 * @param suite: The test suite to add to.
 * @param name: The name of the test.
 * @param test: The function pointer to the test.
 */
void testSuiteAdd(TestSuite *suite, char *name, void (*test)(void)) {
  assert(suite->numTests + 1 < SUITE_MAX_TESTS);

  TestCase *tc = malloc(sizeof(TestCase));
  tc->name = name;
  tc->test= test;
  suite->tests[suite->numTests] = tc;
  suite->numTests++;
}

/*
 * Run a test suite.
 *
 * @param suite: The suite to run.
 * @return The number of failures.
 */
int testSuiteRun(TestSuite *suite) {
  TestCase *tc;
  int numFailed = 0;

  for (int i =  0; i < suite->numTests; i++) {
    tc = suite->tests[i];
    printf("%s                ", tc->name);

    *assertError = '\0';
    suite->setup();

    tc->test();
    if (strlen(assertError)) {
      printf("FAIL (%s)\n", assertError);
      numFailed++;
    } else {
      printf("PASS\n");
    }

    suite->teardown();
  }
  
  return numFailed;
}


/********************************************************************************
 *                                Assert functions.
 *******************************************************************************/

void assertEqual(int value1, int value2) {
  if (value1 != value2)
    sprintf(assertError, "%d does not equal %d", value1, value2);
}

void assertNotEqual(int value1, int value2) {
  if (value1 == value2)
    sprintf(assertError, "%d equals %d", value1, value2);
}

void assertDoubleEqual(double value1, double value2) {
  if (value1 != value2)
    sprintf(assertError, "%f does not equal %f", value1, value2);
}

void assertDoubleNotEqual(double value1, double value2) {
  if (value1 == value2)
    sprintf(assertError, "%f equals %f", value1, value2);
}

void assertPointerEqual(void *value1, void *value2) {
  if (value1 != value2)
    sprintf(assertError, "%p does not equal %p", value1, value2);
}

void assertPointerNotEqual(void *value1, void *value2) {
  if (value1 == value2)
    sprintf(assertError, "%p equals %p", value1, value2);
}

void assertEqualString(char *string1, char *string2) {
  if (!strcmp(string1, string2))
    sprintf(assertError, "%s does not equal %s", string1, string2);
}

void assertNotEqualString(char *string1, char *string2) {
  if (strcmp(string1, string2))
    sprintf(assertError, "%s equals %s", string1, string2);
}

void assertTrue(bool status) {
  if (!status)
    sprintf(assertError, "%d is not true", status);
}

void assertFalse(bool status) {
  if (status)
    sprintf(assertError, "%d is not false", status);
}

