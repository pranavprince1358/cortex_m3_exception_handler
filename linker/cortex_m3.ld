/* linker text - memory setup for generic arm cortex m3 microcontrollers */

/* Entry Point*/
ENTRY(Reset_Handler)

/* memory region access - specifies where the memory locations lies */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K /* flash in arm m3 starts from 0x00000000 - 256kb */
    RAM (rwx): ORIGIN = 0x20000000, LENGTH = 64K  /* sram memory starts from 0x20000000 (64kb) */
}

/*Highest address of the user mode stack;
Stack starts from the absolute top of the RAM*/
_estack = ORIGIN(RAM) + LENGTH(RAM);
_Min_Stack_Size = 0x400;

SECTIONS
{
    /*Vector table must come FIRST in FLASH;
    Reads SP and PC from here on reset;*/
    .isr_vector :
    {
        . = ALIGN(4); 
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /*Executable code and read-only data section;
    Functions are placed in here*/
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /*Initialized global and static data section - stored in FLASH, copied to RAM*/
    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    /*Uninitialized global & static data and zero-initialized data - zeroed by reset handler;
    Not stored in FLASH*/
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM
}
