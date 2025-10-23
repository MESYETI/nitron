#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "asm.h"
#include "mem.h"
#include "util.h"

typedef struct {
	const char* name;
	uint8_t     opc;
} InstDef;

typedef struct {
	const char* name;
	uint8_t     size;
	uint32_t    value;
} Value;

typedef struct {
	const char* name;
	uint32_t*   ptr;
} IncompValue;

uint8_t* Assemble(const char* code, size_t* size, VM* vm) {
	char     token[256];
	uint8_t* ret    = SafeMalloc(256);
	size_t   retLen = 0;
	size_t   retCap = 256;

	Value* values   = NULL;
	size_t valueLen = 0;

	IncompValue* incomplete    = NULL;
	size_t       incompleteLen = 0;

	bool     data    = false; // true if creating data, false if creating code
	uint8_t* dataIdx = vm->area;

	#define EXTEND() \
		if (retLen >= retCap) { \
			retCap += 256; \
			ret     = SafeRealloc(ret, retCap); \
		}

	#define EXTEND_BY(N) \
		retLen += N; \
		EXTEND();

	while (code[0] != 0) {
		// special tokens
		switch (code[0]) {
			case '#': {
				if (data) {
					fprintf(stderr, "# can only be used in code mode\n");
					exit(1);
				}

				++ retLen;
				EXTEND();
				ret[retLen - 1] = 0x80; // NOPi, push
				++ code;
				continue;
			}
			case '@': {
				++ code;

				size_t len = strcspn(code, " \t\n");
				if (len > 255) {
					fprintf(stderr, "Label too long\n");
					return NULL;
				}

				strncpy(token, code, len);
				token[len] = 0;
				code += len;

				values = SafeRealloc(values, valueLen + 1);

				printf("Adding label '%s'\n", token);

				if (data) {
					values[valueLen] = (Value) {NewString(token), 4, (uint32_t) dataIdx};
				}
				else {
					values[valueLen] = (Value) {NewString(token), 4, (uint32_t) retLen};
				}
				++ valueLen;
				continue;
			}
			default: break;
		}

		size_t len = strcspn(code, " \t\n");

		if (len == 0) {
			++ code;
			continue;
		}
		if (len > 255) {
			fprintf(stderr, "Token in file too big\n");
			return NULL;
		}

		strncpy(token, code, len);
		token[len] = 0;
		code += len;

		if (strspn(token, "0123456789abcdef") == len) {
			// this is an integer
			switch (len) {
				case 2: { // byte
					if (!data) EXTEND_BY(1);

					uint8_t* write = data? dataIdx : &ret[retLen - 1];
					*write         = (uint8_t) strtol(token, NULL, 16);

					if (data) dataIdx += 1;
					break;
				}
				case 4: { // short
					if (!data) EXTEND_BY(2);

					uint8_t* write       = data? dataIdx : &ret[retLen - 2];
					*((uint16_t*) write) = (uint16_t) strtol(token, NULL, 16);

					if (data) dataIdx += 2;
					break;
				}
				case 8: { // word
					if (!data) EXTEND_BY(4);

					uint8_t* write       = data? dataIdx : &ret[retLen - 4];
					*((uint32_t*) write) = (uint32_t) strtol(token, NULL, 16);

					if (data) dataIdx += 4;
					break;
				}
				default: {
					fprintf(stderr, "Invalid integer length: %d nibbles\n", (int) len);
					exit(1);
				}
			}
		}
		else {
			// probably an instruction
			uint8_t opcode;
			bool    isInst = false;
			InstDef insts[] = {
				{"NOP",   0x00}, {"NOPi",   0x80},
				{"ADD",   0x01}, {"ADDi",   0x81},
				{"SUB",   0x02}, {"SUBi",   0x82},
				{"MUL",   0x03}, {"MULi",   0x83},
				{"DIV",   0x04}, {"DIVi",   0x84},
				{"MOD",   0x05}, {"MODi",   0x85},
				{"R2D",   0x06}, {"R2Di",   0x86},
				{"D2R",   0x07}, {"D2Ri",   0x87},
				{"DUP",   0x08}, {"DUPi",   0x88},
				{"OVER",  0x09}, {"OVERi",  0x89},
				{"DROP",  0x0A}, {"DROPi",  0x8A},
				{"ROT",   0x0B}, {"ROTi",   0x8B},
				{"AREA",  0x0C}, {"AREAi",  0x8C},
				{"READ",  0x0D}, {"READi",  0x8D},
				{"WRITE", 0x0E}, {"WRITEi", 0x8E},
				{"JUMP",  0x0F}, {"JUMPi",  0x8F},
				{"JNZ",   0x10}, {"JNZi",   0x90},
				{"HALT",  0x11}, {"HALTi",  0x91},
				{"ECALL", 0x12}, {"ECALLi", 0x92},
			};

			for (size_t i = 0; i < sizeof(insts) / sizeof(InstDef); ++ i) {
				if (strcmp(insts[i].name, token) == 0) {
					opcode = insts[i].opc;
					isInst = true;
				}
			}

			if (isInst) {
				++ retLen;
				EXTEND();
				ret[retLen - 1] = opcode;
				continue;
			}

			bool isValue = false;
			for (size_t i = 0; i < valueLen; ++ i) {
				if (strcmp(values[i].name, token) == 0) {
					isValue = true;

					switch (values[i].size) {
						case 1: {
							++ retLen;
							EXTEND();
							ret[retLen - 1] = (uint8_t) values[i].value;
							break;
						}
						case 2: {
							retLen += 2;
							EXTEND();
							*((uint16_t*) &ret[retLen - 2]) = (uint16_t) values[i].value;
							break;
						}
						case 4: {
							retLen += 4;
							EXTEND();
							*((uint32_t*) &ret[retLen - 4]) = values[i].value;
							break;
						}
						default: assert(0);
					}
				}
			}

			if (isValue) continue;

			if (data) {
				fprintf(stderr, "Unknown identifier '%s'\n", token);
				exit(1);
			}
			else {
				retLen += 4;
				EXTEND();

				incomplete = SafeRealloc(incomplete, incompleteLen + 1);

				incomplete[incompleteLen ++] = (IncompValue) {
					NewString(token), (uint32_t*) &ret[retLen - 4]
				};
			}
		}
	}

	for (size_t i = 0; i < incompleteLen; ++ i) {
		bool found = false;

		for (size_t j = 0; j < valueLen; ++ j) {
			if (strcmp(values[j].name, incomplete[i].name) == 0) {
				found = true;

				if (values[j].size != 4) {
					fprintf(
						stderr, "Incomplete value '%s' must be 4 bytes",
						values[j].name
					);
					exit(1);
				}

				*incomplete[i].ptr = values[j].value;
				break;
			}
		}

		if (!found) {
			fprintf(stderr, "Couldn't find value '%s'\n", incomplete[i].name);
			exit(1);
		}
	}

	*size = retLen;
	return ret;
}
