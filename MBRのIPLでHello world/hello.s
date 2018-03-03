        .file "hello.s"
        .code16gcc
        .intel_syntax noprefix

        .global _start

        .text # @ 7c00
_start:
        movb ah, 0x0e
        movb al, 'H'
        int 0x10
        movb al, 'e'
        int 0x10
        movb al, 'l'
        int 0x10
        movb al, 'l'
        int 0x10
        movb al, 'o'
        int 0x10
        movb al, ','
        int 0x10

_halt:
        hlt
        jmp _halt

        .data # @ 7dfe
mbr_signature:
        .byte 0x55
        .byte 0xAA
