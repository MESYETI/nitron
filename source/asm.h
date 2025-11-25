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
	uint8_t  size;
	uint32_t value;
} Value;

typedef struct {
	char      name[ASM_TOKEN_SIZE];
	uint32_t* ptr;
} IncompValue;

typedef struct {
	char  name[ASM_TOKEN_SIZE];
	char* contents;
} Macro;

typedef struct {
	char*        code;
	VM*          vm;
	Value*       values;
	size_t       valuesLen;
	Macro*       macros;
	size_t       macrosLen;
	uint8_t*     bin;
	size_t       binLen;
	IncompValue* incomplete;
	size_t       incompleteLen;
	size_t       valueSize;
	char         (*included)[ASM_TOKEN_SIZE];
	size_t       includedLen;

	// internal state
	char     token[ASM_TOKEN_SIZE];
	uint8_t* binPtr;
	bool     data;
	uint8_t* dataPtr;
} Assembler;

void Assembler_InitBasic(Assembler* this);
void Assembler_Init(Assembler* this, char* code, VM* vm);
void Assembler_Free(Assembler* this);
bool Assembler_Assemble(Assembler* this, bool init, size_t* size, bool completion);

#endif
