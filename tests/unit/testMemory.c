#include "../lib.h"
#include "testMemory.h"
#include "../../src/mmalloc.h"

#include <string.h>


int OOMSize;


static void teardown(void) {
  assertEqual(0, memoryUsage());
  assertEqual(0, memoryLimit());
  assertEqual(0, OOMSize);
}

static void OOMHandler(size_t size) {
  OOMSize = size;
}


static void testMalloc(void) {
  int sizes[7] = {1, 2, 3, 4, 8, 16, 32};
  void *ptr;
  size_t size;

  for (int i = 0; i < arraySize(sizes); i++) {
    size = 1 >> sizes[i];
    ptr = mmalloc(size);
    assertEqual(size, msize(ptr));
    mfree(ptr);
  }
}

static void testMallocZero(void) {
  void *ptr = mmalloc(0);
  assertNotNull(ptr);
  assertEqual(0, msize(ptr));
  mfree(ptr);
}

static void testFreeNull(void) {
  void *ptr = NULL;
  mfree(ptr);
  assertNull(ptr);
}

static void testCalloc(void) {
  Box *box = mcalloc(sizeof(Box));
  assertEqual(0, box->value);
  mfree(box);
}

static void testRealloc(void) {
  size_t s1 = 1024, s2 = 4096;
  void *ptr = mmalloc(s1);
  assertEqual(s1, msize(ptr));
  assertEqual(s1, memoryUsage());
  ptr = mrealloc(ptr, s2);
  assertEqual(s2, msize(ptr));
  assertEqual(s2, memoryUsage());
  mfree(ptr);
}

static void testReallocSmaller(void) {
  size_t s1 = 4096, s2 = 1024;
  void *ptr = mmalloc(s1);
  ptr = mrealloc(ptr, s2);
  assertEqual(s2, msize(ptr));
  assertEqual(s2, memoryUsage());
  mfree(ptr);
}

static void testReallocMany(void) {
  void *ptr = mmalloc(1);
  size_t sizes[] = {1, 2, 4, 8, 36, 128, 512, 4096, 32864};
  for (int i = 0; i < arraySize(sizes); i++) {
    ptr = mrealloc(ptr, sizes[i]);
    assertEqual(sizes[i], msize(ptr));
    assertEqual(sizes[i], memoryUsage());
  }
  mfree(ptr);
}

static void testReallocPreserveData(void) {
  size_t s1 = 6, s2 = 32;
  char *ptr = mmalloc(s1);
  strcpy(ptr, "hello");
  ptr = mrealloc(ptr, s2);
  assertStringEqual("hello", ptr);
  mfree(ptr);
}

static void testMemoryUsage(void) {
  Box *boxes[24];
  int numBoxes = arraySize(boxes);
  for (int i = 0; i < numBoxes; i++) {
    boxes[i] = mmalloc(sizeof(Box));
    assertEqual((i + 1) * sizeof(Box), memoryUsage());
  }
  for (int i = 0; i < numBoxes; i++) {
    mfree(boxes[i]);
    assertEqual((numBoxes - i - 1) * sizeof(Box), memoryUsage());
  }
}

static void testMemoryLimit(void) {
  size_t limit = 4096, second = 1;
  setMemoryLimit(limit);
  setOOMHandler(&OOMHandler);
  void *ptr1, *ptr2;
  ptr1 = mmalloc(limit);
  ptr2 = mmalloc(second);
  assertNotNull(ptr1);
  assertEqual(second, OOMSize);
  mfree(ptr1);
  setMemoryLimit(0);
  OOMSize = 0;
}

static void testSetMemoryLimit(void) {
  assertEqual(0, memoryLimit());
  size_t newLimit = 4096;
  setMemoryLimit(newLimit);
  assertEqual(newLimit, memoryLimit());
  setMemoryLimit(0);
}

TestSuite *memoryTestSuite() {
  TestSuite *suite = testSuiteCreate("memory aware allocations", NULL, &teardown);
  testSuiteAdd(suite, "malloc", &testMalloc);
  testSuiteAdd(suite, "malloc zero", &testMallocZero);
  testSuiteAdd(suite, "free null pointer", &testFreeNull);
  testSuiteAdd(suite, "calloc", &testCalloc);
  testSuiteAdd(suite, "realloc", &testRealloc);
  testSuiteAdd(suite, "realloc smaller", &testReallocSmaller);
  testSuiteAdd(suite, "realloc many", &testReallocMany);
  testSuiteAdd(suite, "realloc preserve data", &testReallocPreserveData);
  testSuiteAdd(suite, "memory usage tracking", &testMemoryUsage);
  testSuiteAdd(suite, "respect memory limit", &testMemoryLimit);
  testSuiteAdd(suite, "set memory limit", &testSetMemoryLimit);
  return suite;
}
