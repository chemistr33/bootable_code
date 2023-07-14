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

    ; Prepare registers for disk read routine 
    mov eax, 1              ; sets LBA to 1, the second sector 
    mov ecx, 100            ; sets sector count to 100
    mov edi, 0x0100000      ; sets destination address to 1MB, code segment
    call ata_lba_read       ; calls the function ata_lba_read to load kernel
                            ; into memory at 0x0100000 ... 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Jump to Kernel ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    jmp CODE_SEG:0x0100000
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;^^^^^^^^^^^^^^^^;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ata_lba_read:
    ; Save LBA in ebx, eax is needed by subsequent IO port instructions
    mov ebx, eax                    

    ; Prepare the drive/head register
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
    shr eax, 24     ; Right-Shift EAC by 24, leaving MS Byte (value 0)
    or eax, 0xE0    ; OR in 0xE0 to set bits 7:5 of eax 
                    ; -> bit7 always 1, bit6=1 slave, bit5=1 LBA mode
    mov dx, 0x1F6   ; Set dx to 0x1F6, the IO bus port for the drive/head,
    out dx, al      ; Write al OUT to IO bus port 0x1F6
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ; Prepare the sector count register
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov eax, ecx    ; Prepare eax with the sector count 
    mov dx, 0x1F2   ; Prepare dx to 0x1F2, the IO bus port for sector count
    out dx, al      ; Write al(sector count) OUT to IO bus port 0x1F2
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ; Prepare the LBA low register
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov eax, ebx    ; Restore ebx to eax
    mov dx, 0x1F3   ; Prepare dx to 0x1F3, the IO bus port for LBA low byte
    out dx, al      ; Write al(LBA low byte) OUT to IO bus port 0x1F3
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ; Prepare the LBA mid register
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov eax, ebx    ; Restore ebx to eax
    mov dx, 0x1F4   ; Prepare dx to 0x1F4, the IO bus port for LBA mid byte
    shr eax, 8      ; Right-Shift eax by 8, leaving mid byte
    out dx, al      ; Write al(LBA mid byte) OUT to IO bus port 0x1F4
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ; Prepare the LBA high register
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov eax, ebx    ; Restore ebx to eax
    mov dx, 0x1F5   ; Prepare dx to 0x1F5, the IO bus port for LBA high byte
    shr eax, 16     ; Right-Shift eax by 16, leaving high byte
    out dx, al      ; Write al(LBA high byte) OUT to IO bus port 0x1F5
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ; Prepare the command register
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov dx, 0x1F7   ; Prepare dx to 0x1F7, the IO bus port for command                      
    mov al, 0x20    ; Prepare al to 0x20, the command for read
    out dx, al      ; Write al(command) OUT to IO bus port 0x1F7
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
    ; Read the data from the disk.
    ; Read 256 words (512 bytes) at a time.
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    .next_sector:
        push ecx            ; Save ecx, count of words to read aka loop variable

    .try_again:
        mov dx, 0x1F7       ; Prepare dx to 0x1F7, the IO bus port for status
        in al, dx           ; Read from IO bus port 0x1F7 into al
        test al, 8          ; Test bit 3 of al, if 1, disk is ready
        jz .try_again       ; If not ready, try again

        mov ecx, 256        ; Prepare ecx with 256, the number of words to read
        mov dx, 0x1F0       ; Prepare dx to 0x1F0, the IO bus port for data
        rep insw            ; Read 256 words from IO bus port 0x1F0 into es:edi
        pop ecx             ; Restore ecx 
        loop .next_sector   ; Decrement ecx, if ecx != 0, jump to .next_sector
        ret

;          *28-bit LBA Address Format*
; +--------------------+----------------------+
; |   LBA Address (0-23) |   Reserved (24-27) |
; |                      |                    |
; +-------------------+-----------------------+
; |        ATA LBA Address (32 bits)          |
; +-------------------+-----------------------+
;
;   1F0 (Read and Write): Data Register, *each time it's R/W, IDE ctrlr advances
;   1F1 (Read): Error Register
;   1F1 (Write): Features Register
;   1F2 (Read and Write): Sector Count Register
;   1F3 (Read and Write): LBA Low Register
;   1F4 (Read and Write): LBA Mid Register
;   1F5 (Read and Write): LBA High Register
;   1F6 (Read and Write): Drive/Head Register
;   1F7 (Read): Status Register
;   1F7 (Write): Command Register
;   3F6 (Read): Alternate Status Register
;   3F6 (Write): Device Control Register
;
;   Writing to Command Register (1F7) initiates a command
;   -> 0x20 = Read Sectors with Retry
;   -> 0x30 = Write Sectors with Retry

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;; Pad the boot sector out to 510 bytes and add 2B boot signature  ;;;;;;;;;
times 510-($-$$) db 0
dw 0xAA55
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
