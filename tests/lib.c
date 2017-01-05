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

typedef struct TestCase {
  char *name;                 /* The name of the test. */
  void (*test)(void);         /* The pointer to the test function. */
} TestCase;

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
 * Run a test suite.
 *
 * @param suite: The suite to run.
 * @return The number of failures.
 */
int testSuiteRun(TestSuite *suite) {
  TestCase *tc;
  int numFailed = 0;

  for (int i =  0; i < suite->numTests; i++) {
    resetAssertError();
    tc = suite->tests[i];
    printf("%-40s", tc->name);

    if (suite->setup != NULL)
      suite->setup();
    tc->test();
    if (suite->teardown != NULL)
      suite->teardown();

    if (strlen(assertErrorMessage)) {
      printf("%s✗\n%s%s%s", COLOR_RED, COLOR_MAGENTA, assertErrorMessage, COLOR_NORMAL);
      numFailed++;
    } else {
      printf("%s✓%s\n", COLOR_GREEN, COLOR_NORMAL);
    }
  }

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
