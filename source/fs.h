#ifndef N_FS_H
#define N_FS_H

#include "disk.h"
#include "common.h"

typedef struct FileSystem FileSystem;

struct FileSystem {
	bool        readOnly;
	void*       data;
	Disk*       disk;
	const char* name;
	const char* niceName;

	void    (*free)(FileSystem* fs);
	bool    (*fileExists)(FileSystem* fs, const char* path);
	int32_t (*fileSize)(FileSystem* fs, const char* path);
	Error   (*readFile)(FileSystem* fs, const char* path, size_t* size, uint8_t** dest);
};

#define FS_AMOUNT 8
extern FileSystem* fileSystems[FS_AMOUNT];

void         FS_Init(void);
FileSystem** FS_NextFree(void);
FileSystem** FS_Add(FileSystem* fs);
Error        FS_ReadFile(const char* path, size_t* size, uint8_t** res);

#endif
