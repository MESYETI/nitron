; IO
define 00010000 N_PrintChar
define 00010001 N_PrintString
define 00010002 N_PrintHex
define 00010003 N_InputChar
define 00010004 N_PrintNTStr
define 00010005 N_InputLine

; Memory
define 00020000 N_Alloc
define 00020001 N_Realloc
define 00020002 N_Free

; VM
define 00030000 N_Dump
define 00030001 N_UserCallAmount
define 00030002 N_RunOnNewInstance

; Assembler
define 00040000 N_RunAssembler
define 00040001 N_NewAssembler
define 00040002 N_FreeAssembler
