#ifndef N_FS_ARK_H
#define N_FS_ARK_H

#include "../fs.h"

FileSystem* Ark_CreateFileSystem(Disk* disk, Error* error, const char* name);

#endif
