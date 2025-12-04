; IO
define N_PrintChar   00010000
define N_PrintString 00010001
define N_PrintHex    00010002
define N_InputChar   00010003
define N_PrintNTStr  00010004
define N_InputLine   00010005
define N_PrintDec    00010006

; Memory
define N_Alloc   00020000
define N_Realloc 00020001
define N_Free    00020002

; VM
define N_Dump             00030000
define N_UserCallAmount   00030001
define N_RunOnNewInstance 00030002
define N_ErrorToString    00030003
define N_DumpMemory       00030004
define N_GetMemUsage      00030005
define N_SetAreaPtr       00030006
define N_GetAreaPtr       00030007
define N_Nothing          00030008

; Assembler
define N_RunAssembler         00040000
define N_NewAssembler         00040001
define N_FreeAssembler        00040002
define N_GetAsmBinPtr         00040003
define N_SetAsmArea           00040004
define N_GetAsmDataPtr        00040005
define N_SetAsmDataPtr        00040006
define N_SetAsmBin            00040007
define N_FreeIncompleteAsmRef 00040008
define N_ResetAsmBinLen       00040009
define N_SetAsmBinPtr         0004000a
define N_ExtendAsmBin         0004000b
define N_SetAsmMode           0004000c

; Filesystem
define N_ReadFile      00050000
define N_ReadTextFile  00050001
define N_WriteText     00050002
define N_WriteTextFile 00050003
