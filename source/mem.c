#include <stdio.h>
#include <string.h>
#include "mem.h"
#include "common.h"

void* SafeMalloc(size_t size) {
	void* ret = Alloc(size);

	if ((ret == NULL) && (size != 0)) {
		fprintf(stderr, "Malloc returned NULL\n");
		exit(1);
	}

	return ret;
}

void* SafeRealloc(void* ptr, size_t size) {
	void* ret = Realloc(ptr, size);

	if ((ret == NULL) && (size > 0)) {
		fprintf(stderr, "Malloc returned NULL\n");
		exit(1);
	}

	return ret;
}

typedef struct MemBlock MemBlock;

struct MemBlock {
	MemBlock* prev;
	MemBlock* next;
	size_t    size;
	bool      free;
};

static uint8_t*  zone;
static MemBlock* base;
static MemBlock* nextBlock;

void InitAllocator(void) {
	size_t zoneSize = 64 * 1024 * 1024;

	zone      = malloc(zoneSize);
	base      = (MemBlock*) zone;
	nextBlock = base;

	if (zone == NULL) {
		fprintf(stderr, "Failed to allocate zone\n");
		exit(1);
	}

	base->prev = NULL;
	base->next = NULL;
	base->size = zoneSize - sizeof(MemBlock);
	base->free = true;
}

void FreeAllocator(void) {
	free(zone);
}

#define MIN_LEFTOVER_SIZE 32
#define BLOCK_DATA(BLOCK) (((uint8_t*) (BLOCK)) + sizeof(MemBlock))
#define MEM_BLOCK(DATA)   (((MemBlock*) (DATA)) - 1)

void* Alloc(size_t size) {
	#ifdef N_SYS_ALLOC
		return malloc(size);
	#else
		if (size == 0) return NULL;

		MemBlock* start = nextBlock;

		while ((size > nextBlock->size) || !nextBlock->free) {
			nextBlock = nextBlock->next;

			if (!nextBlock) {
				nextBlock = base;
			}
			else if (nextBlock == start) {
				return NULL;
			}
		}

		nextBlock->free = false;

		if (
			(size < nextBlock->size) &&
			(nextBlock->size - size >= sizeof(MemBlock) + MIN_LEFTOVER_SIZE)
		) {
			MemBlock* leftover = (MemBlock*) (BLOCK_DATA(nextBlock) + size);
			leftover->size     = nextBlock->size - size - sizeof(MemBlock);
			leftover->free     = true;
			leftover->prev     = nextBlock;
			leftover->next     = nextBlock->next;
			nextBlock->size    = size;
			nextBlock->next    = leftover;

			if (leftover->next) {
				leftover->next->prev = leftover;
			}
		}

		MemBlock* block = nextBlock;
		nextBlock       = block->next;
		return BLOCK_DATA(block);
	#endif
}

static void MergeNext(MemBlock* block) {
	block->size += block->next->size + sizeof(MemBlock);
	block->next  = block->next->next;

	if (block->next) {
		block->next->prev = block;
	}
}

void Free(void* ptr) {
	#ifdef N_SYS_ALLOC
		free(ptr);
	#else
		MemBlock* block = MEM_BLOCK((uint8_t*) ptr);

		if (block->free) {
			fprintf(stderr, "Double free: %p\n", ptr);
			exit(1);
		}

		block->free = true;
		if (block->next) if (block->next->free) {
			MergeNext(block);
		}
		if (block->prev) if (block->prev->free) {
			block = block->prev;
			MergeNext(block);
		}

		nextBlock = block;
	#endif
}

void* Realloc(void* ptr, size_t size) {
	#ifdef N_SYS_ALLOC
		return realloc(ptr, size);
	#else
		if (ptr == NULL) {
			return Alloc(size);
		}
		else if (size == 0) {
			Free(ptr);
			return NULL;
		}

		MemBlock* oldBlock = MEM_BLOCK((uint8_t*) ptr);

		if (oldBlock->free) {
			fprintf(stderr, "Re-allocating freed block at %p\n", ptr);
			exit(1);
		}

		size_t oldSize = oldBlock->size;
		Free(ptr);

		MemBlock* newBlock = MEM_BLOCK((uint8_t*) Alloc(size));
		if (newBlock != oldBlock) {
			memmove(
				BLOCK_DATA(newBlock), BLOCK_DATA(oldBlock),
				oldSize > newBlock->size? newBlock->size : oldSize
			);
		}

		return BLOCK_DATA(newBlock);
	#endif
}

MemUsage GetMemUsage(void) {
	MemUsage ret;
	ret.available = true;
	ret.used      = 0;
	ret.total     = 64 * 1024 * 1024;

	for (MemBlock* block = base; block; block = block->next) {
		if (!block->free) {
			ret.used += block->size + sizeof(MemBlock);
		}
	}

	return ret;
}

void DumpAllocator(void) {
	for (MemBlock* block = base; block; block = block->next) {
		printf("BLOCK %s: %d bytes", block->free? "free" : "used", block->size);
	}
}
