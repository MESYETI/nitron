#ifndef N_FS_HOST_H
#define N_FS_HOST_H

#include "../fs.h"

FileSystem* Host_CreateFileSystem(Error* error, const char* path, const char* name);

#endif
