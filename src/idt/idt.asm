section .asm

extern int21h_handler
extern no_interrupt_handler

global int21h
global idt_load
global no_interrupt
global enable_interrupts
global disable_interrupts

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
    cli
    pushad
    call int21h_handler
    popad
    sti
    iret

no_interrupt:
    cli
    pushad
    call no_interrupt_handler
    popad
    sti
    iret

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
