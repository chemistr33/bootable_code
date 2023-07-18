;;;;;;;;;;;;;;;;;;;;;;;;;;;;; LameOS Kernel.asm ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;^^^^^^^^^^^^^^^^^^^;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[BITS 32]
global _start           ; Tell linker entry point of kernel binary is here
extern kernel_main      ; Tell linker kernel_main is defined elsewhere

CODE_SEG equ 0x08       ; 0x08 is the code segment offset in the GDT
DATA_SEG equ 0x10       ; 0x10 is the data segment offset in the GDT
_start:
    mov ax, DATA_SEG    ; Set up the data segment registers explicityly
    mov ds, ax          ; data segment 
    mov es, ax          ; extra segment
    mov fs, ax          ; file segment (typically for Thread local storage)
    mov gs, ax          ; general segment
    mov ss, ax          ; tells the CPU the stack segment is defined at 0x10
    mov ebp, 0x00200000 ; Stack frame at 2MB 
    mov esp, ebp        ; Stack pointer at 2MB, grows down from here

    ; Remap the Master Programmable Interrupt Controller (PIC)
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov al, 00010001b   ; Prepare al with PIC initialization command
    out 0x20, al        ; Send initialization command to the PIC, 0x20 is ICW1
    
    mov al, 0x20        ; Prepare al with PIC base interrupt vector, 0x20 (32) 
    out 0x21, al        ; Send base interrupt vector to the PIC data port, 0x21

    mov al, 00000001b   ; Prepare to put the PIC in x86 protected mode
    out 0x21, al        ; Send command to put PIC in x86 protected mode
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    sti

    call kernel_main    ; Call the kernel's main function

    jmp $               ; After kernel_main returns, hang

times 512-($ - $$) db 0 ; Pad remainder of boot sector with 0s
                        ; for a total size of 512B to keep disk driver happy

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Interrupts 0-31: CPU exceptions (divide-by-zero, page fault, etc.)

; Interrupts 32-47: Hardware interrupts from the PIC (after remapping)

; Interrupts 48-255: Available for other uses, such as software interrupts or 
; additional hardware devices