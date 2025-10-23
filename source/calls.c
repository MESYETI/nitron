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

static void PrintHex(VM* vm) {
	-- vm->dsp;
	printf("%.8X", *vm->dsp);
}

void Calls_InitVMCalls(VM* vm) {
	vm->calls[0] = &PrintCh;
	vm->calls[1] = &PrintStr;
	vm->calls[2] = &Dump;
	vm->calls[3] = &Alloc;
	vm->calls[4] = &Realloc;
	vm->calls[5] = &Free;
	vm->calls[6] = &PrintHex;
}
