#include <stdio.h>
#include "fs.h"
#include "asm.h"
#include "mem.h"
#include "util.h"
#include "scheduler.h"

Scheduler scheduler;

void Scheduler_Init(void) {
	scheduler.currentProcess = NULL;
	scheduler.processes      = NULL;
	scheduler.firstProcess   = NULL;
	scheduler.nextID         = 1;
}

static void FreeProcesses(Process* process) {
	if (process->prev) {
		FreeProcesses(process->prev);
	}

	Free(process);
}

void Scheduler_Free(void) {
	if (scheduler.processes) {
		FreeProcesses(scheduler.processes);
	}
}

static Process* GetProcess(Process* proc, size_t id) {
	if (proc->id == id) {
		return proc;
	}
	else if (proc->next) {
		return GetProcess(proc->next, id);
	}
	else {
		return NULL;
	}
}

Process* Scheduler_GetProcess(size_t id) {
	return GetProcess(scheduler.processes, id);
}

static void FreeProcess(Process* process) {
	process->prev->next = NULL;
	Free(process->name);
}

Process* Scheduler_StartProcess(Error* error, const char* path) {
	Process* ret;
	*error = N_ERROR_SUCCESS;

	char*  file;
	size_t size;
	Error  success = FS_ReadFile(path, &size, (uint8_t**) &file);

	if (success != N_ERROR_SUCCESS) {
		*error = success;
		return NULL;
	}

	if (scheduler.processes) {
		ret                       = SafeAlloc(sizeof(Process));
		scheduler.processes->next = ret;
		ret->prev                 = scheduler.processes;
		scheduler.processes       = ret;
	}
	else {
		ret                    = SafeAlloc(sizeof(Process));
		scheduler.processes    = ret;
		scheduler.firstProcess = ret;
		ret->prev              = NULL;
	}
	ret->next = NULL;

	ret->id = scheduler.nextID;
	++ scheduler.nextID;

	ret->name = NewString(path);
	VM_Init(&ret->vm, ret->area, 1024);

	file = Realloc(file, size + 1);

	if (file == NULL) {
		*error = N_ERROR_OUT_OF_MEMORY;
		FreeProcess(ret);
		return NULL;
	}

	file[size] = 0;	

	Assembler assembler;
	Assembler_Init(&assembler, file, &ret->vm);
	if (!Assembler_Assemble(&assembler, &ret->vm.codeSize, true)) {
		*error = N_ERROR_GENERIC;
		FreeProcess(ret);
		return NULL;
	}

	ret->vm.areaPtr = assembler.dataPtr;
	ret->vm.code    = assembler.bin;
	ret->vm.ip      = ret->vm.code;
	Assembler_Free(&assembler);
	Free(file);
	return ret;
}

void Scheduler_Run(void) {
	scheduler.currentProcess = scheduler.firstProcess;

	static bool noProc = false;

	while (true) {
		if (scheduler.currentProcess == NULL) {
			if (!noProc) {
				noProc = true;
				puts("(no processes)");
			}

			continue;
		}
		else {
			noProc = false;
		}

		VM_Run(&scheduler.currentProcess->vm, 5);
		scheduler.currentProcess = scheduler.currentProcess->next;
		if (!scheduler.currentProcess) {
			scheduler.currentProcess = scheduler.firstProcess;
		}
	}
}
