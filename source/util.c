#include <stdio.h>
#include <string.h>
#include "mem.h"
#include "util.h"

void* ReadFile(const char* path, size_t* size) {
	FILE* file = fopen(path, "rb");

	if (file == NULL) {
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	void* res = SafeAlloc(*size + 1);
	fread(res, 1, *size, file);
	fclose(file);

	((uint8_t*) res)[*size] = 0;
	return res;
}

char* NewString(const char* src) {
	char* ret = SafeAlloc(strlen(src) + 1);
	strcpy(ret, src);
	return ret;
}
