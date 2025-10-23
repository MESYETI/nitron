#ifndef N_ASM_H
#define N_ASM_H

#include "vm.h"
#include "common.h"

uint8_t* Assemble(const char* code, size_t* size, VM* vm);

#endif
