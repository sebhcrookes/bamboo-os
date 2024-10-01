[bits 16]

; This file, smp.asm, defines the ap_trampoline function, which is the first function to be run when APs start

GLOBAL ap_trampoline

; + 0 - this is the load address of the function
%define to_target(addr) ((addr - ap_trampoline) + 0)

; We have to subtract one from the addresses for parameters
%define to_param(addr) (to_target(addr) - 1)

ap_trampoline:

    jmp to_target(start) ; This instruction takes up 2 bytes

    PML4: dd 0 ; 32-bit pointer to the PML4
    STACK: dd 0 ; 32-bit pointer to an empty page for a stack
    GDTR: dd 0 ; 32-bit pointer to the GDTR
    CONFIRM_STARTED: db 0 ; Location to write to, to tell BSP we have started
    CONTINUE_EXEC: db 0 ; Location to read from after sending started signal, wait for BSP to allow us to keep going
    CPP_ENTRY: dq 0 ; Address to jump to, to enter C++ land :)
    APIC_ID: db 0

start:
    
    cli
    cld
    
    mov byte [to_param(CONFIRM_STARTED)], 1

    ; Looping until CONTINUE_EXEC set to true
    loop_continue:
    mov al, byte [to_param(CONTINUE_EXEC)] ; Gets the value in CONTINUE_EXEC
    ; Comparing the values, jumping if CONTINUE_EXEC =/= 1
    mov ebx, 0x01
    cmp eax, ebx
    jne loop_continue

continue:

	; Enable PAE, Paging
	mov eax, 0x668
	mov cr4, eax

	; Load PML4
    mov edx, [to_param(PML4)]
	mov cr3, edx

	mov ecx, 0xc0000080
	rdmsr
	or eax, 1 << 8
	wrmsr

    mov ebx, 0x80010031
	mov cr0, ebx

    mov eax, [to_param(GDTR)]
    lgdt [eax]

    ; Setting up the stack
    mov eax, to_param(STACK)
    mov esp, [eax]
    mov ebp, [eax]

[bits 32]

    ; Use far return to go into long mode
    push 0x8
    push to_target(ap_trampoline_64)

    retf

[bits 64]

ap_trampoline_64:

    ; Initialise segment registers
    mov ax, 0x10 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Passing arguments. The order is: rdi, rsi, rdx, rcx... etc
    mov rdi, [to_param(APIC_ID)]

    ; Calling the C function
    mov rax, [to_param(CPP_ENTRY)]
    call rax

loop:
    hlt
    jmp to_target(loop) ; This is here just in case we return (we shouldn't)
	