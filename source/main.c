#include <stdio.h>
#include <string.h>
#include "asm.h"
#include "mem.h"
#include "calls.h"
#include "fs/ark.h"
#include "fs/host.h"
#include "disk/mem.h"

#include "../romfs.h"

VM vm;

int main(int argc, char** argv) {
	InitAllocator();

	puts("Nitron OS in-dev");
	puts("Made by MESYETI");
	puts("This project is not made with AI and will never accept contributions by AI\n");

	// create disks
	Disk_Init();
	Disk* romfs = Disk_Add(NewMemDisk("rom", romfs_ark, sizeof(romfs_ark), true));
	if (!romfs) {
		fprintf(stderr, "Failed to add romfs disk\n");
	}

	// mount filesystems
	Error success;
	FS_Add(Ark_CreateFileSystem(romfs, &success, "rom"));
	if (success != N_ERROR_SUCCESS) {
		fprintf(stderr, "Failed to mount romfs: %s\n", ErrorToString(success));
	}

	// check arg flags/mount user drives
	size_t driveCounter = 0;
	char*  driveNames[] = {
		"0", "1", "2", "3", "4", "5", "6", "7"
	};

	for (int i = 1; i < argc; ++ i) {
		if (strcmp(argv[i], "--mount-home") == 0) {
			FS_Add(Host_CreateFileSystem(
				&success, getenv("HOME"), driveNames[driveCounter])
			);

			if (success != N_ERROR_SUCCESS) {
				fprintf(
					stderr, "Failed to mount host filesystem: %s\n",
					ErrorToString(success)
				);
			}
			++ driveCounter;
		}
	}

	// print all disks
	puts("Disks:");
	for (size_t i = 0; i < DISK_AMOUNT; ++ i) {
		if (!disks[i].active) {
			continue;
		}

		printf("    %d: %s (%d bytes)\n", i, disks[i].name, disks[i].size);
	}
	puts("");

	// print all filesystems
	puts("Filesystems:");
	for (size_t i = 0; i < FS_AMOUNT; ++ i) {
		if (!fileSystems[i]) {
			continue;
		}

		printf("    %s: %s\n", fileSystems[i]->name, fileSystems[i]->niceName);
	}
	puts("");

	char*  autoStart;
	size_t size;
	success = FS_ReadFile(":rom/autostart.txt", &size, (uint8_t**) &autoStart);

	if (success != N_ERROR_SUCCESS) {
		fprintf(stderr, "Failed to read autostart: %s\n", ErrorToString(success));
		exit(1);
	}

	autoStart       = SafeRealloc(autoStart, size + 1);
	autoStart[size] = 0;

	char* newLine = strchr(autoStart, '\n');
	if (newLine) {
		*newLine = 0;
	}

	// print memory usage
	MemUsage usage = GetMemUsage();
	if (usage.available) {
		printf("%d B total, %d B used\n\n", usage.total, usage.used);
	}

	VM_Init(&vm);
	Calls_InitVMCalls(&vm);

	char*  file;
	success = FS_ReadFile(autoStart, &size, (uint8_t**) &file);

	if (success != N_ERROR_SUCCESS) {
		fprintf(stderr, "Failed to read %s: %s\n", autoStart, ErrorToString(success));
		exit(1);
	}

	file       = SafeRealloc(file, size + 1);
	file[size] = 0;

	Assembler assembler;
	Assembler_Init(&assembler, file, &vm);
	Assembler_Assemble(&assembler, true, &vm.codeSize, true);
	Assembler_Free(&assembler);
	vm.code = assembler.bin;
	Free(file);

	/*if (dumpRom) {
		printf("ROM is %d bytes\n", vm.codeSize);
		for (size_t i = 0; i < vm.codeSize; ++ i) {
			printf("%.8X: %.2X (%c)\n", i, vm.code[i], vm.code[i]);
		}

		return;
	}*/
	VM_Run(&vm);
	Free(vm.code);

	FreeAllocator();
}
