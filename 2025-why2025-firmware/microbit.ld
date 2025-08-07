MEMORY {
    FLASH (r) : ORIGIN = 0,          LENGTH = 512k
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128k
}

ENTRY(Reset_Handler)

SECTIONS
{
    .text : {
        KEEP(*(.vectors))
        *(.text)
    } > FLASH

    .stack : {
        . += 4096;
        __stack_pointer = .;
    } > RAM
}
