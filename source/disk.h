#ifndef N_DISK_H
#define N_DISK_H

#include "error.h"
#include "common.h"

typedef struct Disk Disk;

struct Disk {
	bool        active;
	bool        block; // true = block device, false = character device
	const char* name;
	bool        readOnly;
	size_t      size;
	void*       data;

	// functions
	void  (*free)(Disk* this);
	Error (*read)(Disk* this, size_t where, size_t size, uint8_t* dest);
	Error (*write)(Disk* this, size_t where, size_t size, uint8_t* data);
};

#define DISK_AMOUNT 8

extern Disk disks[DISK_AMOUNT];

void  Disk_Init(void);
Disk* Disk_NextFree(void);
Disk* Disk_Add(Disk disk);

#endif
