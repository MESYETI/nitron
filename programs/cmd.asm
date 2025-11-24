JMPi cmd/end

%include ":0/lib/calls.asm"

@cmd/echo
	ECALLi N_PrintNTStr RET

:echo CALLi cmd/echo :

@cmd/end
