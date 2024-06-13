; A simple bootloader for x86-64 using NASM

BITS 16                      ; We start in 16-bit real mode

start:
    ; Set up the stack
    cli                      ; Disable interrupts
    xor ax, ax               ; Zero out AX
    mov ds, ax               ; Set DS to 0
    mov es, ax               ; Set ES to 0
    mov ss, ax               ; Set SS to 0
    mov sp, 0x7C00           ; Set SP to 0x7C00

    ; Set up segment registers
    mov ax, 0x07C0           ; Set up data segment registers
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Set up A20 gate
    in al, 0x64              ; Read the keyboard controller status
    test al, 2               ; Check if the input buffer is full
    jnz $-2                  ; If it is, wait
    mov al, 0xD1             ; Send the write command
    out 0x64, al
    in al, 0x64              ; Read the keyboard controller status
    test al, 2               ; Check if the input buffer is full
    jnz $-2                  ; If it is, wait
    mov al, 0xDF             ; Enable A20
    out 0x60, al

    ; Switch to protected mode
    lgdt [gdt_descriptor]    ; Load the GDT
    mov eax, cr0
    or eax, 1                ; Set the PE bit (Protection Enable)
    mov cr0, eax

    jmp 0x08:protected_mode  ; Far jump to flush the prefetch queue

[BITS 32]

protected_mode:
    ; Set up segment registers for protected mode
    mov ax, 0x10             ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up the stack
    mov esp, 0x90000

    ; Enable paging
    mov eax, cr4
    or eax, 0x20             ; Enable PAE (Physical Address Extension)
    mov cr4, eax

    ; Load the PML4 table
    mov eax, cr3
    or eax, pml4_table
    mov cr3, eax

    ; Set up long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100            ; Set the LME bit (Long Mode Enable)
    wrmsr

    ; Enable long mode
    mov eax, cr0
    or eax, 0x80000001       ; Set the PG and PE bits
    mov cr0, eax

    jmp 0x08:long_mode       ; Far jump to flush the prefetch queue

[BITS 64]

long_mode:
    ; Set up segment registers for long mode
    mov ax, 0x10             ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Display a message
    mov rsi, hello_message
    call print_string

hang:
    jmp hang                 ; Hang

; Function to print a string
print_string:
    mov ah, 0x0E             ; BIOS teletype function
.next_char:
    lodsb                    ; Load next character
    cmp al, 0
    je .done
    int 0x10                 ; Print character
    jmp .next_char
.done:
    ret

; GDT descriptor
gdt_start:
    dq 0x0000000000000000    ; Null descriptor
    dq 0x00AF9A000000FFFF    ; Code segment descriptor
    dq 0x00AF92000000FFFF    ; Data segment descriptor
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dq gdt_start

; PML4 table (aligned to 4096 bytes)
align 4096
pml4_table:
    dq pdpt_table | 0x03

; PDPT table (aligned to 4096 bytes)
align 4096
pdpt_table:
    dq pd_table | 0x03

; PD table (aligned to 4096 bytes)
align 4096
pd_table:
    times 512 dq 0

; Hello message
hello_message db "Hello, World!", 0

times 510 - ($ - $$) db 0   ; Pad the bootloader to 510 bytes
dw 0xAA55                   ; Boot signature
