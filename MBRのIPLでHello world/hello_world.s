        .file "hello_world.s"
        .code16gcc
        .intel_syntax noprefix

        .global _start

        .text # @ 7c00
_start:
        lea si, hello_world
        call puts
_halt:
        hlt
        jmp _halt

puts:
        pushw ax
        movb ah, 0x0e
_puts.l1:
        lodsb
        cmp al, 0x00
        jz _puts.l2
        int 0x10
        jmp _puts.l1
_puts.l2:
        popw ax
        ret

        .align 0x10
hello_world:
        .string "Hello, world!\r\n"

        .data # @ 7dfe
mgr_signature:
        .byte 0x55
        .byte 0xAA
