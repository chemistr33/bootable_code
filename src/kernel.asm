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

    call kernel_main    ; Call the kernel's main function

    jmp $               ; After kernel_main returns, hang

times 512-($ - $$) db 0 ; Pad remainder of boot sector with 0s
                        ; for a total size of 512B to keep disk driver happy

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;