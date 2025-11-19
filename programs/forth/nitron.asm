; Nitron Forth source

; Nitron system calls
define CALL_PRINT_CH  00000000
define CALL_PRINT_STR 00000001
define CALL_DUMP      00000002
define CALL_ALLOC     00000003
define CALL_REALLOC   00000004
define CALL_FREE      00000005

define REG_INST_PTR  00000000
define REG_THIS_WORD 00000001

; Dictionary entry
; u32   codeword
; dict* link
; u8    flags
; u8    nameLen
; u8[]  name
; ...   definition (indirect threaded code!!)

; NEXT macro
$code
	:NEXT
		REGi REG_INST_PTR DUP ADDi %w 4 WREGi REG_INST_PTR READ
		DUP WREGi REG_THIS_WORD ; save codeword address
		READ JUMP :

	; entry point
	@start
		#cold-start WREGi REG_INST_PTR
		NEXT ; start interpreter
	@cold-start
		quit
	@do-col
		REGi REG_INST_PTR D2R
		REGi REG_THIS_WORD ADDi %w 4 DUP WREGi REG_THIS_WORD
		WREGi REG_INST_PTR
		NEXT
	@drop-def POP NEXT
	@swap-def SWAP NEXT
	@dup-def DUP NEXT
	@over-def OVER NEXT
	@rot-def ROT NEXT
	@-rot-def HALT ; TODO
	@2drop-def DROP DROP NEXT
	@2dup HALT ; TODO
	@2swap HALT ; TODO
	@?dup-def
		DUP JZi ?dup-def/1
			DUP
		@?dup-def/1 NEXT
	@1+-def     ADDi %w 1 NEXT
	@1--def     SUBi %w 1 NEXT
	@4+-def     ADDi %w 4 NEXT
	@4--def     SUBi %w 4 NEXT
	@+-def      ADD NEXT
	@--def      SUB NEXT
	@*-def      MUL NEXT
	@/mod       DIVMOD NEXT
	@=-def      EQU NEXT
	@/=         EQU NEG NEXT
	@<-def      LESS NEXT
	@>-def      GREATER NEXT
	@<=-def     LE NEXT
	@>=-def     GE NEXT
	@0=-def     %w 0 EQU NEXT
	@0/=-def    %w 0 EQU NEG NEXT
	@0<-def     %w 0 LESS NEXT
	@0>-def     %w 0 GREATER NEXT
	@0<=-def    %w 0 LE NEXT
	@0>=-def    %w 0 GE NEXT
	@and-def    AND NEXT
	@or-def     OR NEXT
	@xor-def    XOR NEXT
	@invert-def NOT NEXT
		
