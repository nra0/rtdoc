#ifndef __TEST_LIB_H__
#define __TEST_LIB_H__

#include <stdbool.h>


/*
 * Mock structs.
 */

typedef struct Box {
  int value;
} Box;

Box *boxCreate(int value);
void *boxCopy(void *box);
void boxFree(void *box);
int boxValue(void *box);
int boxEquals(void *box1, void *box2);


#define arraySize(a) (sizeof(a) / sizeof(a[0]))


/*
 * Test utility functions.
 */

typedef struct TestSuite TestSuite;

/* Test suite functions. */
TestSuite *testSuiteCreate(char *name, void (*setup)(void), void (*teardown)(void));
void testSuiteFree(TestSuite *suite);
char *testSuiteName(TestSuite *suite);
unsigned int testSuiteNumTests(TestSuite *suite);
void testSuiteAdd(TestSuite *suite, char *name, void (*test)(void));
int testSuiteRun(TestSuite *suite);

/* Assert functions */
void assertEqual(int value1, int value2);
void assertNotEqual(int value1, int value2);
void assertDoubleEqual(double value1, double value2);
void assertDoubleNotEqual(double value1, double value2);
void assertPointerEqual(void *pointer1, void *pointer2);
void assertPointerNotEqual(void *pointer1, void *pointer2);
void assertNull(void *pointer);
void assertNotNull(void *pointer);
void assertStringEqual(char *string1, char *string2);
void assertStringNotEqual(char *string1, char *string2);
void assertTrue(bool status);
void assertFalse(bool status);

#endif
