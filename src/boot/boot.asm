;;;;;;;;;;;;;;;;;;;;;;;;;;;;; ^^^^^^^^^^^^^^^^^ ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;                lameOS Bootloader                ;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;; vvvvvvvvvvvvvvvvv ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ORG 0x7c00
[BITS 16]

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

;;;;;;;;;;;;;; Prepare for final far-jump to 32-bit protected mode ;;;;;;;;;;;;;
    
.load_protected:
    cli                     ; disable interrupts
    lgdt [gdt_descriptor]   ; load GDT, consists of NULL, CODE_SEG, DATA_SEG
    mov eax, cr0            ; store cr0 in eax
    or eax, 0x1             ; or in 1 to bit0 of eax 
    mov cr0, eax            ; store eax in cr0, enabling protected mode...
    jmp CODE_SEG:load32     ; jump to 32-bit protected mode :)

;;;;;;;;;;;;;;;;;; Global Descriptor Table (GDT) Definition ;;;;;;;;;;;;;;;;;;;;

gdt_start:
gdt_null:           ; 8 bytes, NULL SEGMENT, required by Intel
    dd 0x0
    dd 0x0
; offset 0x8
gdt_code:           ; 8 bytes, CS SHOULD POINT TO THIS 
    dw 0xFFFF       ; segment limit first 0-15 bits
    dw 0x0          ; base address first 0-15 bits
    db 0x0          ; base address second 16-23 bits
    db 0x9A         ; access byte
    db 0xCF         ; high nibble: limit 16-19 bits, low nibble: flags
    db 0x0          ; base address third 24-31 bits
; offset 0x10
gdt_data:           ; 8 bytes, DS, SS, ES, FS, GS SHOULD POINT TO THIS
    dw 0xFFFF       ; segment limit first 0-15 bits
    dw 0x0          ; base address first 0-15 bits
    db 0x0          ; base address second 16-23 bits
    db 0x92         ; access byte
    db 0xCF         ; high nibble: limit 16-19 bits, low nibble: flags
    db 0x0          ; base address third 24-31 bits
gdt_end:            ; +24 bytes from gdt_start

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT Magnitude 24B (0x18), Limit 0x17
    dd gdt_start                ; GDT Base Address

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 32-bit Protected Mode ;;;;;;;;;;;;;;;;;;;;;;;;;;;

[BITS 32]
load32:
    ;Enable A20 Line
    in al, 0x92             ; read from IO bus port 0x92
    or al, 2                ; or in 2 to bit1 of al
    out 0x92, al            ; write al to IO bus port 0x92
    ; 
    mov eax, 1              ; sets LBA to 1, the second sector 
    mov ecx, 100            ; sets sector count to 100
    mov edi, 0x0100000      ; sets destination address to 1MB, code segment
    call ata_lba_read       ; calls the function ata_lba_read to load the kernel
                            ; into memory at 0x0100000 ... 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Jump to Kernel ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    jmp CODE_SEG:0x0100000
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;^^^^^^^^^^^^^^^^;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ata_lba_read:
    mov ebx, eax        ; save LBA in ebx, eax is needed by IO port instructions            
    shr eax, 24         ; shift eax right 24 bits...results in zero?
    or eax, 0xE0        ; or in 0xE0 to bits 7-5 of eax
    mov dx, 0x1F6       ; set dx to 0x1F6, the IO bus port for the drive/head,
                        ; bit7 always 1, bit6=1 slave, bit5=1 LBA mode
    out dx, al          ; write al to IO bus port 0x1F6


    mov eax, ecx
    mov dx, 0x1F2
    out dx, al

    mov eax, ebx 
    mov dx, 0x1F3
    out dx, al

    mov dx, 0x1F4
    mov eax, ebx 
    shr eax, 8
    out dx, al

    mov dx, 0x1F5
    mov eax, ebx
    shr eax, 16
    out dx, al

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

    .next_sector:
        push ecx

    .try_again:
        mov dx, 0x1F7
        in al, dx
        test al, 8
        jz .try_again

        mov ecx, 256
        mov dx, 0x1F0
        rep insw
        pop ecx
        loop .next_sector
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;; Pad the boot sector out to 510 bytes and add 2B boot signature  ;;;;;;;;;
times 510-($-$$) db 0
dw 0xAA55
;;;;;;.................................................................;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
