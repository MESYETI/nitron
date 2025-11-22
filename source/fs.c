#include "fs.h"

FileSystem* fileSystems[DISK_AMOUNT];

void FS_Init(void) {
	for (size_t i = 0; i < FS_AMOUNT; ++ i) {
		fileSystems[i] = NULL;
	}
}

FileSystem** FS_NextFree(void) {
	for (size_t i = 0; i < FS_AMOUNT; ++ i) {
		if (!fileSystems[i]) {
			return &fileSystems[i];
		}
	}

	return NULL;
}

FileSystem** FS_Add(FileSystem* fs) {
	FileSystem** next = FS_NextFree();

	if (next == NULL) {
		return NULL;
	}

	*next = fs;
	return next;
}
