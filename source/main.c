#include <stdio.h>
#include <string.h>
#include "vm.h"
#include "asm.h"
#include "util.h"
#include "calls.h"
#include "system.h"

int main(int argc, char** argv) {
	System_Init();

	bool  dumpRom = false;
	char* path;

	if (argc == 1) {
		printf("Usage: %s FILE.asm\n", argv[0]);
		return 0;
	}

	for (int i = 1; i < argc; ++ i) {
		if (argv[i][0] != '-') {
			path = argv[i];
		}
		else {
			if (strcmp(argv[i], "-hd") == 0) {
				dumpRom = true;
			}
			else {
				fprintf(stderr, "Unknown flag: '%s'", argv[i]);
				return 1;
			}
		}
	}

	VM vm;
	VM_Init(&vm);
	Calls_InitVMCalls(&vm);

	size_t size;
	char*  file = ReadFile(path, &size);

	Assembler assembler;
	Assembler_Init(&assembler, file, &vm);
	Assembler_Assemble(&assembler, true, &vm.codeSize);
	free(file);

	if (dumpRom) {
		printf("ROM is %d bytes\n", vm.codeSize);
		for (size_t i = 0; i < vm.codeSize; ++ i) {
			printf("%.8X: %.2X (%c)\n", i, vm.code[i], vm.code[i]);
		}

		return 0;
	}
	VM_Run(&vm);
	return 0;
}
