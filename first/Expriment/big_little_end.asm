global _main
section .data
    number  dd  0x12345678

section .text
_main:
    mov rax,    number
    call b
    
b:
    mov rax,    number
