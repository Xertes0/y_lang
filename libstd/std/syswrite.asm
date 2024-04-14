format ELF64

section '.text' executable

public std_syswrite
std_syswrite:
	mov rdx, rsi
	mov rsi, rdi
	mov rdi, 1
	mov rax, 1
	syscall

	mov rax, 0
	ret
