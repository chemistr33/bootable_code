ORG 0
BITS 16
_start:
    jmp short start
    nop

handle_zero:
    mov ah, 0Eh
    mov al, 'A'
    mov bx, 0x00
    int 0x10
    iret

times 33 db 0

start:
jmp 0x7c0:start2

start2:
    cli ; disable interrupts
    mov ax, 0x7c0   ; set up segments,
    mov ds, ax      ; setting ds to 0x7c0  
    mov es, ax      ; setting es to 0x7c0
    mov ax, 0x00    ; clear out ax
    mov ss, ax      ; setting ss to 0x00
    mov sp, 0x7c00  ; setting sp to 0x7c00
    sti ; enable interrupts

    ;ss is used here because it starts at 0...
    mov word[ss:0x00], handle_zero          ; set the offset to our handler code
    mov word[ss:0x02], 0x7c0                ; set the segment to our bootloader CS

    int 0

    mov si, message
    call print
    jmp $

print:
    mov bx, 0
.loop:
    lodsb
    cmp al, 0
    je .done
    call print_char
    jmp .loop
.done:
    ret

print_char:
    mov ah, 0Eh
    int 0x10
    ret

message: db 'Welcome to the lameOS bootloader.', 0

times 510-($-$$) db 0
dw 0xAA55