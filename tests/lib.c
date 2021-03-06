#include "lib.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define SUITE_MAX_TESTS 256

#define COLOR_NORMAL    "\x1B[0m"
#define COLOR_RED       "\x1B[31m"
#define COLOR_GREEN     "\x1B[32m"
#define COLOR_YELLOW    "\x1B[33m"
#define COLOR_BLUE      "\x1B[34m"
#define COLOR_MAGENTA   "\x1B[35m"
#define COLOR_CYAN      "\x1B[36m"
#define COLOR_WHITE     "\x1B[37m"

#define COLOR_SUCCESS   COLOR_GREEN
#define COLOR_FAILURE   COLOR_RED
#define COLOR_ERROR     COLOR_MAGENTA

char assertErrorMessage[8092];
char *assertError;

/********************************************************************************
 *                            Structs for mock objects.
 *******************************************************************************/

Box *boxCreate(int value) {
  Box *box = malloc(sizeof(Box));
  box->value = value;
  return box;
}

void boxFree(void *box) {
  free(box);
}

int boxValue(void *box) {
  Box *b = (Box*) box;
  return b->value;
}


/********************************************************************************
 *                            Test suite functions.
 *******************************************************************************/

struct TestCase {
  char *name;                             /* The name of the test. */
  TestSuite *suite;                       /* The suite the test belongs to. */
  void (*test)(void);                     /* The pointer to the test function. */
};

struct TestSuite {
  char *name;                             /* The name of the test suite. */
  unsigned int numTests;                  /* The number of tests in the suite. */
  TestCase *tests[SUITE_MAX_TESTS];       /* Array of test cases. */
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
  tc->suite = suite;
  tc->test= test;
  suite->tests[suite->numTests] = tc;
  suite->numTests++;
}

static void resetAssertError(void) {
  if (assertError == NULL) {
    assertError = malloc(sizeof(char*));
  }
  assertError = assertErrorMessage;
  *assertError = '\0';
}

/*
 * Get a test case with the given name.
 *
 * @param suite: The suite to search.
 * @param name: The name of the test case.
 * @return The matching test case.
 */
TestCase *testSuiteGet(TestSuite *suite, char *name) {
  for (int i = 0; i < suite->numTests; i++) {
    TestCase *tc = suite->tests[i];
    if (!strcmp(name, tc->name))
      return tc;
  }
  return NULL;
}

/*
 * Run a test case.
 *
 * @param tc: The test case to run.
 * @return 0 if the test passed, 1 if it failed.
 */
int testCaseRun(TestCase *tc) {
  resetAssertError();
  printf("%-40s", tc->name);

  /* Setup, test, and teardown. */
  if (tc->suite->setup != NULL)
    tc->suite->setup();
  tc->test();
  if (tc->suite->teardown != NULL)
    tc->suite->teardown();

  if (strlen(assertErrorMessage)) {
    /* The test failed. */
    printf("%s✗\n%s%s%s", COLOR_FAILURE, COLOR_ERROR, assertErrorMessage, COLOR_NORMAL);
    return 1;
  } else {
    /* The test passed. */
    printf("%s✓%s\n", COLOR_SUCCESS, COLOR_NORMAL);
    return 0;
  }
}

/*
 * Run a test suite.
 *
 * @param suite: The suite to run.
 * @return The number of failures.
 */
int testSuiteRun(TestSuite *suite) {
  int numFailed = 0;

  for (int i = 0; i < suite->numTests; i++)
    numFailed += testCaseRun(suite->tests[i]);

  return numFailed;
}


/********************************************************************************
 *                                Assert functions.
 *******************************************************************************/

void assertEqual(int value1, int value2) {
  if (value1 != value2)
    assertError += sprintf(assertError, "%d does not equal %d\n", value1, value2);
}

void assertNotEqual(int value1, int value2) {
  if (value1 == value2)
    assertError += sprintf(assertError, "%d equals %d\n", value1, value2);
}

void assertDoubleEqual(double value1, double value2) {
  if (value1 != value2)
    assertError += sprintf(assertError, "%f does not equal %f\n", value1, value2);
}

void assertDoubleNotEqual(double value1, double value2) {
  if (value1 == value2)
    assertError += sprintf(assertError, "%f equals %f\n", value1, value2);
}

void assertPointerEqual(void *value1, void *value2) {
  if (value1 != value2)
    assertError += sprintf(assertError, "%p does not equal %p\n", value1, value2);
}

void assertPointerNotEqual(void *value1, void *value2) {
  if (value1 == value2)
    assertError += sprintf(assertError, "%p equals %p\n", value1, value2);
}

void assertNull(void *pointer) {
  if (pointer != NULL)
    assertError += sprintf(assertError, "%p is not NULL\n", pointer);
}

void assertNotNull(void *pointer) {
  if (pointer == NULL)
    assertError += sprintf(assertError, "%p is NULL\n", pointer);
}

void assertStringEqual(char *string1, char *string2) {
  if (strcmp(string1, string2))
    assertError += sprintf(assertError, "%s does not equal %s\n", string1, string2);
}

void assertStringNotEqual(char *string1, char *string2) {
  if (!strcmp(string1, string2))
    assertError += sprintf(assertError, "%s equals %s\n", string1, string2);
}

void assertTrue(bool status) {
  if (!status)
    assertError += sprintf(assertError, "%d is not true\n", status);
}

void assertFalse(bool status) {
  if (status)
    assertError += sprintf(assertError, "%d is not false\n", status);
}
