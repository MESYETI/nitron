#include "disk.h"

Disk disks[DISK_AMOUNT];

void Disk_Init(void) {
	for (size_t i = 0; i < DISK_AMOUNT; ++ i) {
		disks[i].active = false;
	}
}

Disk* Disk_NextFree(void) {
	for (size_t i = 0; i < DISK_AMOUNT; ++ i) {
		if (!disks[i].active) {
			return &disks[i];
		}
	}

	return NULL;
}

Disk* Disk_Add(Disk disk) {
	Disk* next = Disk_NextFree();

	if (next == NULL) {
		return NULL;
	}

	*next = disk;
	return next;
}
