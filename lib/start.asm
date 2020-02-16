extern main

global _start

; simple start entry, wrapper for main
_start:
    xor ebp, ebp
    mov rdi, [rsp]  ; rdi is used to pass the 1st argument
    lea rsi, [rsp + 8]  ; rsi is used to pass the 2nd argument
                        ; shall pass the address `%rsp + 8`

    call main

    mov rdi, rax    ; the return is in %rax
    mov eax, 60 ; prepare system call 60
    syscall ; actual system call: 60: exit(int excode)
    int3    ; never here
