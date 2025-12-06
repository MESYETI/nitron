#include <string.h>
#include "mem.h"
#include "../mem.h"

typedef struct {
	uint8_t* mem;
	size_t   size;
} MemDisk;

static void FreeDisk(Disk* this) {
	Free(this->data);
}

static Error ReadDisk(Disk* this, size_t where, size_t size, uint8_t* dest) {
	MemDisk* memDisk = (MemDisk*) this->data;

	if (size > memDisk->size - where) {
		return N_ERROR_OUT_OF_BOUNDS_DISK;
	}

	memcpy(dest, &memDisk->mem[where], size);
	return N_ERROR_SUCCESS;
}

static Error WriteDisk(Disk* this, size_t where, size_t size, uint8_t* data) {
	MemDisk* memDisk = (MemDisk*) this->data;

	if (size > memDisk->size - where) {
		return N_ERROR_OUT_OF_BOUNDS_DISK;
	}

	memcpy(&memDisk->mem[where], data, size);
	return N_ERROR_SUCCESS;
}

Disk NewMemDisk(const char* name, uint8_t* mem, size_t size, bool readOnly) {
	Disk ret;
	ret.active   = true;
	ret.block    = false;
	ret.name     = name;
	ret.readOnly = readOnly;
	ret.size     = size;
	ret.free     = &FreeDisk;
	ret.read     = &ReadDisk;
	ret.write    = &WriteDisk;

	MemDisk* memDisk = SafeAlloc(sizeof(MemDisk));
	memDisk->mem     = mem;
	memDisk->size    = size;
	ret.data         = memDisk;
	return ret;
}
