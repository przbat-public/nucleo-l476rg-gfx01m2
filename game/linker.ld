/*
 * Linker script for the NUCLEO-L476RG:
 *   1 MB flash @ 0x08000000, 96 KB SRAM1 @ 0x20000000.
 *
 * Sections:
 *   .isr_vector - the vector table, must stay at the start of flash
 *   .text       - code + read-only data (string literals, the sine LUT)
 *   .data       - initialized globals, stored in flash and copied to
 *                 SRAM at startup (see startup_l476.s)
 *   .bss        - zero-initialized globals (SRAM only)
 */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 96K
}

ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);   /* top of stack = end of SRAM1 */

SECTIONS
{
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } >FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } >RAM AT>FLASH

    _sidata = LOADADDR(.data);         /* flash copy of .data */

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >RAM
}
