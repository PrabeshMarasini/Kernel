; Start in 16-bit real mode
[BITS 16]
[ORG 0x7C00]

; Set up segments
xor ax, ax
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7C00

; Enable A20 line
in al, 0x92
or al, 2
out 0x92, al

; Switch to protected mode
cli                     ; Disable interrupts
lgdt [gdt_descriptor]   ; Load Global Descriptor Table
mov eax, cr0            ; Get control register 0
or eax, 1               ; Set the PE bit (Protection Enabled)
mov cr0, eax            ; Update control register 0
jmp CODE_SEG:protected_mode

; Global Descriptor Table (GDT)
gdt_start:
    dq 0x0             ; Null descriptor
gdt_code:
    dw 0xFFFF          ; Limit (bits 0-15)
    dw 0x0000          ; Base (bits 0-15)
    db 0x00            ; Base (bits 16-23)
    db 10011010b       ; Present=1, Privileg=00, Descriptor type=1 -> Code segment
    db 11001111b       ; Granularity=1, 32-bit=1, 64-bit=0, Limit=4 (bits 16-19)
    db 0x00            ; Base (bits 24-31)
gdt_data:
    dw 0xFFFF          ; Limit (bits 0-15)
    dw 0x0000          ; Base (bits 0-15)
    db 0x00            ; Base (bits 16-23)
    db 10010010b       ; Present=1, Privileg=00, Descriptor type=0 -> Data segment
    db 11001111b       ; Granularity=1, 32-bit=1, 64-bit=0, Limit=4 (bits 16-19)
    db 0x00            ; Base (bits 24-31)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT limit
    dd gdt_start                ; GDT base

[BITS 32]
protected_mode:
    mov ax, DATA_SEG    ; Update segment registers
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; TODO: Set up 64-bit paging and jump to 64-bit mode

times 510-($-$$) db 0   ; Pad remaining bytes with zeros
dw 0xAA55               ; Boot signature

; Constants
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start