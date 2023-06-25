ORG 0
BITS 16

start:
    cli ; disable interrupts
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    sti ; enable interrupts   
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

message: db 'Welcome to the whatOS bootloader.', 0

times 510-($-$$) db 0
dw 0xAA55


