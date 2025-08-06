MEMORY {
    FLASH (r) : ORIGIN = 0, LENGTH = 32k
}

SECTIONS
{
    .text : {
        KEEP(*(.vectors))
        *(.text)
    } > FLASH
}
