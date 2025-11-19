$data
	@msg    "Hello 20 "World 0a
	@msg_nt "Hello 20 "World 0a 00
$code
	#msg #0000000c ECALLi 00010001

	; or an easier way
	#msg_nt ECALLi 00010004
	HALT
