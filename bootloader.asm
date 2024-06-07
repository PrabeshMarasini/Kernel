[BITS 16]           ; Tells the assembler that this is 16-bit code
[ORG 0x7C00]        ; The origin, where BIOS loads the bootloader

start:
    ; Set up the segment registers
    xor ax, ax         ; Clear AX register
    mov ds, ax         ; Set Data Segment to 0
    mov es, ax         ; Set Extra Segment to 0
    mov ss, ax         ; Set Stack Segment to 0
    mov sp, 0x7C00     ; Set Stack Pointer to 0x7C00 (top of the 512-byte boot sector)

    ; Print "Hello, World!"
    mov si, hello_msg  ; Point SI to the start of the message

print_char:
    lodsb              ; Load byte at SI into AL and increment SI
    or al, al          ; Test if AL is zero (end of string)
    jz done            ; If zero, jump to done
    mov ah, 0x0E       ; BIOS teletype function (print character)
    int 0x10           ; Call BIOS interrupt
    jmp print_char     ; Repeat for next character

done:
    hlt                ; Halt the CPU

hello_msg db 'Hello, World!', 0  ; The message to be printed

times 510-($-$$) db 0  ; Fill the rest of the bootloader with zeros
dw 0xAA55              ; Boot sector signature (must be 0xAA55)
