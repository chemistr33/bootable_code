;;;;;;;;;;;;;;;;;;;;;;;;;;;;; ^^^^^^^^^^^^^^^^^ ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;           lameOS Bootloader                ;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;; vvvvvvvvvvvvvvvvv ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

;;;;;;;;;;;;;;;;;; BIOS PARAMETER BLOCK COMPATIBILITY LAYER ;;;;;;;;;;;;;;;;;;;;
_start:
    jmp short start
    nop

times 33 db 0

start:
jmp 0:start2
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  16-bit Real Mode ...  ;;;;;;;;;;;;;;;;;;;;;;;;;;;
start2:
    ; Setup segments
    cli             ; disable interrupts
    mov ax, 0x00    ; set up segments,
    mov ds, ax      ; setting ds to 0x7c0  
    mov es, ax      ; setting es to 0x7c0
    mov ss, ax      ; setting ss to 0x00
    mov sp, 0x7c00  ; setting sp to 0x7c00
    sti             ; enable interrupts

;;;;;;;;;;;;;;;;;;;;;;;;;;;; 32-bit Protected Mode ... ;;;;;;;;;;;;;;;;;;;;;;;;;;
    
.load_protected:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32 ; jump to 32-bit protected mode


; GDT
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

; offset 0x8
gdt_code:           ; CS SHOULD POINT TO THIS
    dw 0xFFFF       ; segment limit first 0-15 bits
    dw 0x0          ; base address first 0-15 bits
    db 0x0          ; base address second 16-23 bits
    db 0x9A         ; access byte
    db 11001111b    ; high nibble: limit 16-19 bits, low nibble: flags
    db 0x0          ; base address third 24-31 bits

; offset 0x10
gdt_data:           ; DS, SS, ES, FS, GS SHOULD POINT TO THIS
    dw 0xFFFF       ; segment limit first 0-15 bits
    dw 0x0          ; base address first 0-15 bits
    db 0x0          ; base address second 16-23 bits
    db 0x92         ; access byte
    db 11001111b    ; high nibble: limit 16-19 bits, low nibble: flags
    db 0x0          ; base address third 24-31 bits

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
load32:
    jmp $

;;;;;; Pad the boot sector to 510 bytes and add 2B boot signature  ;;;;;;;;;;;;;
times 510-($-$$) db 0
dw 0xAA55
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
