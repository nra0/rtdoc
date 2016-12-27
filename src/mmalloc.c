#include "mmalloc.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


#define PREFIX_SIZE (sizeof(size_t))  /* In front of each block, we have a prefix that tracks the block size. */


size_t mLimit;                        /* The maximum amount of memory to allocate. */
size_t totalAllocated;                /* The amount of memory allocated so far. */
void (*OOMHandler)(size_t size);      /* The user set OOM handler function */

/* The types of ways we can allocate memory. */
typedef enum allocType {
  T_MALLOC,
  T_CALLOC,
  T_REALLOC
} allocType;


/*
 * Check that allocating the amount falls within the memory limit.
 *
 * @param size: The number of bytes about to be allocated.
 * @returns Whether the memory limit will be satisfied.
 */
inline static bool canAllocate(size_t size) {
  return mLimit == 0 || (size + totalAllocated <= mLimit);
}



/*
 * Out of memory error; Called when the memory limit is exceeded.
 *
 * @param size: The amount of memory attempted to be allocated.
 */
static void mmallocOOM(size_t size) {
  if (OOMHandler != NULL) {
    OOMHandler(size);
  } else {
    /* Default behavior, if the user has not set a handler. */
    fprintf(stderr, "Out of memory error while trying to allocate %zu bytes.\n", size);
    fflush(stderr);
    abort();
  }
}

/*
 * Either malloc or calloc a block of memory.
 *
 * @param size: The number of bytes to allocate.
 * @param type: How to allocate the memory.
 * @param ptr: If reallocating, the original pointer.
 * @return The pointer to the memory.
 */
static void *allocate(size_t size, allocType type, void *ptr) {
  size_t *pointer;
  size_t allocSize = PREFIX_SIZE + size;

  if (!canAllocate(size)) {
    mmallocOOM(size);
    return NULL;
  }

  switch (type) {
    case T_MALLOC: pointer = malloc(allocSize); break;
    case T_CALLOC: pointer = calloc(1, allocSize); break;
    case T_REALLOC: totalAllocated -= msize(ptr);
                    pointer = realloc(ptr - PREFIX_SIZE, allocSize); break;
  }

  if (pointer == NULL)
    mmallocOOM(size);

  totalAllocated += size;
  *pointer = size;

  return ((void*) pointer) + PREFIX_SIZE;
}

/*
 * Allocate a contiguous block of memory with a given size.
 *
 * @param size: The number of bytes to allocate.
 * @return The pointer to the allocated memory.
 */
inline void *mmalloc(size_t size) {
  return allocate(size, T_MALLOC, NULL);
}

/*
 * Allocate a zeroed block of memory.
 *
 * @param size: The number of bytes to allocate.
 * @return The pointer to the memory.
 */
inline void *mcalloc(size_t size) {
  return allocate(size, T_CALLOC, NULL);
}

/*
 * Reallocate (change the size of) memory for an existing pointer.
 *
 * @param ptr: The pointer to reallocate.
 * @param size: The new number of bytes.
 * @return The new pointer.
 */
inline void *mrealloc(void *ptr, size_t size) {
  return allocate(size, T_REALLOC, ptr);
}

/*
 * Check the size of a pointer.
 *
 * @param ptr: The pointer to measure the size of.
 * @return The number of bytes the pointer points to.
 */
size_t msize(void *ptr) {
  assert(ptr != NULL);
  return *((size_t*) ptr - 1);
}

/*
 * Free a pointer.
 *
 * @param ptr: The pointer to free.
 */
void mfree(void *ptr) {
  if (ptr == NULL)
    return;
  totalAllocated -= msize(ptr);
  free(ptr - PREFIX_SIZE);
}

/*
 * Get the current memory limit.
 *
 * @return The memory limit.
 */
inline size_t memoryLimit(void) {
  return mLimit;
}

/*
 * Set the maximum amount of memory the program should allocate.
 * This does not include prefixes which hold size information; only the actual
 * requested number of allocated bytes.
 *
 * @param size: The new limit.
 */
inline void setMemoryLimit(size_t size) {
  mLimit = size;
}

/*
 * Get the amount of memory used.
 * This does not include prefix metadata, only the actual requested amount of memory allocated.
 *
 * @return The number of bytes currently allocated.
 */
inline size_t memoryUsage(void) {
  return totalAllocated;
}

/*
 * Set the function to handler out of memory errors.
 *
 * @param size: The size attempting to be allocated when the error occured.
 */
void setOOMHandler(void (*handler)(size_t size)) {
  OOMHandler = handler;
}
