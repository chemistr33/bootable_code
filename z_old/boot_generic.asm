	section .text                ; Section containing code
	; Here's where the bootloader code starts executing
	global _start                ; Needed so linker knows where to start execution
_start:
	
	; Set up the stack
	cli                          ; Disable interrupts
	mov ax, 0x07C0               ; Set up 4K of stack space after this bootloader
	add ax, 0x20                 ; 0x20 is the size of this bootloader.
	mov ss, ax                   ; Set SS to the top of the stack
	mov sp, 0x0                  ; Set SP to the top of the stack
	sti                          ; Enable interrupts
	
	; Clear the screen
	mov ax, 0x03                 ; Clear screen
	int 0x10                     ; Call video interrupt
	
	; Print a message
	mov si, msg                  ; Put string position into SI
	call print_string            ; Call our string - printing routine
	
	; Hang forever
hang:                         ; Create an infinite loop
	jmp hang                     ; Jump to the label "hang"
	
	; Routine to print strings
print_string:                 ; Routine: output string in SI to screen
	lodsb                        ; Load the byte at DS:SI into AL
	or al, al                    ; Set zero flag if AL=0
	jz done_printing             ; Jump to end of routine if zero flag is set
	mov ah, 0x0E                 ; Tell BIOS we want to print 1 charater
	int 0x10                     ; Call video interrupt
	jmp print_string             ; Loop around to print the next character
	
done_printing:                ; Routine: output string in SI to screen
	ret                          ; Return from routine
	
	; Data
	section .data                ; Section containing initialised data
	msg db 'Welcome to my bootloader!', 0 ; String to print
