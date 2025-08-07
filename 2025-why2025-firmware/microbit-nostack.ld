MEMORY {
    FLASH (r) : ORIGIN = 0, LENGTH = 512k
}

ENTRY(Reset_Handler)

SECTIONS
{
    .text : {
        KEEP(*(.vectors))
        *(.text)
    } > FLASH
}
