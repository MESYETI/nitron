#include <stdio.h>
#include "vm.h"
#include "mem.h"

#define INST(NAME) static void Inst_##NAME(VM* vm)

INST(Nop) {
	(void) vm;
}

INST(Add) {
	-- vm->dsp;
	vm->dsp[-1] += vm->dsp[0];
}

INST(Sub) {
	-- vm->dsp;
	vm->dsp[-1] -= vm->dsp[0];
}

INST(Mul) {
	-- vm->dsp;
	vm->dsp[-1] *= vm->dsp[0];
}

INST(Div) {
	-- vm->dsp;
	vm->dsp[-1] /= vm->dsp[0];
}

INST(Mod) {
	-- vm->dsp;
	vm->dsp[-1] %= vm->dsp[0];
}

INST(R2D) {
	-- vm->rsp;
	++ vm->dsp;

	vm->dsp[-1] = vm->rsp[0];
}

INST(D2R) {
	-- vm->dsp;
	++ vm->rsp;

	vm->rsp[-1] = vm->dsp[0];
}

INST(Dup) {
	vm->dsp[0] = vm->dsp[-1];
	++ vm->dsp;
}

INST(Over) {
	vm->dsp[0] = vm->dsp[-2];
	++ vm->dsp;
}

INST(Drop) {
	-- vm->dsp;
}

INST(Rot) {
	size_t val = vm->dsp[-3];
	vm->dsp[-3] = vm->dsp[-2];
	vm->dsp[-2] = vm->dsp[-1];
	vm->dsp[-1] = val;
}

INST(Area) {
	vm->dsp[-1] += (uint32_t) &vm->area;
}

INST(Read) {
	vm->dsp[-1] = *((uint32_t*) vm->dsp[-1]);
}

INST(Write) {
	vm->dsp -= 2;
	*((uint32_t*) vm->dsp[1]) = vm->dsp[0];
}

INST(Jump) {
	-- vm->dsp;

	if (*vm->dsp >= vm->codeSize) {
		fprintf(stderr, "Invalid jump: %.8X\n", *vm->dsp);
	}

	vm->ip = &vm->code[*vm->dsp];
}

INST(Jnz) {
	vm->dsp -= 2;

	if (vm->dsp[1] >= vm->codeSize) {
		fprintf(stderr, "Invalid jump: %.8X\n", vm->dsp[1]);
	}

	if (vm->dsp[0] != 0) {
		vm->ip = &vm->code[vm->dsp[1]];
	}
}

INST(Halt) {
	(void) vm;

	exit(0);
}

INST(ECall) {
	-- vm->dsp;

	if (*vm->dsp > 255) {
		fprintf(stderr, "Invalid call %.8X\n", *vm->dsp);
		exit(1);
	}

	if (vm->calls[*vm->dsp] == NULL) { // TODO: merge
		fprintf(stderr, "Invalid call %.8X\n", *vm->dsp);
		exit(1);
	}

	vm->calls[*vm->dsp](vm);
}

INST(Reg) {
	vm->dsp[-1] = vm->reg[vm->dsp[-1] % 8];
}

INST(WReg) {
	vm->dsp                 -= 2;
	vm->reg[vm->dsp[1] % 8]  = vm->dsp[0];
}

INST(Read8) {
	vm->dsp[-1] = *((uint8_t*) vm->dsp[-1]);
}

INST(Write8) {
	vm->dsp -= 2;
	*((uint8_t*) vm->dsp[1]) = vm->dsp[0];
}

INST(Read16) {
	vm->dsp[-1] = *((uint16_t*) vm->dsp[-1]);
}

INST(Write16) {
	vm->dsp -= 2;
	*((uint16_t*) vm->dsp[1]) = vm->dsp[0];
}

INST(Jz) {
	vm->dsp -= 2;

	if (vm->dsp[1] >= vm->codeSize) {
		fprintf(stderr, "Invalid jump: %.8X\n", vm->dsp[1]);
	}

	if (vm->dsp[0] == 0) {
		vm->ip = &vm->code[vm->dsp[1]];
	}
}

INST(DivMod) {
	uint32_t a = vm->dsp[-2];
	uint32_t b = vm->dsp[-1];

	vm->dsp[-2] = a % b;
	vm->dsp[-1] = a / b;
}

INST(Equ) {
	-- vm->dsp;

	vm->dsp[-1] = vm->dsp[-1] == vm->dsp[0]? 1 : 0;
}

INST(Less) {
	-- vm->dsp;

	vm->dsp[-1] = vm->dsp[-1] < vm->dsp[0]? 1 : 0;
}

INST(Greater) {
	-- vm->dsp;

	vm->dsp[-1] = vm->dsp[-1] > vm->dsp[0]? 1 : 0;
}

INST(LE) {
	-- vm->dsp;

	vm->dsp[-1] = vm->dsp[-1] <= vm->dsp[0]? 1 : 0;
}

INST(GE) {
	-- vm->dsp;

	vm->dsp[-1] = vm->dsp[-1] >= vm->dsp[0]? 1 : 0;
}

INST(Neg) {
	if (vm->dsp[-1] == 0) {
		vm->dsp[-1] = 1;
	}
	else {
		vm->dsp[-1] = 0;
	}
}

INST(And) {
	-- vm->dsp;
	vm->dsp[-1] &= vm->dsp[0];
}

INST(Xor) {
	-- vm->dsp;
	vm->dsp[-1] ^= vm->dsp[0];
}

INST(Or) {
	-- vm->dsp;
	vm->dsp[-1] |= vm->dsp[0];
}

INST(Not) {
	vm->dsp[-1] = ~vm->dsp[-1];
}

static uint8_t  area[VM_AREA_SIZE];
static uint8_t  code[VM_CODE_SIZE];
static uint32_t dStack[VM_DSTACK_SIZE];
static uint32_t rStack[VM_RSTACK_SIZE];

void VM_Init(VM* vm) {
	vm->area     = area;
	vm->areaSize = VM_AREA_SIZE;
	vm->ip       = 0;
	vm->code     = code;
	vm->codeSize = VM_CODE_SIZE;
	vm->dStack   = dStack;
	vm->rStack   = rStack;
	vm->dsp      = vm->dStack;
	vm->rsp      = vm->rStack;

	for (size_t i = 0; i < sizeof(vm->insts) / sizeof(void*); ++ i) {
		vm->insts[i] = NULL;
	}

	vm->insts[0x00] = &Inst_Nop;
	vm->insts[0x01] = &Inst_Add;
	vm->insts[0x02] = &Inst_Sub;
	vm->insts[0x03] = &Inst_Mul;
	vm->insts[0x04] = &Inst_Div;
	vm->insts[0x05] = &Inst_Mod;
	vm->insts[0x06] = &Inst_R2D;
	vm->insts[0x07] = &Inst_D2R;
	vm->insts[0x08] = &Inst_Dup;
	vm->insts[0x09] = &Inst_Over;
	vm->insts[0x0A] = &Inst_Drop;
	vm->insts[0x0B] = &Inst_Rot;
	vm->insts[0x0C] = &Inst_Area;
	vm->insts[0x0D] = &Inst_Read;
	vm->insts[0x0E] = &Inst_Write;
	vm->insts[0x0F] = &Inst_Jump;
	vm->insts[0x10] = &Inst_Jnz;
	vm->insts[0x11] = &Inst_Halt;
	vm->insts[0x12] = &Inst_ECall;
	vm->insts[0x13] = &Inst_Reg;
	vm->insts[0x14] = &Inst_WReg;
	vm->insts[0x15] = &Inst_Read8;
	vm->insts[0x16] = &Inst_Write8;
	vm->insts[0x17] = &Inst_Read16;
	vm->insts[0x18] = &Inst_Write16;
	vm->insts[0x19] = &Inst_Jz;
	vm->insts[0x20] = &Inst_DivMod;
	vm->insts[0x21] = &Inst_Equ;
	vm->insts[0x22] = &Inst_Less;
	vm->insts[0x23] = &Inst_Greater;
	vm->insts[0x24] = &Inst_LE;
	vm->insts[0x25] = &Inst_GE;
	vm->insts[0x26] = &Inst_Neg;
	vm->insts[0x27] = &Inst_And;
	vm->insts[0x28] = &Inst_Xor;
	vm->insts[0x29] = &Inst_Or;
	vm->insts[0x2A] = &Inst_Not;
}

void VM_Free(VM* vm) {
	free(vm->area);
}

void VM_Run(VM* vm) {
	vm->ip = vm->code;

	while (true) {
		uint8_t inst = *vm->ip;
		++ vm->ip;

		if (inst & 0x80) { // immediate
			*vm->dsp = *((uint32_t*) vm->ip);
			++ vm->dsp;
			vm->ip  += 4;
		}

		if (!vm->insts[inst & 0x7F]) {
			fprintf(
				stderr, "Invalid opcode %.2X at %.8X\n",
				inst & 0x7F, vm->ip - vm->code
			);
			exit(1);
		}

		vm->insts[inst & 0x7F](vm);
	}
}
