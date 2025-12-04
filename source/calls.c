#include <stdio.h>
#include <string.h>
#include "fs.h"
#include "asm.h"
#include "mem.h"
#include "calls.h"
#include "error.h"

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
	fflush(stdout);
}

static void InputChar(VM* vm) {
	++ vm->dsp;
	vm->dsp[-1] = getchar();
}

static void PrintNTStr(VM* vm) {
	-- vm->dsp;
	fputs((char*) (*vm->dsp), stdout);
	fflush(stdout);
}

static void InputLine(VM* vm) {
	-- vm->dsp;
	size_t maxLen = *vm->dsp;
	-- vm->dsp;
	char* dest = (char*) *vm->dsp;

	fgets(dest, maxLen, stdin);
	dest[strlen(dest) - 1] = 0; // remove new line
}

static void PrintDec(VM* vm) {
	-- vm->dsp;
	printf("%d", *vm->dsp);
	fflush(stdout);
}

static void CallAlloc(VM* vm) {
	vm->dsp[-1] = (uint32_t) Alloc(vm->dsp[-1]);
}

static void CallRealloc(VM* vm) {
	-- vm->dsp;
	vm->dsp[-1] = (uint32_t) Realloc((void*) vm->dsp[-1], vm->dsp[0]);
}

static void CallFree(VM* vm) {
	-- vm->dsp;
	Free((void*) *vm->dsp);
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

static void RunNewInstance(VM* vm) {
	-- vm->dsp;
	uint8_t* areaPtr = (uint8_t*) *vm->dsp;
	-- vm->dsp;
	size_t stackSize = *vm->dsp;
	-- vm->dsp;
	size_t areaSize = *vm->dsp;
	-- vm->dsp;
	size_t codeSize = *vm->dsp;
	-- vm->dsp;
	uint8_t* code = (uint8_t*) *vm->dsp;

	VM vm2;
	vm2.area     = areaPtr;
	vm2.areaSize = areaSize;
	vm2.ip       = code;
	vm2.code     = code;
	vm2.codeSize = codeSize;
	vm2.dStack   = SafeMalloc(stackSize * 4);
	vm2.rStack   = SafeMalloc(stackSize * 4);
	vm2.dsp      = vm2.dStack;
	vm2.rsp      = vm2.rStack;
	Calls_InitVMCalls(&vm2);

	VM_Run(&vm2);
}

static void ErrorToStringCall(VM* vm) {
	vm->dsp[-1] = (uint32_t) ErrorToString((Error) vm->dsp[-1]);
}

static void DumpMemory(VM* vm) {
	vm->dsp -= 2;

	uint8_t* addr = (uint8_t*) vm->dsp[0];
	size_t   size = vm->dsp[1];

	for (size_t i = 0; i < size; ++ i) {
		printf("%.8X: %.2X\n", i, addr[i]);
	}
}

static void GetMemoryUsage(VM* vm) {
	MemUsage usage = GetMemUsage();
	vm->dsp[0]  = usage.used;
	vm->dsp[1]  = usage.total;
	vm->dsp    += 2;
}

static void SetAreaPtr(VM* vm) {
	-- vm->dsp;
	vm->areaPtr = (uint8_t*) vm->dsp[0];
}

static void GetAreaPtr(VM* vm) {
	vm->dsp[0] = (uint32_t) vm->areaPtr;
	++ vm->dsp;
}

static void Nothing(VM* vm) {
	(void) vm;
}

static void RunFile(VM* vm) {
	const char* path = (const char*) vm->dsp[-1];
	vm->dsp[-1]      = N_ERROR_SUCCESS;

	char*  file;
	size_t size;
	Error  success = FS_ReadFile(path, &size, (uint8_t**) &file);

	if (success != N_ERROR_SUCCESS) {
		vm->dsp[-1] = success;
		return;
	}

	file = Realloc(file, size + 1);

	if (file == NULL) {
		vm->dsp[-1] = N_ERROR_OUT_OF_MEMORY;
		return;
	}

	file[size] = 0;	

	size_t oldCodeSize = vm->codeSize;

	Assembler assembler;
	Assembler_Init(&assembler, file, vm);
	if (!Assembler_Assemble(&assembler, &vm->codeSize, true)) {
		vm->dsp[-1] = N_ERROR_GENERIC;
		return;
	}

	VM_StateStore state = VM_SaveState(vm);
	vm->areaPtr = assembler.dataPtr;
	vm->code    = assembler.bin;
	Assembler_Free(&assembler);
	Free(file);

	VM_Run(vm);
	Free(vm->code);
	VM_LoadState(vm, &state);
	vm->codeSize = oldCodeSize;
}

static void Assemble(VM* vm) {
	-- vm->dsp;
	Assembler* assembler = (Assembler*) *vm->dsp;
	-- vm->dsp;
	char* src = (char*) *vm->dsp;
	-- vm->dsp;
	bool completion = *vm->dsp != 0? true : false;

	size_t size;

	assembler->code = src;
	bool success = Assembler_Assemble(assembler, &size, completion);

	*vm->dsp = (uint32_t) size;
	++ vm->dsp;

	*vm->dsp = success? 1 : 0;
	++ vm->dsp;
}

static void NewAssembler(VM* vm) {
	Assembler* assembler = Alloc(sizeof(Assembler));

	if (assembler == NULL) {
		*vm->dsp = 0;
		++ vm->dsp;
		return;
	}

	Assembler_InitBasic(assembler);

	*vm->dsp = (uint32_t) assembler;
	++ vm->dsp;
}

static void FreeAssembler(VM* vm) {
	-- vm->dsp;
	Assembler* assembler = (Assembler*) *vm->dsp;

	Assembler_Free(assembler);
	Free(assembler);
}

static void GetAssemblerBinPtr(VM* vm) {
	Assembler* assembler = (Assembler*) vm->dsp[-1];
	vm->dsp[-1]          = (uint32_t)   assembler->binPtr;
}

static void SetAssemblerArea(VM* vm) {
	vm->dsp -= 3;
	uint8_t*   area      = (uint8_t*) vm->dsp[0];
	size_t     size      = vm->dsp[1];
	Assembler* assembler = (Assembler*) vm->dsp[2];

	assembler->area     = area;
	assembler->areaSize = size;
	assembler->dataPtr  = area;
}

static void GetAssemblerDataPtr(VM* vm) {
	Assembler* assembler = (Assembler*) vm->dsp[-1];
	vm->dsp[-1]          = (uint32_t) assembler->dataPtr;
}

static void SetAssemblerDataPtr(VM* vm) {
	vm->dsp -= 2;
	uint8_t*   data      = (uint8_t*)   vm->dsp[0];
	Assembler* assembler = (Assembler*) vm->dsp[1];

	assembler->dataPtr = data;
}

static void SetAssemblerBinary(VM* vm) {
	vm->dsp -= 3;
	uint8_t*   bin       = (uint8_t*)   vm->dsp[0];
	size_t     size      = (size_t)     vm->dsp[1];
	Assembler* assembler = (Assembler*) vm->dsp[2];

	assembler->bin    = bin;
	assembler->binLen = 0;
	assembler->binCap = size;
	assembler->binPtr = bin;
}

static void FreeIncompleteAssemblerRef(VM* vm) {
	-- vm->dsp;
	Assembler* assembler = (Assembler*) vm->dsp[0];

	if (assembler->incomplete) {
		Free(assembler->incomplete);
		assembler->incomplete    = NULL;
		assembler->incompleteLen = 0;
	}
}

static void ResetAssemblerBinLen(VM* vm) {
	-- vm->dsp;
	Assembler* assembler = (Assembler*) vm->dsp[0];

	assembler->binLen = 0;
}

static void SetAssemblerBinPtr(VM* vm) {
	vm->dsp -= 2;
	uint8_t*   bin       = (uint8_t*)   vm->dsp[0];
	Assembler* assembler = (Assembler*) vm->dsp[1];

	assembler->binPtr = bin;
}

static void ExtendAssemblerBinary(VM* vm) {
	vm->dsp -= 1;
	size_t     size      = (size_t)     vm->dsp[-1];
	Assembler* assembler = (Assembler*) vm->dsp[0];

	vm->dsp[-1] = N_ERROR_SUCCESS;

	if (!Assembler_AssertBinSpace(assembler, size)) {
		vm->dsp[-1] = N_ERROR_GENERIC;
		return;
	}
}

static void SetAssemblerMode(VM* vm) {
	vm->dsp -= 2;
	uint32_t   mode      = vm->dsp[0];
	Assembler* assembler = (Assembler*) vm->dsp[1];

	assembler->data = mode != 0;
}

static void ReadFile(VM* vm) {
	-- vm->dsp;
	const char* path = (const char*) *vm->dsp;

	size_t   size;
	uint8_t* contents;

	Error success = FS_ReadFile(path, &size, &contents);

	vm->dsp[0]  = (uint32_t) size;
	vm->dsp[1]  = (uint32_t) contents;
	vm->dsp[2]  = success;
	vm->dsp    += 3;
}

static void ReadTextFile(VM* vm) {
	-- vm->dsp;
	const char* path = (const char*) *vm->dsp;

	size_t   size;
	uint8_t* contents;

	Error success = FS_ReadFile(path, &size, &contents);

	contents = Realloc(contents, size + 1);
	if (contents != NULL) {
		contents[size] = 0;
	}

	vm->dsp[0]  = (uint32_t) contents;
	vm->dsp[1]  = success;
	vm->dsp    += 2;
}

static void WriteFile(VM* vm) {
	vm->dsp -= 3;
	const char* path = (const char*) vm->dsp[0];
	size_t      size = (size_t)      vm->dsp[1];
	uint8_t*    data = (uint8_t*)    vm->dsp[2];

	*vm->dsp = (uint32_t) FS_WriteFile(path, size, data);
	++ vm->dsp;
}

static void WriteTextFile(VM* vm) {
	vm->dsp -= 2;
	const char* path = (const char*) vm->dsp[0];
	uint8_t*    data = (uint8_t*)    vm->dsp[1];

	*vm->dsp = (uint32_t) FS_WriteFile(path, strlen((const char*) data), data);
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
		/* 0x03 */ &InputChar,
		/* 0x04 */ &PrintNTStr,
		/* 0x05 */ &InputLine,
		/* 0x06 */ &PrintDec
	};
	ADD_SECTION(0x0001, sect0001);

	static ECall sect0002[] = {
		/* 0x00 */ &CallAlloc,
		/* 0x01 */ &CallRealloc,
		/* 0x02 */ &CallFree
	};
	ADD_SECTION(0x0002, sect0002);

	static ECall sect0003[] = {
		/* 0x00 */ &Dump,
		/* 0x01 */ &UserCallSize,
		/* 0x02 */ &RunNewInstance,
		/* 0x03 */ &ErrorToStringCall,
		/* 0x04 */ &DumpMemory,
		/* 0x05 */ &GetMemoryUsage,
		/* 0x06 */ &SetAreaPtr,
		/* 0x07 */ &GetAreaPtr,
		/* 0x08 */ &Nothing,
		/* 0x09 */ &RunFile
	};
	ADD_SECTION(0x0003, sect0003);

	static ECall sect0004[] = {
		/* 0x00 */ &Assemble,
		/* 0x01 */ &NewAssembler,
		/* 0x02 */ &FreeAssembler,
		/* 0x03 */ &GetAssemblerBinPtr,
		/* 0x04 */ &SetAssemblerArea,
		/* 0x05 */ &GetAssemblerDataPtr,
		/* 0x06 */ &SetAssemblerDataPtr,
		/* 0x07 */ &SetAssemblerBinary,
		/* 0x08 */ &FreeIncompleteAssemblerRef,
		/* 0x09 */ &ResetAssemblerBinLen,
		/* 0x0a */ &SetAssemblerBinPtr,
		/* 0x0b */ &ExtendAssemblerBinary,
		/* 0x0c */ &SetAssemblerMode
	};
	ADD_SECTION(0x0004, sect0004);

	static ECall sect0005[] = {
		/* 0x00 */ &ReadFile,
		/* 0x01 */ &ReadTextFile,
		/* 0x02 */ &WriteFile,
		/* 0x03 */ &WriteTextFile
	};
	ADD_SECTION(0x0005, sect0005);
}
