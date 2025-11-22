#include <stdio.h>
#include "system.h"
#include "disk/mem.h"
#include "fs/ark.h"

#include "../romfs.h"

void System_Init(void) {
	puts("Nitron BETA");
	puts("Made by MESYETI\n");

	// create disks
	Disk_Init();
	Disk* romfs = Disk_Add(NewMemDisk("romfs", romfs_ark, sizeof(romfs_ark), true));
	if (!romfs) {
		fprintf(stderr, "Failed to add romfs disk\n");
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

	// mount filesystems
	Error success;
	FS_Add(Ark_CreateFileSystem(romfs, &success));
	if (success != N_ERROR_SUCCESS) {
		printf("Failed to mount romfs: %s\n", ErrorToString(success));
	}

	// print all filesystems
	puts("Filesystems:");
	for (size_t i = 0; i < FS_AMOUNT; ++ i) {
		if (!fileSystems[i]) {
			continue;
		}

		printf("    %d: %s\n", i, fileSystems[i]->name);
	}
	puts("");
}
