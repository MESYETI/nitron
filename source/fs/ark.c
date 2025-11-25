#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "ark.h"
#include "../fs.h"
#include "../mem.h"
#include "../disk.h"

typedef struct ArkEntry ArkEntry;

struct ArkEntry {
	bool   folder;
	size_t size;
	char*  name;
	size_t contentsOffset;

	ArkEntry* folderContents;
	size_t    folderSize;
};

typedef struct {
	Disk*    disk;
	size_t   diskPtr;
	uint16_t ver;
	char*    strings;
	size_t   stringsLen;
	ArkEntry root;
} Reader;

static uint8_t Read8(Reader* reader) {
	uint8_t ret;
	assert(reader->disk->read(reader->disk, reader->diskPtr, 1, &ret) == 0);
	++ reader->diskPtr;
	return ret;
}

static uint16_t Read16(Reader* reader) {
	uint16_t ret;
	uint8_t  bytes[4];

	assert(reader->disk->read(reader->disk, reader->diskPtr, 2, bytes) == 0);
	reader->diskPtr += 2;

	ret  = bytes[0];
	ret |= ((uint16_t) bytes[1]) << 8;

	return ret;
}

static uint32_t Read32(Reader* reader) {
	uint32_t ret;
	uint8_t  bytes[4];

	assert(reader->disk->read(reader->disk, reader->diskPtr, 4, bytes) == 0);
	reader->diskPtr += 4;

	ret  = bytes[0];
	ret |= ((uint32_t) bytes[1]) << 8;
	ret |= ((uint32_t) bytes[2]) << 16;
	ret |= ((uint32_t) bytes[3]) << 24;

	return ret;
}

static void InitReader(Reader* reader, Disk* disk) {
	reader->disk    = disk;
	reader->diskPtr = 0;
}

static void FreeDir(ArkEntry* dir) {
	if (dir->folder) {
		for (size_t i = 0; i < dir->folderSize; ++ i) {
			FreeDir(&dir->folderContents[i]);
		}

		Free(dir->folderContents);
	}
}

static void FreeReader(Reader* reader) {
	FreeDir(&reader->root);
	Free(reader->strings);
}

static ArkEntry ReadEntry(Reader* reader, Error* success) {
	ArkEntry entry;

	entry.folder = Read8(reader) != 0;
	entry.size   = Read32(reader);

	uint32_t offset = Read32(reader);
	if (offset > reader->stringsLen) {
		*success = N_ERROR_INTERNAL;
		fprintf(stderr, "Out of bounds string table offset in archive\n");
		return entry;
	}

	entry.name           = &reader->strings[offset];
	entry.contentsOffset = reader->diskPtr;

	if (entry.folder) {
		size_t length        = Read32(reader);
		entry.folderContents = SafeMalloc(length * sizeof(ArkEntry));
		entry.folderSize     = length;

		for (size_t i = 0; i < length; ++ i) {
			Error error2;
			entry.folderContents[i] = ReadEntry(reader, &error2);

			if (error2 != N_ERROR_SUCCESS) {
				*success = error2;
				return entry;
			}
		}
	}
	else {
		reader->diskPtr += entry.size;
	}

	*success = N_ERROR_SUCCESS;
	return entry;
}

static Error Read(Reader* reader) {
	reader->ver = Read16(reader);
	Read8(reader); // unused

	reader->stringsLen = Read32(reader);
	Read32(reader); // random number

	if (reader->stringsLen == 0xFFFFFFFF) {
		fprintf(stderr, "String table too big\n");
		return N_ERROR_INTERNAL;
	}

	// read strings
	reader->strings = SafeMalloc(reader->stringsLen + 1);

	reader->strings[reader->stringsLen] = 0;

	Error res = reader->disk->read(
		reader->disk, reader->diskPtr, reader->stringsLen, (uint8_t*) reader->strings
	);
	reader->diskPtr += reader->stringsLen;
	if (res != N_ERROR_SUCCESS) {
		return res;
	}

	// read file entries
	Error error;
	reader->root      = ReadEntry(reader, &error);
	reader->root.name = "";

	return error;
}

static Error ReadEntryContents(Reader* reader, ArkEntry* entry, uint8_t** dest) {
	*dest = SafeMalloc(entry->size);
	return reader->disk->read(reader->disk, entry->contentsOffset, entry->size, *dest);
}

typedef struct {
	FileSystem parent;
	Reader     reader;
} ArkFS;

static void FreeFS(FileSystem* p_fs) {
	ArkFS* fs = (ArkFS*) p_fs;
	FreeReader(&fs->reader);
}

static ArkEntry* GetEntryInDir(ArkEntry* folder, const char* name, size_t len) {
	for (size_t i = 0; i < folder->folderSize; ++ i) {
		if (
			(strlen(folder->folderContents[i].name) == len) &&
			(strncmp(name, folder->folderContents[i].name, len) == 0)
		) {
			return &folder->folderContents[i];
		}
	}

	return NULL;
}

static ArkEntry* GetEntry(Reader* reader, const char* path) {
	if (path[0] == 0) {
		return &reader->root;
	}

	ArkEntry* dir = &reader->root;

	const char* pathIt = path;
	while (true) {
		char* next = strchr(pathIt, '/');

		if (next) {
			ArkEntry* entry = GetEntryInDir(dir, pathIt, next - pathIt);

			if (entry && entry->folder) {
				dir = entry;
			}
			else {
				return NULL;
			}
		}
		else {
			return GetEntryInDir(dir, pathIt, strlen(pathIt));
		}

		pathIt = next + 1;
	}
}

static bool FileExists(FileSystem* p_fs, const char* path) {
	ArkFS* fs = (ArkFS*) p_fs;

	return GetEntry(&fs->reader, path)? true : false;
}

static Error ReadFile(FileSystem* p_fs, const char* path, size_t* size, uint8_t** dest) {
	ArkFS* fs = (ArkFS*) p_fs;
	ArkEntry* entry = GetEntry(&fs->reader, path);

	if (!entry) {
		return N_ERROR_FILE_NOT_FOUND;
	}
	else if (entry->folder) {
		return N_ERROR_NOT_A_FILE;
	}

	*size = entry->size;
	return ReadEntryContents(&fs->reader, entry, dest);
}

FileSystem* Ark_CreateFileSystem(Disk* disk, Error* error, const char* name) {
	ArkFS* ret = SafeMalloc(sizeof(ArkFS));
	// expect caller to write to name

	InitReader(&ret->reader, disk);

	ret->parent.free       = &FreeFS;
	ret->parent.fileExists = &FileExists;
	ret->parent.readFile   = &ReadFile;
	ret->parent.writeFile  = NULL; // read only
	ret->parent.name       = name;
	ret->parent.niceName   = "Read-only ARK";

	Error res = Read(&ret->reader);
	*error    = res;
	if (res != N_ERROR_SUCCESS) {
		Free(ret);
		return NULL;
	}

	return (FileSystem*) ret;
}
