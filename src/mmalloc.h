#ifndef __MMALLOC_H__
#define __MMALLOC_H__

#include <stddef.h>

/*
 * Memory allocator that keeps track of memory usage.
 */

void *mmalloc(size_t size);
void *mcalloc(size_t size);
void *mrealloc(void *ptr, size_t size);
size_t msize(void *ptr);
void mfree(void *ptr);
size_t memoryLimit(void);
void setMemoryLimit(size_t size);
size_t memoryUsage(void);
size_t memorySize(void);
void setOOMHandler(void (*handler)(size_t size));

#endif
