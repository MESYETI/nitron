JUMPi cmd/end

%include ":rom/lib/calls.asm"

@cmd/echo
	ECALLi N_PrintNTStr
	$w #a ECALLi N_PrintChar
	RET

:echo CALLi cmd/echo :

@cmd/end
