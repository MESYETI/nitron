%data
	@initMsg "Nitron Assembly Playground" 0a 00
	@prompt "> " 00
	@input reserve 256
	@assembler reserve 4
	@programPtr reserve 4
	@oldProgramPtr reserve 4
%code
	%include ":rom/lib/calls.asm"

	; create assembler
	ECALLi N_NewAssembler WRITEi assembler

	; allocate an area for the assembler
	$w #1000 ECALLi N_Alloc $w #1000 READi assembler ECALLi N_SetAsmArea

	#initMsg ECALLi N_PrintNTStr
	@loop
		; free incomplete array to prevent permanent errors
		READi assembler ECALLi N_FreeIncompleteAsmRef

		; reset binary length
		READi assembler ECALLi N_ResetAsmBinLen

		; read input
		#prompt ECALLi N_PrintNTStr
		#input #00000100 ECALLi N_InputLine

		; assemble it somewhere
		#00000001 #input #assembler READ ECALLi N_RunAssembler
		JZi fail ; restart if failed

		; add RET instruction
		#programPtr READ ADD #0000002d SWAP WRITE8

		; call user's code
		#oldProgramPtr READ CALL

		; loop again
		JUMPi loop
	@fail
		{"Failed to assemble" 0a 00} ECALLi N_PrintNTStr
		JUMPi loop
