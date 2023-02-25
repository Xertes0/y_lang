
global std_puts
std_puts:
;; rdi rsi rdx rcx r8 r9
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, 1
    mov rax, 1
    syscall

    mov rax, 0
    ret

