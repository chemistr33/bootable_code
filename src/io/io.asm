section .asm

global insb
global insw
global outb
global outw

insb:
    push ebp           ; save old base pointer
    mov ebp, esp       ; set new base pointer

    xor eax, eax       ; clear eax
    mov edx, [ebp+8]   ; get port number
    in al, dx          ; read char from port into lower 8 bits of eax (al)

    pop ebp            ; restore old base pointer, takes us back to caller
    ret                ; return to instruction immediately after call in caller

insw:
    push ebp           ; save old base pointer
    mov ebp, esp       ; set new base pointer to current stack pointer

    xor eax, eax       ; clear eax
    mov edx, [ebp+8]   ; get port number
    in ax, dx          ; read short from port into lower 16 bits of eax (ax)

    pop ebp            ; restore old base pointer, takes us back to caller
    ret                ; return to instruction immediately after call in caller

outb:
    push ebp           ; save old base pointer
    mov ebp, esp       ; set new base pointer to current stack pointer

    mov eax, [ebp+12]  ; store argument 2 in eax, the byte to write
    mov edx, [ebp+8]   ; store argument 1 in edx, the port number
    out dx, al         ; write byte in lower 8 bits of eax (al) to port

    pop ebp            ; restore old base pointer, takes us back to caller
    ret                ; return to instruction immediately after call in caller

outw:
    push ebp           ; save old base pointer
    mov ebp, esp       ; set new base pointer to current stack pointer

    mov eax, [ebp+12]  ; store argument 2 in eax, the short to write
    mov edx, [ebp+8]   ; store argument 1 in edx, the port number
    out dx, ax         ; write short in lower 16 bits of eax (ax) to port

    pop ebp            ; restore old base pointer, takes us back to caller
    ret                ; return to instruction immediately after call in caller








;       _______________
;       |...          | <-   ...        Higher Memory (Towards bottom of stack)
;       |ebp+(4n)     | <-   nth argument
;       |...          | <-   ...
;       |ebp+12       | <-   second argument (arg2)
;       |ebp+8        | <-   first argument (arg1)
;       |ebp+4        | <-   return address (saved by the call instruction)
;       |ebp          | <-   old ebp (saved base pointer of caller)
;       |ebp-4        | <-   local variable 1
;       |ebp-8        | <-   local variable 2
;       |...          | <-   ...
;       |ebp-(4n)     | <-   local variable n
;       |...          | <-   ...         Lower Memory (Towards top of stack)
;       |...__________|







