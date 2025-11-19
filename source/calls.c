#include <stdio.h>
#include "mem.h"
#include "calls.h"

static void PrintCh(VM* vm) {
	-- vm->dsp;
	printf("%c", *vm->dsp);
	fflush(stdout);
}

static void PrintStr(VM* vm) {
	vm->dsp -= 2;

	for (uint32_t i = 0; i < vm->dsp[1]; ++ i) {
		printf("%c", ((char*) vm->dsp[0])[i]);
	}
	fflush(stdout);
}

static void PrintHex(VM* vm) {
	-- vm->dsp;
	printf("%.8X", *vm->dsp);
}

static void InputChar(VM* vm) {
	++ vm->dsp;
	vm->dsp[-1] = getchar();
}

static void Alloc(VM* vm) {
	vm->dsp[-1] = (uint32_t) SafeMalloc(vm->dsp[-1]);
}

static void Realloc(VM* vm) {
	-- vm->dsp;
	vm->dsp[-1] = (uint32_t) SafeRealloc((void*) vm->dsp[-1], vm->dsp[0]);
}

static void Free(VM* vm) {
	-- vm->dsp;
	free((void*) *vm->dsp);
}

static void Dump(VM* vm) {
	puts("=== REGISTERS ===");
	for (size_t i = 0; i < sizeof(vm->reg) / sizeof(uint32_t); ++ i) {
		printf("r%X: %.8X\n", (int) i, vm->reg[i]);
	}

	puts("=== STACK ===");
	for (size_t i = 0; i < 8; ++ i) {
		printf("%.8X %s\n", vm->dStack[i], &vm->dStack[i] == vm->dsp? "<-" : "");
	}
}

static void UserCallSize(VM* vm) {
	*vm->dsp = VM_USER_CALLS_AMOUNT;
	++ vm->dsp;
}

#define ADD_SECTION(INDEX, BUF) \
	vm->sections[INDEX] = (ECallSect) {sizeof(BUF) / sizeof(ECall), BUF}

void Calls_InitVMCalls(VM* vm) {
	static ECall sect0000[VM_USER_CALLS_AMOUNT];
	ADD_SECTION(0x0000, sect0000);

	static ECall sect0001[] = {
		/* 0x00 */ &PrintCh,
		/* 0x01 */ &PrintStr,
		/* 0x02 */ &PrintHex,
		/* 0x03 */ &InputChar
	};
	ADD_SECTION(0x0001, sect0001);

	static ECall sect0002[] = {
		/* 0x00 */ &Alloc,
		/* 0x01 */ &Realloc,
		/* 0x02 */ &Free
	};
	ADD_SECTION(0x0002, sect0002);

	static ECall sect0003[] = {
		/* 0x00 */ &Dump,
		/* 0x01 */ &UserCallSize
	};
	ADD_SECTION(0x0003, sect0003);
}
