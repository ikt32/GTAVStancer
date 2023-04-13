; Wheel height patch so it only sets the height once
; Wheels should stop bouncing but it break Lowriders
; MASM is weird but ok I hope this compiles normally
; also the registers should be still in place???????

PUBLIC compare_height

.data
	align 16
	floatZero REAL4		0.0
	align 16
	backup1 REAL4 0.0
	align 16
	backup2 REAL4 0.0

.code
compare_height proc
	;sub esp, 16								; save xmm1
	;movdqu XMMWORD PTR [esp], xmm1
	
	;sub esp, 16								; save xmm2
	;movdqu XMMWORD PTR [esp], xmm2
	movss backup1, xmm1
	movss backup2, xmm2

	movss xmm1, DWORD PTR [rbx + 38h]		; get wheel height
	movaps xmm2, floatZero					; get wheel default height
 
	comiss xmm1, xmm2
	je set_height							; if it's default, run original
	jmp exit								; otherwise do nothing

set_height:
	movss DWORD PTR [rbx + 38h], xmm3		; original GTA V instruction
	jmp exit

exit:
	;movdqu xmm2, XMMWORD PTR [esp]			; restore xmm2
	;add esp, 16
	
	;movdqu xmm1, XMMWORD PTR [esp]			; restore xmm1
	;add esp, 16
	movss xmm1, backup1
	movss xmm2, backup2
	ret
compare_height endp

end
