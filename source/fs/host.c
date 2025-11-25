#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include "host.h"
#include "../mem.h"

typedef struct {
	FileSystem  parent;
	const char* path;
} HostFS;

static void FreeFS(FileSystem* p_fs) {
	(void) p_fs;
}

static bool FileExists(FileSystem* p_fs, const char* path) {
	HostFS* fs = (HostFS*) p_fs;

	if (strlen(fs->path) + strlen(path) > PATH_MAX) {
		return false;
	}

	char fullPath[PATH_MAX + 1];
	strcpy(fullPath, fs->path);
	strcat(fullPath, path);

	struct stat statBuf;
	return stat(fullPath, &statBuf) == 0;
}

static Error ReadFile(FileSystem* p_fs, const char* path, size_t* size, uint8_t** dest) {
	HostFS* fs = (HostFS*) p_fs;

	if (strlen(fs->path) + strlen(path) > PATH_MAX - 1) {
		return false;
	}

	char fullPath[PATH_MAX + 1];
	strcpy(fullPath, fs->path);
	strcat(fullPath, "/");
	strcat(fullPath, path);

	FILE* file = fopen(fullPath, "rb");
	if (!file) {
		return N_ERROR_FILE_NOT_FOUND;
	}

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	*dest = Alloc(*size);
	if (*dest) {
		return N_ERROR_OUT_OF_MEMORY;
	}

	if (fread(*dest, 1, *size, file) != *size) {
		return N_ERROR_INTERNAL;
	}

	return N_ERROR_SUCCESS;
}

static Error WriteFile(FileSystem* p_fs, const char* path, size_t size, uint8_t* data) {
	HostFS* fs = (HostFS*) p_fs;

	if (strlen(fs->path) + strlen(path) > PATH_MAX - 1) {
		return false;
	}

	char fullPath[PATH_MAX + 1];
	strcpy(fullPath, fs->path);
	strcat(fullPath, "/");
	strcat(fullPath, path);

	FILE* file = fopen(fullPath, "wb");
	if (!file) {
		return N_ERROR_FILE_NOT_FOUND;
	}

	if (fwrite(data, 1, size, file) != size) {
		return N_ERROR_INTERNAL;
	}

	return N_ERROR_SUCCESS;
}

FileSystem* Host_CreateFileSystem(Error* error, const char* path, const char* name) {
	HostFS* ret = SafeMalloc(sizeof(HostFS));
	ret->parent.free       = &FreeFS;
	ret->parent.fileExists = &FileExists;
	ret->parent.readFile   = &ReadFile;
	ret->parent.writeFile  = &WriteFile;
	ret->parent.name       = name;
	ret->parent.niceName   = "Host system drive";
	ret->path              = path;

	*error = N_ERROR_SUCCESS;
	return (FileSystem*) ret;
}
