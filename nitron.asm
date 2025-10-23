; Nitron Forth source

; Nitron system calls
define CALL_PRINT_CH  00000000
define CALL_PRINT_STR 00000001
define CALL_DUMP      00000002

define REG_INST_PTR  00000000
define REG_THIS_WORD 00000001

; Dictionary entry
; u32  codeword
; u8   nameLen
; u8[] name
; 

$code
	; NEXT code
	; REGi REG_INST_PTR DUP ADDi 00000004 WREGi REG_INST_PTR READ READ JUMP
	
	
