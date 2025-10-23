#include <stdio.h>
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

void Calls_InitVMCalls(VM* vm) {
	vm->calls[0] = &PrintCh;
	vm->calls[1] = &PrintStr;
}
