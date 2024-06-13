/* linker.ld */
ENTRY(start)
SECTIONS {
    . = 0x7C00;   /* Start address of the bootloader */
    
    .text : {
        *(.text.bootloader) /* Bootloader code */
    }
    
    .data : {
        *(.data.bootloader) /* Bootloader data */
    }
    
    . = 0x8000;   /* Start address of the kernel */
    
    .text : {
        *(.text.kernel) /* Kernel code */
    }
    
    .data : {
        *(.data.kernel) /* Kernel data */
    }
    
    /* Additional sections if needed */
    
    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
    }
}
