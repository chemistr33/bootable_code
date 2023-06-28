ORG 0x7C00
BITS 16

start:
    mov ah, 0Eh         ;Setup ah with 0Eh to 
    mov al, 'A'
    int 0x10            ;Call BIOS routine, output 'A' to terminal
    jmp $               ;Jump to current address, infinite loop

times 510-($ - $$) db 0 ;Fill the rest of the sector with 0s
dw 0xAA55               ;Add the boot signature
