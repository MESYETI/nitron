$data
	@initMsg "Nitron Assembly Program" 0a 00
	@prompt "> " 00
	@input reserve 256
	@program reserve 1024
$code
	define PrintChar  00010000
	define PrintHex   00010002
	define PrintNTStr 00010004
	define InputLine  00010005
	define Assemble   00040000

	#initMsg ECALLi PrintNTStr

	@loop
		; read input
		#prompt ECALLi PrintNTStr
		#input #00000100 ECALLi InputLine

		; assemble it somewhere
		#program #00000400 #input ECALLi Assemble

		; add RET instruction
		ADDi program #0000002d SWAP WRITE8

		; call user's code
		FARCALLi program

		; loop again
		JUMPi loop
