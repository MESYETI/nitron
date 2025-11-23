$data
	@initMsg "Nitron Assembly Playground" 0a 00
	@prompt "> " 00
	@input reserve 256
	@program reserve 16384
	@assembler reserve 4
	@programPtr reserve 4
	@oldProgramPtr reserve 4
$code
	define PrintChar     00010000
	define PrintHex      00010002
	define PrintNTStr    00010004
	define InputLine     00010005
	define Assemble      00040000
	define NewAssembler  00040001
	define FreeAssembler 00040002

	#program WRITEi programPtr

	; create assembler
	ECALLi NewAssembler #assembler WRITE

	#initMsg ECALLi PrintNTStr
	@loop
		; read input
		#prompt ECALLi PrintNTStr
		#input #00000100 ECALLi InputLine

		; assemble it somewhere
		#programPtr READ #00000400 #input #assembler READ ECALLi Assemble
		DUP D2R ; save for advancing the program pointer

		; add RET instruction
		#programPtr READ ADD #0000002d SWAP WRITE8

		; advance program pointer
		R2D DUP
		#programPtr READ WRITEi oldProgramPtr
		#programPtr READ ADD #00000001 ADD WRITEi programPtr

		; call user's code
		#oldProgramPtr READ CALL

		; loop again
		JUMPi loop
