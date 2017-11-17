PUBLIC original_thing

.code
original_thing proc
	movss DWORD PTR [rbx + 38h], xmm4		; original GTA V instruction
	ret
original_thing endp

end

