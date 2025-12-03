JUMPi cmd/end

%include ":rom/lib/calls.asm"

@cmd/echo
	ECALLi N_PrintNTStr
	$w #a ECALLi N_PrintChar
	RET
:echo CALLi cmd/echo :

@cmd/touch
	{00} ECALLi N_WriteTextFile
	DUP JNZi cmd/touch/fail
	DROP RET
@cmd/touch/fail
	{"Failed to create file: " 00} ECALLi N_PrintNTStr
	ECALLi N_ErrorToString ECALLi N_PrintNTStr
	$w #a ECALLi N_PrintChar
	RET
:touch CALLi cmd/touch :

@cmd/free
	ECALLi N_GetMemUsage

	ECALLi N_PrintDec {" bytes total" 00} ECALLi N_PrintNTStr
	$w #a ECALLi N_PrintChar

	ECALLi N_PrintDec {" bytes used"  00} ECALLi N_PrintNTStr
	$w #a ECALLi N_PrintChar
	RET
:free CALLi cmd/free :

@cmd/read
	ECALLi N_ReadTextFile DUP JNZi cmd/read/fail
	DROP ECALLi N_PrintNTStr
	RET
@cmd/read/fail
	{"Failed to read file: " 00} ECALLi N_PrintNTStr
	ECALLi N_ErrorToString ECALLi N_PrintNTStr
	$w #a ECALLi N_PrintChar
	RET
:read CALLi cmd/read :

@cmd/end
