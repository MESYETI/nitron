#include <string.h>
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

static FileSystem* GetFS(const char* path) {
	if (path[0] != ':') return NULL;

	const char* name = &path[1];
	size_t      nameLen;

	const char* slash = strchr(path, '/');

	if (slash) {
		nameLen = slash - name;
	}
	else {
		nameLen = strlen(name);
	}

	// find drive
	for (size_t i = 0; i < FS_AMOUNT; ++ i) {
		if (!fileSystems[i]) continue;

		if (
			(strlen(fileSystems[i]->name) == nameLen) &&
			(strncmp(fileSystems[i]->name, name, nameLen) == 0)
		) {
			return fileSystems[i];
		}
	}

	return NULL;
}

Error FS_ReadFile(const char* path, size_t* size, uint8_t** res) {
	FileSystem* fs = GetFS(path);

	if (!fs) {
		return N_ERROR_INVALID_DRIVE;
	}

	const char* fsPath = strchr(path, '/');

	if (fsPath == NULL) {
		fsPath = "";
	}
	else {
		++ fsPath;
	}

	return fs->readFile(fs, fsPath, size, res);
}

Error FS_WriteFile(const char* path, size_t size, uint8_t* data) {
	FileSystem* fs = GetFS(path);

	if (!fs) {
		return N_ERROR_INVALID_DRIVE;
	}

	const char* fsPath = strchr(path, '/');

	if (fsPath == NULL) {
		fsPath = "";
	}
	else {
		++ fsPath;
	}

	if (!fs->writeFile) {
		return N_ERROR_FS_READ_ONLY;
	}

	return fs->writeFile(fs, fsPath, size, data);
}
