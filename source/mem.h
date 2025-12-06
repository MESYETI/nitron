#ifndef N_MEM_H
#define N_MEM_H

#include "common.h"

#define NEW(T) (T*) SafeAlloc(sizeof(T))

void* SafeAlloc(size_t size);
void* SafeRealloc(void* ptr, size_t size);

// custom memory allocator
typedef struct MemBlock MemBlock;

struct MemBlock {
	char      magic[7];
	MemBlock* prev;
	MemBlock* next;
	size_t    size;
	bool      free;
};

typedef struct {
	bool   available;
	size_t used;
	size_t total;
} MemUsage;

void     InitAllocator(void);
void     FreeAllocator(void);
void*    Alloc(size_t size);
void     Free(void* ptr);
void*    Realloc(void* ptr, size_t size);
MemUsage GetMemUsage(void);
void     DumpAllocator(void);

#endif
