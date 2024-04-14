format ELF64

extrn main

section '.text' executable

public _start
_start:
	call main

	mov rdi, rax
	mov rax, 60
	syscall
