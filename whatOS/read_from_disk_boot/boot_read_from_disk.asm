ORG 0
BITS 16
_start:
    jmp short start
    nop

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

    ; setup disk read...
    mov ah, 2 ; read sector cmd
    mov al, 1 ; one sector to read
    mov ch, 0 ; cylinder low eight bits
    mov cl, 2 ; read sector two
    mov dh, 0 ; head number
   ;mov dl, 0 ; drive number (already set to boot drive by BIOS)

    mov bx, buffer ; setup register bx as "buffer" to read to
    int 0x13 ; call BIOS disk interrupt
    jc error
    mov si, buffer
    call print
    jmp $

error:
    mov si, error_msg
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

error_msg:
    db "Failed to read disk", 0


times 510-($-$$) db 0
dw 0xAA55

; boot sector ends here
buffer: