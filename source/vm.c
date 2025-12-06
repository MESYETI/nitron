#include <stdio.h>
#include <string.h>
#include "vm.h"
#include "mem.h"
#include "calls.h"

static ECallSect sections[VM_ECALL_SECTIONS];
static void (*insts[128])(VM*);

static void Jump(VM* vm, uint8_t* addr) {
	vm->ip = addr;
}

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
	vm->dsp[-1] += vm->reg[8];;
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
	Jump(vm, (uint8_t*) *vm->dsp);
}

INST(Jnz) {
	vm->dsp -= 2;

	if (vm->dsp[0] != 0) {
		Jump(vm, (uint8_t*) vm->dsp[1]);
	}
}

INST(Halt) {
	vm->halted = true;
}

INST(ECall) {
	-- vm->dsp;

	uint32_t sect = (*vm->dsp & 0xFFFF0000) >> 16;
	uint32_t call =  *vm->dsp & 0x0000FFFF;

	if (sect >= VM_ECALL_SECTIONS) {
		fprintf(stderr, "Invalid e-call section: %.4X\n", sect);
		exit(1);
	}
	if (call >= sections[sect].amount) {
		fprintf(stderr, "Invalid e-call: %.4X\n", sect);
		exit(1);
	}
	if (!sections[sect].calls[call]) {
		fprintf(stderr, "Invalid e-call: %.4X\n", sect);
		exit(1);
	}

	// printf("Calling %.4X in %.4X\n", call, sect);

	sections[sect].calls[call](vm);
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

	if (vm->dsp[0] == 0) {
		Jump(vm, (uint8_t*) vm->dsp[1]);
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

INST(Swap) {
	uint32_t a = vm->dsp[-2];
	uint32_t b = vm->dsp[-1];

	vm->dsp[-2] = b;
	vm->dsp[-1] = a;
}

INST(Call) {
	-- vm->dsp;
	*vm->rsp = (uint32_t) vm->ip;
	++ vm->rsp;
	Jump(vm, (uint8_t*) *vm->dsp);
}

INST(Ret) {
	-- vm->rsp;
	Jump(vm, (uint8_t*) *vm->rsp);
}

void VM_Init(VM* vm, uint8_t* area, size_t areaSize) {
	vm->area     = area;
	vm->areaPtr  = area;
	vm->areaSize = areaSize;
	vm->ip       = 0;
	vm->code     = NULL;
	vm->codeSize = 0;
	vm->dStack   = SafeAlloc(VM_DSTACK_SIZE);
	vm->rStack   = SafeAlloc(VM_RSTACK_SIZE);
	vm->dsp      = vm->dStack;
	vm->rsp      = vm->rStack;
	vm->halted   = false;

	static bool init = false;

	if (init) return;
	init = true;

	for (size_t i = 0; i < sizeof(insts) / sizeof(void*); ++ i) {
		insts[i] = NULL;
	}

	insts[0x00] = &Inst_Nop;
	insts[0x01] = &Inst_Add;
	insts[0x02] = &Inst_Sub;
	insts[0x03] = &Inst_Mul;
	insts[0x04] = &Inst_Div;
	insts[0x05] = &Inst_Mod;
	insts[0x06] = &Inst_R2D;
	insts[0x07] = &Inst_D2R;
	insts[0x08] = &Inst_Dup;
	insts[0x09] = &Inst_Over;
	insts[0x0A] = &Inst_Drop;
	insts[0x0B] = &Inst_Rot;
	insts[0x0C] = &Inst_Area;
	insts[0x0D] = &Inst_Read;
	insts[0x0E] = &Inst_Write;
	insts[0x0F] = &Inst_Jump;
	insts[0x10] = &Inst_Jnz;
	insts[0x11] = &Inst_Halt;
	insts[0x12] = &Inst_ECall;
	insts[0x13] = &Inst_Reg;
	insts[0x14] = &Inst_WReg;
	insts[0x15] = &Inst_Read8;
	insts[0x16] = &Inst_Write8;
	insts[0x17] = &Inst_Read16;
	insts[0x18] = &Inst_Write16;
	insts[0x19] = &Inst_Jz;
	insts[0x20] = &Inst_DivMod;
	insts[0x21] = &Inst_Equ;
	insts[0x22] = &Inst_Less;
	insts[0x23] = &Inst_Greater;
	insts[0x24] = &Inst_LE;
	insts[0x25] = &Inst_GE;
	insts[0x26] = &Inst_Neg;
	insts[0x27] = &Inst_And;
	insts[0x28] = &Inst_Xor;
	insts[0x29] = &Inst_Or;
	insts[0x2A] = &Inst_Not;
	insts[0x2B] = &Inst_Swap;
	insts[0x2C] = &Inst_Call;
	insts[0x2D] = &Inst_Ret;

	Calls_InitVMCalls(sections);
}

void VM_Free(VM* vm) {
	Free(vm->dStack);
	Free(vm->rStack);
	Free(vm->code);
}

void VM_Run(VM* vm, size_t instNum) {
	for (size_t i = 0; i < instNum; ++ i) {
		uint8_t* ip   = vm->ip;
		uint8_t  inst = *vm->ip;
		++ vm->ip;

		if (inst & 0x80) { // immediate
			*vm->dsp = *((uint32_t*) vm->ip);
			++ vm->dsp;
			vm->ip  += 4;
		}

		if (!insts[inst & 0x7F]) {
			fprintf(
				stderr, "Invalid opcode %.2X at %p\n",
				inst & 0x7F, ip
			);
			exit(1);
		}

		insts[inst & 0x7F](vm);

		if (vm->halted) return;
	}
}
