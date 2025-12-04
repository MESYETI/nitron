#ifndef N_ASM_H
#define N_ASM_H

#include "vm.h"
#include "common.h"

#define ASM_TOKEN_SIZE 128

typedef struct {
	const char* name;
	uint8_t     opc;
} InstDef;

typedef struct {
	char     name[ASM_TOKEN_SIZE];
	bool     code; // code label?
	uint8_t  size;
	uint32_t value;
} Value;

typedef struct {
	char   name[ASM_TOKEN_SIZE];
	bool   data;
	size_t offset;
} IncompValue;

typedef struct {
	char  name[ASM_TOKEN_SIZE];
	char* contents;
} Macro;

typedef struct {
	char*        code;
	Value*       values;
	size_t       valuesLen;
	Macro*       macros;
	size_t       macrosLen;
	uint8_t*     bin;
	size_t       binLen;
	size_t       binCap;
	IncompValue* incomplete;
	size_t       incompleteLen;
	size_t       valueSize;
	char         (*included)[ASM_TOKEN_SIZE];
	size_t       includedLen;
	uint8_t*     area;
	size_t       areaSize;

	// internal state
	char     token[ASM_TOKEN_SIZE];
	uint8_t* binPtr;
	bool     data;
	uint8_t* dataPtr;
} Assembler;

void Assembler_InitBasic(Assembler* this);
void Assembler_Init(Assembler* this, char* code, VM* vm);
void Assembler_Free(Assembler* this);
bool Assembler_AssertBinSpace(Assembler* this, size_t size);
bool Assembler_Assemble(Assembler* this, size_t* size, bool completion);

#endif
