section .asm

extern int21h_handler
extern no_interrupt_handler

global int21h
global idt_load
global no_interrupt
global enable_interrupts
global disable_interrupts
global isr80h_wrapper
extern isr80h_handler

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

idt_load:
    push ebp         ; Save old ebp of caller (idt_init)
    mov ebp, esp     ; Set ebp to point to current stack frame

    mov ebx, [ebp+8] ; Get address of idtr_descriptor from stack
    lidt [ebx]       ; Load idtr_descriptor into idtr register with `lidt`
    pop ebp          ; Restore old ebp of caller (idt_init)
    ret              ; Return to caller (idt_init)

int21h:
    pushad
    call int21h_handler
    popad
    iret

no_interrupt:
    pushad
    call no_interrupt_handler
    popad
    iret

isr80h_wrapper:
    ; INTERRUPT FRAME START
    ; ALREADY PUSHED TO US BY THE PROCESSOR UPON ENTRY TO THIS INTERRUPT
    ; uint32_t ip
    ; uint32_t cs
    ; uint32_t flags
    ; uint32_t sp
    ; uint32_t ss
    ; pushes the general purpose registers to the stack
    pushad 

    ; Interrupt frame ends...

    ; Push stack pointer so we're pointing to the interrupt frame
    ; we can cast it to a c struct later on to access it...
    push esp

    ; EAX holds the command, push to stack for isr80h_handler
    push eax
    call isr80h_handler
    mov dword[tmp_res], eax
    add esp, 8  ; adjust stack because we added two variables (2 * 4B)

    ; Restore GPRs for userland
    popad
    mov eax, [tmp_res]
    iretd


section .data
; Stores the return result from isr80h_handler
tmp_res: dd 0

    


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Stack Diagram ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                              ;
;  +-----------------------------+ <- High Addresses (Bottom of Stack)         ;
;  | <&idtr_descriptor=0x103020> | <- ebp+8 , first and only argument          ;
;  | <return address>   0x10057c | <- ebp+4 , just after call instruction      ;
;  | <old ebp>          0x100578 | <- ebp+0 , saved by push ebp, then esp->ebp ;
;  | <local variables>  0x100574 | <- ebp-4, -8, -12, etc.                     ;
;  *-----------------------------* <- Low Addresses (Top of Stack, grows down) ;
;                                                                              ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
