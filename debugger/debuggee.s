.section .text
    .global _start
_start:
    mov  $4, %eax
    mov  $1, %ebx
    mov  $message, %ecx
    mov  $msglength, %edx
    int  $0x80

    mov  $1, %eax
    mov  $0, %ebx
    int  $0x80
.section .data
    message: .ascii "Hello world!"
    msglength: .word 12
