[bits 64]
_load_gdt:   
    lgdt [rdi]

    ; Far return
    push 0x08
    lea rax, [rel _reload_segment_regs]
    push rax
    retfq

_reload_segment_regs:
    mov ax, 0x10 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

GLOBAL _load_gdt