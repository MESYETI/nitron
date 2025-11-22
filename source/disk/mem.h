#ifndef N_DISK_MEM_H
#define N_DISK_MEM_H

#include "../disk.h"

Disk NewMemDisk(const char* name, uint8_t* mem, size_t size, bool readOnly);

#endif
