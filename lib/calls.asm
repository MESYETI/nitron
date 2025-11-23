; IO
define N_PrintChar   00010000
define N_PrintString 00010001
define N_PrintHex    00010002
define N_InputChar   00010003
define N_PrintNTStr  00010004
define N_InputLine   00010005

; Memory
define N_Alloc   00020000
define N_Realloc 00020001
define N_Free    00020002

; VM
define N_Dump             00030000
define N_UserCallAmount   00030001
define N_RunOnNewInstance 00030002

; Assembler
define N_RunAssembler  00040000
define N_NewAssembler  00040001
define N_FreeAssembler 00040002
