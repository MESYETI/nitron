#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "fs.h"
#include "asm.h"
#include "mem.h"
#include "util.h"
#include "error.h"

void Assembler_InitBasic(Assembler* this) {
	this->values        = NULL;
	this->valuesLen     = 0;
	this->macros        = NULL;
	this->macrosLen     = 0;
	this->incomplete    = NULL;
	this->incompleteLen = 0;
	this->valueSize     = 0;
	this->included      = NULL;
	this->includedLen   = 0;
	this->data          = false;
	this->bin           = NULL;
	this->binPtr        = NULL;
	this->binLen        = 0;
	this->binCap        = 0;
	this->area          = NULL;
	this->areaSize      = 0;
}

void Assembler_Init(Assembler* this, char* code, VM* vm) {
	Assembler_InitBasic(this);
	this->code     = code;
	this->area     = vm->areaPtr;
	this->areaSize = vm->areaSize;
	this->dataPtr  = this->area;
	this->data     = false;
}

void Assembler_Free(Assembler* this) {
	if (this->values != NULL) {
		Free(this->values);
	}

	for (size_t i = 0; i < this->macrosLen; ++ i) {
		Free(this->macros[i].contents);
	}
	if (this->macros != NULL) {
		Free(this->macros);
	}
}

static bool CharMatch(char ch, const char* match) {
	while (*match) {
		if (ch == *match) return true;
		++ match;
	}

	return false;
}

static void SkipEmptyTokens(Assembler* this) {
	this->token[0] = 0;

	// skip empty tokens
	while (true) {
		if (*this->code == 0) {
			this->token[0] = 0;
			return;
		}

		if (CharMatch(this->code[0], " \t\n")) {
			++ this->code;
			continue;
		}
		else {
			break;
		}
	}
}

static void NextToken(Assembler* this) {
	SkipEmptyTokens(this);

	size_t len = strcspn(this->code, " \t\n}");
	strncpy(this->token, this->code, len);
	this->token[len]  = 0;
	this->code       += len;
}

static void ReadStringToken(Assembler* this) {
	SkipEmptyTokens(this);

	if (this->code[0] != '"') {
		fprintf(stderr, "Expectedd \", got '%c'", this->code[0]);
		exit(1); // TODO
	}
	++ this->code;

	size_t len = strcspn(this->code, "\"");
	strncpy(this->token, this->code, len);
	this->token[len]  = 0;
	this->code       += len + 1;
}

static bool AssertBinSpace(Assembler* this, size_t size) {
	if (this->data) {
		size_t bufSize = (this->dataPtr - this->area) + size;

		if (bufSize > this->areaSize) {
			fprintf(stderr, "%s\n", this->code);
			fprintf(
				stderr, "Not enough room to allocate %u bytes in %u byte buffer\n",
				bufSize, this->areaSize
			);
			return false;
		}
	}
	else {
		this->binLen += size;

		if ((this->binPtr - this->bin) + size > this->binCap) {
			size_t offset  = this->binPtr - this->bin;
			this->bin      = SafeRealloc(this->bin, this->binCap + size + 256);
			this->binCap  += 256 + size;
			this->binPtr   = this->bin + offset;
			puts("RESIZED");
		}
	}

	return true;
}

static void Debug(Assembler* this, const char* src, const char* name) {
	printf("=== %s %s ===\n", src, name);
	printf("START %p\n", this->values);
	printf("BLOCK %p\n", ((uint8_t*) this->values) - sizeof(MemBlock));
	printf("END   %p\n", this->values + this->valuesLen);
	DumpAllocator();

	for (size_t i = 0; i < this->valuesLen; ++ i) {
		if (strcmp(this->values[i].name, "N_ResetAsmBinLen") == 0) {
			printf("VALUES CONTAINS N_ResetAsmBinLen at %p\n", this->values[i].name);
		}
		if (strcmp(this->values[i].name, "NITRON") == 0) {
			printf("VALUES CONTAINS NITRON at %p\n", this->values[i].name);
		}
	}

	puts("===== END =====");
}

bool Assembler_Assemble(Assembler* this, size_t* size, bool completion) {
	/*if (this->incomplete) {
		Free(this->incomplete);
		this->incompleteLen = 0;
	}*/ // what is this doing here???

	static const InstDef insts[] = {
		{"NOP",     0x00}, {"NOPi",     0x80},
		{"ADD",     0x01}, {"ADDi",     0x81},
		{"SUB",     0x02}, {"SUBi",     0x82},
		{"MUL",     0x03}, {"MULi",     0x83},
		{"DIV",     0x04}, {"DIVi",     0x84},
		{"MOD",     0x05}, {"MODi",     0x85},
		{"R2D",     0x06}, {"R2Di",     0x86},
		{"D2R",     0x07}, {"D2Ri",     0x87},
		{"DUP",     0x08}, {"DUPi",     0x88},
		{"OVER",    0x09}, {"OVERi",    0x89},
		{"DROP",    0x0A}, {"DROPi",    0x8A},
		{"ROT",     0x0B}, {"ROTi",     0x8B},
		{"AREA",    0x0C}, {"AREAi",    0x8C},
		{"READ",    0x0D}, {"READi",    0x8D},
		{"WRITE",   0x0E}, {"WRITEi",   0x8E},
		{"JUMP",    0x0F}, {"JUMPi",    0x8F},
		{"JNZ",     0x10}, {"JNZi",     0x90},
		{"HALT",    0x11}, {"HALTi",    0x91},
		{"ECALL",   0x12}, {"ECALLi",   0x92},
		{"REG",     0x13}, {"REGi",     0x93},
		{"WREG",    0x14}, {"WREGi",    0x94},
		{"READ8",   0x15}, {"READ8i",   0x95},
		{"WRITE8",  0x16}, {"WRITE8i",  0x96},
		{"READ16",  0x17}, {"READ16i",  0x97},
		{"WRITE16", 0x18}, {"WRITE16i", 0x98},
		{"JZ",      0x19}, {"JZi",      0x99},
		{"DIVMOD",  0x20}, {"DIVMODi",  0xA0},
		{"EQU",     0x21}, {"EQUi",     0xA1},
		{"LESS",    0x22}, {"LESSi",    0xA2},
		{"GREATER", 0x23}, {"GREATERi", 0xA3},
		{"LE",      0x24}, {"LEi",      0xA4},
		{"GE",      0x25}, {"GEi",      0xA5},
		{"NEG",     0x26}, {"NEGi",     0xA6},
		{"AND",     0x27}, {"ANDi",     0xA7},
		{"XOR",     0x28}, {"XORi",     0xA8},
		{"OR",      0x29}, {"ORi",      0xA9},
		{"NOT",     0x2A}, {"NOTi",     0xAA},
		{"SWAP",    0x2B}, {"SWAPi",    0xAB},
		{"CALL",    0x2C}, {"CALLi",    0xAC},
		{"RET",     0x2D}, {"RETi",     0xAD}
	};

	#define ASSERT_BIN_SPACE(SIZE) do { \
		if (!AssertBinSpace(this, (SIZE))) { \
			fprintf(stderr, "Not enough room for binary\n"); \
			return false; \
		} \
	} while(0)

	while (this->code[0] != 0) {
		while (CharMatch(this->code[0], " \t\n")) {
			++ this->code;
		}

		// special tokens
		if (this->code[0] == '#') {
			if (this->data) {
				fprintf(stderr, "'#' can only be used in code mode\n");
				return false;
			}

			ASSERT_BIN_SPACE(1);
			*this->binPtr = 0x80; // NOPi = Push
			++ this->binPtr;
			++ this->code;
			continue;
		}
		else if (this->code[0] == '"') {
			++ this->code;
			size_t len = strcspn(this->code, "\"");

			strncpy(this->token, this->code - 1, len + 1);
			this->token[len + 1] = 0;
			this->code += len + 1;
		}
		else if (this->code[0] == '{') {
			ASSERT_BIN_SPACE(5);
			*this->binPtr = 0x80; // NOPi = Push

			*((uint32_t*) (this->binPtr + 1)) = (uint32_t) this->dataPtr;
			this->binPtr += 5;
			++ this->code;

			this->data = true;
			continue;
		}
		else if (this->code[0] == '}') {
			this->data = false;
			++ this->code;
			continue;
		}
		else if (this->code[0] == '(') {
			while (this->code[0] && (this->code[0] != ')')) ++ this->code;
			++ this->code;
			continue;
		}
		else {
			NextToken(this);
		}

		if (strcmp(this->token, "") == 0) {
			continue;
		}

		size_t len = strlen(this->token);

		// token types
		switch (this->token[0]) {
			case ';': {
				while ((this->code[0] != '\n') && (this->code[0] != 0)) {
					++ this->code;
				}

				continue;
			}
			case ':': {
				Macro macro;
				strcpy(macro.name, &this->token[1]);

				size_t contentsLen = strcspn(this->code, ":");
				macro.contents     = SafeMalloc(contentsLen + 1);

				strncpy(macro.contents, this->code, contentsLen);

				macro.contents[contentsLen] = 0;
				this->code += contentsLen + 1;

				this->macros = SafeRealloc(
					this->macros, (this->macrosLen + 1) * sizeof(Macro)
				);
				this->macros[this->macrosLen] = macro;
				++ this->macrosLen;
				continue;
			}
			case '@': {
				this->values = SafeRealloc(
					this->values, (this->valuesLen + 1) * sizeof(Value)
				);

				strcpy(this->values[this->valuesLen].name, &this->token[1]);
				this->values[this->valuesLen].size  = 4;

				if (this->data) {
					this->values[this->valuesLen].value = (uint32_t) this->dataPtr;
				}
				else {
					this->values[this->valuesLen].value = (uint32_t) this->binPtr;
				}

				++ this->valuesLen;
				Debug(this, "LABEL", &this->token[1]);
				continue;
			}
			case '"': {
				ASSERT_BIN_SPACE(strlen(this->token) - 1);

				uint8_t** dest = this->data? &this->dataPtr : &this->binPtr;
				memcpy(*dest, &this->token[1], strlen(this->token) - 1);
				*dest += strlen(this->token) - 1;
				continue;
			}
			case '$': {
				if (strlen(this->token) != 2) {
					fprintf(stderr, "Invalid integer size marker: %s\n", this->token);
					return false;
				}

				switch (this->token[1]) {
					case 'b': this->valueSize = 2; break;
					case 's': this->valueSize = 4; break;
					case 'w': this->valueSize = 8; break;
					default: {
						fprintf(stderr, "Invalid integer size marker: %s\n", this->token);
						return false;
					}
				}
				continue;
			}
			case '%': {
				if (strcmp(this->token, "%include") == 0) {
					ReadStringToken(this);

					bool alreadyIncluded = false;
					for (size_t i = 0; i < this->includedLen; ++ i) {
						if (strcmp(this->token, this->included[i]) == 0) {
							alreadyIncluded = true;
							break;
						}
					}

					if (alreadyIncluded) continue;

					this->included = SafeRealloc(
						this->included, (this->includedLen + 1) * ASM_TOKEN_SIZE
					);
					strcpy(this->included[this->includedLen], this->token);
					++ this->includedLen;

					char*    oldCode   = this->code;
					uint8_t* oldBinPtr = this->binPtr;
					size_t   size;

					Error error = FS_ReadFile(
						this->token, &size, (uint8_t**) &this->code
					);

					if (error != N_ERROR_SUCCESS) {
						fprintf(
							stderr, "Failed to read '%s': %s\n", this->token,
							ErrorToString(error)
						);
						return false;
					}

					this->code       = SafeRealloc(this->code, size + 1);
					this->code[size] = 0;

					char* code = this->code;

					if (!Assembler_Assemble(this, NULL, false)) {
						this->binPtr = oldBinPtr;
						Free(code);
						return false;
					}

					Free(code);
					this->code = oldCode;
					continue;
				}
				else if (strcmp(this->token, "%code") == 0) {
					this->data = false;
				}
				else if (strcmp(this->token, "%data") == 0) {
					this->data = true;
				}
				else {
					fprintf(stderr, "Invalid directive: %s\n", this->token);
					return false;
				}

				continue;
			}
			default: break;
		}

		if (strspn(this->token, "0123456789abcdef") == len) {
			uint8_t** dest = this->data? &this->dataPtr : &this->binPtr;
			size_t    size = this->valueSize == 0? len : this->valueSize;

			if (this->valueSize != 0) {
				this->valueSize = 0;
			}

			switch (size) {
				case 2: { // byte
					ASSERT_BIN_SPACE(1);

					*((uint8_t*) *dest) = (uint8_t) strtol(this->token, NULL, 16);
					++ *dest;
					break;
				}
				case 4: { // short
					ASSERT_BIN_SPACE(2);

					*((uint16_t*) *dest) = (uint16_t) strtol(this->token, NULL, 16);
					*dest += 2;
					break;
				}
				case 8: { // word
					ASSERT_BIN_SPACE(4);

					*((uint32_t*) *dest) = (uint32_t) strtol(this->token, NULL, 16);
					*dest += 4;
					break;
				}
				default: {
					fprintf(stderr, "Invalid integer length: %d nibbles\n", (int) len);
					fprintf(stderr, "    '%s'\n", this->token);
					return false;
				}
			}
		}
		else if (strcmp(this->token, "define") == 0) {
			NextToken(this);

			Value value;
			strcpy(value.name, this->token);

			NextToken(this);

			size_t size = this->valueSize == 0? strlen(this->token) : this->valueSize;

			if (this->valueSize != 0) {
				this->valueSize = 0;
			}

			switch (size) {
				case 2: value.size = 1; break;
				case 4: value.size = 2; break;
				case 8: value.size = 4; break;
				default: {
					fprintf(stderr, "Invalid integer length: %d nibbles\n", (int) len);
					fprintf(stderr, "    '%s'\n", this->token);
					return false;
				}
			}

			value.value  = strtol(this->token, NULL, 16);
			this->values = SafeRealloc(
				this->values, (this->valuesLen + 1) * sizeof(Value)
			);

			if (strcmp(value.name, "N_ResetAsmBinLen") == 0) {
				puts("HERE!");
			}

			printf("Defining value %s = %d\n", value.name, value.value);

			this->values[this->valuesLen] = value;
			++ this->valuesLen;

			Debug(this, "DEFINE", value.name);
		}
		else if (strcmp(this->token, "reserve") == 0) {
			NextToken(this);
			size_t size = strtol(this->token, NULL, 10);

			ASSERT_BIN_SPACE(size);

			if (this->data) this->dataPtr += size;
			else            this->binPtr  += size;
		}
		else {
			bool    isInst = false;
			uint8_t opcode;

			for (size_t i = 0; i < sizeof(insts) / sizeof(InstDef); ++ i) {
				if (strcmp(insts[i].name, this->token) == 0) {
					opcode = insts[i].opc;
					isInst = true;
				}
			}

			if (isInst) {
				ASSERT_BIN_SPACE(1);
				*this->binPtr = opcode;
				++ this->binPtr;
				continue;
			}

			bool isValue = false;
			for (size_t i = 0; i < this->valuesLen; ++ i) {
				uint8_t** dest = this->data? &this->dataPtr : &this->binPtr;

				if (strcmp(this->values[i].name, this->token) == 0) {
					isValue = true;

					switch (this->values[i].size) {
						case 1: { // byte
							ASSERT_BIN_SPACE(1);

							*((uint8_t*) *dest) = (uint8_t) this->values[i].value;
							++ *dest;
							break;
						}
						case 2: { // short
							ASSERT_BIN_SPACE(2);

							*((uint16_t*) *dest) = (uint16_t) this->values[i].value;
							*dest += 2;
							break;
						}
						case 4: { // word
							ASSERT_BIN_SPACE(4);

							*((uint32_t*) *dest) = this->values[i].value;
							*dest += 4;
							break;
						}
						default: assert(0);
					}
				}
			}

			if (isValue) continue;

			bool isMacro = false;
			for (size_t i = 0; i < this->macrosLen; ++ i) {
				if (strcmp(this->macros[i].name, this->token) == 0) {
					isMacro = true;

					char* oldCode = this->code;
					this->code    = this->macros[i].contents;

					Assembler_Assemble(this, NULL, false);

					this->code = oldCode;
				}
			}

			if (isMacro) continue;

			if (this->data) {
				fprintf(stderr, "Unknown identifier '%s'\n", this->token);
				return false;
			}
			else {
				ASSERT_BIN_SPACE(4);

				this->incomplete = SafeRealloc(
					this->incomplete, (this->incompleteLen + 1) * sizeof(IncompValue)
				);

				strcpy(this->incomplete[this->incompleteLen].name, this->token);
				this->incomplete[this->incompleteLen].ptr = (uint32_t*) this->binPtr;

				this->binPtr += 4;

				++ this->incompleteLen;
			}
		}
	}

	if (completion) {
		for (size_t i = 0; i < this->incompleteLen; ++ i) {
			bool found = false;

			for (size_t j = 0; j < this->valuesLen; ++ j) {
				printf("Checking '%s' for '%s'\n", this->values[j].name, this->incomplete[i].name);
				if (strcmp(this->values[j].name, this->incomplete[i].name) == 0) {
					found = true;

					if (this->values[j].size != 4) {
						fprintf(
							stderr, "Incomplete value '%s' must be 4 bytes",
							this->values[j].name
						);
						return false;
					}

					*this->incomplete[i].ptr = this->values[j].value;
					break;
				}
			}

			if (!found) {
				fprintf(stderr, "Couldn't find value '%s'\n", this->incomplete[i].name);
				DumpAllocator();
				return false;
			}
		}
	}

	if (size) {
		*size = this->binLen;
	}

	return true;
}
