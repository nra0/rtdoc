#ifndef __TEST_LIB_H__
#define __TEST_LIB_H__

#include <stdbool.h>

#define __TESTING__


/*
 * Test utility functions.
 */

typedef struct TestSuite TestSuite;

/* Test suite functions. */
TestSuite *testSuiteCreate(char *name, void (*setup), void (*teardown));
void testSuiteFree(TestSuite *suite);
void testSuiteAdd(TestSuite *suite, char *name, void (*test));
void testSuiteRun(TestSuite *suite);

/* Assert functions */
void assertEqual(void *value1, void *value2);
void assertEqualString(char *string1, char *string2);
void assertTrue(bool status);
void assertFalse(bool status);

#endif

