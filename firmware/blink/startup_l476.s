/*
 * Startup code for the STM32L476 (Cortex-M4).
 *
 * The vector table is the FIRST thing in flash (the CPU reads the
 * initial stack pointer from word 0 and jumps to the reset handler
 * from word 1). Every exception maps to Default_Handler, which just
 * spins — this project enables no interrupts (SPI and UART are
 * polled), so that is fine for a demo.
 *
 * Reset_Handler:
 *   1. copies .data from flash to SRAM (initialized globals),
 *   2. zeroes .bss (uninitialized globals),
 *   3. calls main().
 */
.syntax unified
.cpu cortex-m4
.thumb

.section .isr_vector, "a", %progbits
.word _estack          /* 0x00: initial SP (top of SRAM)        */
.word Reset_Handler    /* 0x04: reset                           */
.word Default_Handler  /* 0x08: NMI                             */
.word Default_Handler  /* 0x0C: HardFault                       */
.word Default_Handler  /* 0x10: MemManage                       */
.word Default_Handler  /* 0x14: BusFault                        */
.word Default_Handler  /* 0x18: UsageFault                      */
.word 0
.word 0
.word 0
.word 0
.word Default_Handler  /* 0x2C: SVCall                          */
.word Default_Handler  /* 0x30: DebugMon                        */
.word 0
.word Default_Handler  /* 0x38: PendSV                          */
.word Default_Handler  /* 0x3C: SysTick                         */
.rept 64
.word Default_Handler  /* IRQ0..IRQ63 (unused in this demo)     */
.endr

.section .text
.thumb_func
.global Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    /* Copy .data (initialized globals) from flash to SRAM. */
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata
1:  cmp r0, r1
    bge 2f
    ldr r3, [r2], #4
    str r3, [r0], #4
    b 1b
2:
    /* Zero .bss (uninitialized globals). */
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0
3:  cmp r0, r1
    bge 4f
    str r2, [r0], #4
    b 3b
4:
    bl main             /* never returns */
5:  b 5b

.thumb_func
.global Default_Handler
.type Default_Handler, %function
Default_Handler:
    b .                 /* spin forever */
