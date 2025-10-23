#ifndef N_VM_H
#define N_VM_H

#include "common.h"

enum {
	VM_INST_NOP   = 0x00,
	VM_INST_ADD   = 0x01,
	VM_INST_SUB   = 0x02,
	VM_INST_MUL   = 0x03,
	VM_INST_DIV   = 0x04,
	VM_INST_MOD   = 0x05,
	VM_INST_R2D   = 0x06,
	VM_INST_D2R   = 0x07,
	VM_INST_DUP   = 0x08,
	VM_INST_OVER  = 0x09,
	VM_INST_DROP  = 0x0A,
	VM_INST_ROT   = 0x0B,
	VM_INST_AREA  = 0x0C,
	VM_INST_READ  = 0x0D,
	VM_INST_WRITE = 0x0E,
	VM_INST_JUMP  = 0x0F,
	VM_INST_JNZ   = 0x10,
	VM_INST_HALT  = 0x11,
	VM_INST_ECALL = 0x12
};

typedef struct VM VM;

struct VM{
	uint8_t*  area;
	uint8_t*  ip;
	uint8_t*  code;
	size_t    codeSize;
	uint32_t* dStack;
	uint32_t* rStack;
	uint32_t* dsp;
	uint32_t* rsp;
	uint32_t  reg[8];

	void (*insts[128])(VM*);
	void (*calls[256])(VM*);
};

void VM_Init(VM* vm);
void VM_Free(VM* vm);
void VM_Load(VM* vm, uint8_t* program, size_t size);
void VM_Run(VM* vm);

#endif
