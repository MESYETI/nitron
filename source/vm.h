#ifndef N_VM_H
#define N_VM_H

#include "common.h"

enum {
	VM_INST_NOP     = 0x00,
	VM_INST_ADD     = 0x01,
	VM_INST_SUB     = 0x02,
	VM_INST_MUL     = 0x03,
	VM_INST_DIV     = 0x04,
	VM_INST_MOD     = 0x05,
	VM_INST_R2D     = 0x06,
	VM_INST_D2R     = 0x07,
	VM_INST_DUP     = 0x08,
	VM_INST_OVER    = 0x09,
	VM_INST_DROP    = 0x0A,
	VM_INST_ROT     = 0x0B,
	VM_INST_AREA    = 0x0C,
	VM_INST_READ    = 0x0D,
	VM_INST_WRITE   = 0x0E,
	VM_INST_JUMP    = 0x0F,
	VM_INST_JNZ     = 0x10,
	VM_INST_HALT    = 0x11,
	VM_INST_ECALL   = 0x12,
	VM_INST_REG     = 0x13,
	VM_INST_WREG    = 0x14,
	VM_INST_READ8   = 0x15,
	VM_INST_WRITE8  = 0x16,
	VM_INST_READ16  = 0x17,
	VM_INST_WRITE16 = 0x18,
	VM_INST_JZ      = 0x19,
	VM_INST_DIVMOD  = 0x20,
	VM_INST_EQU     = 0x21,
	VM_INST_LESS    = 0x22,
	VM_INST_GREATER = 0x23,
	VM_INST_LE      = 0x24,
	VM_INST_GE      = 0x25,
	VM_INST_NEG     = 0x26,
	VM_INST_AND     = 0x27,
	VM_INST_XOR     = 0x28,
	VM_INST_OR      = 0x29,
	VM_INST_NOT     = 0x2A
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
