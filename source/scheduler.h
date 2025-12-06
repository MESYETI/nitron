#ifndef N_SCHEDULER_H
#define N_SCHEDULER_H

#include "vm.h"
#include "error.h"

#define PROCESS_POOL_SIZE 16

typedef struct Process Process;

struct Process {
	size_t   id;
	char*    name;
	VM       vm;
	uint8_t  area[1024];
	Process* next;
	Process* prev;
};

typedef struct {
	Process* currentProcess;
	Process* processes; // points to the last process
	Process* firstProcess;
	size_t   nextID;
} Scheduler;

extern Scheduler scheduler;

void     Scheduler_Init(void);
void     Scheduler_Free(void);
Process* Scheduler_GetProcess(size_t id);
Process* Scheduler_StartProcess(Error* error, const char* path);
void     Scheduler_Kill(Process* process);
void     Scheduler_Run(void);

#endif
