//MAIN-6
/* main.c;
Deliberately triggers ARM Cortex-M3 exceptions to verify fault handlers work correctly */

#include <stdint.h>

/* SCB registers for enabling fault handlers */
#define SCB_BASE        (0xE000ED00UL)
#define SCB_CCR         (*((volatile uint32_t *)(SCB_BASE + 0x14)))
#define SCB_SHCSR       (*((volatile uint32_t *)(SCB_BASE + 0x24)))

/* SCB_CCR bits */
#define CCR_DIV_0_TRP   (1UL << 4)   /* enable divide-by-zero trap */
#define CCR_UNALIGN_TRP (1UL << 3)   /* enable unaligned access trap */

/* SCB_SHCSR bits — enable individual fault handlers */
#define SHCSR_USGFAULTENA (1UL << 18)
#define SHCSR_BUSFAULTENA (1UL << 17)
#define SHCSR_MEMFAULTENA (1UL << 16)

/* UART */
#define UART0_BASE      (0x4000C000UL)
#define UART0_DR        (*((volatile uint32_t *)(UART0_BASE + 0x000)))
#define UART0_FR        (*((volatile uint32_t *)(UART0_BASE + 0x018)))
#define UART_FR_TXFF    (1U << 5)

static void uart_putchar(char c) {
    while (UART0_FR & UART_FR_TXFF);
    UART0_DR = (uint32_t)c;
}
static void uart_print(const char *s) {
    while (*s) uart_putchar(*s++);
}

/* Fault trigger functions */

/* Divide by zero → UsageFault (if CCR.DIV_0_TRP set) */
static void trigger_div_zero(void)
{
    uart_print("Triggering divide by zero...\r\n");
    volatile uint32_t a = 10;
    volatile uint32_t b = 0;
    volatile uint32_t result = a / b;
    (void)result;
}

/* Undefined instruction → UsageFault */
static void trigger_undef_instr(void)
{
    // SCB_SHCSR &= ~(1 << 18); // Triggers HardFault if uncommented;
    uart_print("Triggering undefined instruction...\r\n");
    /* Inline ARM undefined instruction encoding */
    __asm volatile(".hword 0xDE00");
}

/* ── Main ── */
int main(void)
{
    /* Enable individual fault handlers ─ By default ALL faults escalate to HardFault;
     * Setting these bits lets UsageFault, BusFault, MemManage fire their OWN handlers instead; */
    SCB_SHCSR |= SHCSR_USGFAULTENA |
                 SHCSR_BUSFAULTENA |
                 SHCSR_MEMFAULTENA;

    /* Enable divide-by-zero trap ─ Without this, dividing by zero produces 0 silently 
    — no fault triggered at all; */
    SCB_CCR |= CCR_DIV_0_TRP;

    uart_print("=== Cortex-M3 Fault Demo ===\r\n");
    uart_print("Fault handlers enabled\r\n");
    uart_print("Divide-by-zero trap enabled\r\n\r\n");

    /* Pick ONE test to run ─ Comment out the others;
    Each one halts in its handler so only the first triggered fault matters;*/

    trigger_div_zero();       /* safe to try first */
    // trigger_undef_instr(); /* uncomment to test */

    /* Should never reach here */
    uart_print("ERROR: fault was not triggered!\r\n");
    while (1);

    return 0;
}

OUTPUT:
make clean && make
rm -rf build/
rm -f firmware.elf firmware.bin
mkdir -p build/src
mkdir -p build/startup
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -std=c11 -Wall -Wextra -O0 -g -ffunction-sections -fdata-sections -nostdlib -ffreestanding -c src/main.c -o build/src/main.o
src/main.c:58:13: warning: 'trigger_undef_instr' defined but not used [-Wunused-function]
   58 | static void trigger_undef_instr(void)
      |             ^~~~~~~~~~~~~~~~~~~
src/main.c:39:13: warning: 'trigger_null_deref' defined but not used [-Wunused-function]
   39 | static void trigger_null_deref(void)
      |             ^~~~~~~~~~~~~~~~~~
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -std=c11 -Wall -Wextra -O0 -g -ffunction-sections -fdata-sections -nostdlib -ffreestanding -c startup/startup.c -o build/startup/startup.o
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -std=c11 -Wall -Wextra -O0 -g -ffunction-sections -fdata-sections -nostdlib -ffreestanding -c src/fault_handlers.c -o build/src/fault_handlers.o
src/fault_handlers.c: In function 'HardFault_Handler':
src/fault_handlers.c:55:23: warning: unused variable 'mmfar' [-Wunused-variable]
   55 |     volatile uint32_t mmfar = SCB_MMFAR;
      |                       ^~~~~
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Tlinker/cortex_m3.ld -nostdlib -Wl,--gc-sections -Wl,-Map=build/firmware.map -Wl,--print-memory-usage  build/src/main.o  build/startup/startup.o  build/src/fault_handlers.o -o build/firmware.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:        1908 B       256 KB      0.73%
             RAM:           0 B        64 KB      0.00%
arm-none-eabi-objcopy -O binary build/firmware.elf build/firmware.bin

SECTION LAYOUT (verify VMA/LMA)
arm-none-eabi-objdump -h build/firmware.elf

build/firmware.elf:     file format elf32-littlearm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .isr_vector   00000040  00000000  00000000  00001000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  1 .text         00000734  00000040  00000040  00001040  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  2 .data         00000000  20000000  20000000  00001774  2**0
                  CONTENTS, ALLOC, LOAD, DATA
  3 .bss          00000000  20000000  20000000  00000000  2**0
                  ALLOC
  4 .debug_info   000004cd  00000000  00000000  00001774  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  5 .debug_abbrev 000002e7  00000000  00000000  00001c41  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  6 .debug_aranges 000000e8  00000000  00000000  00001f28  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  7 .debug_rnglists 000000a1  00000000  00000000  00002010  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  8 .debug_line   00000497  00000000  00000000  000020b1  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  9 .debug_str    000002c4  00000000  00000000  00002548  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 10 .comment      00000026  00000000  00000000  0000280c  2**0
                  CONTENTS, READONLY
 11 .ARM.attributes 0000002d  00000000  00000000  00002832  2**0
                  CONTENTS, READONLY
 12 .debug_frame  0000029c  00000000  00000000  00002860  2**2
                  CONTENTS, READONLY, DEBUGGING, OCTETS

LINKER SYMBOLS (verify addresses)
arm-none-eabi-nm build/firmware.elf | grep -E "_sdata|_edata|_sidata|_sbss|_ebss|_estack|_etext"
20000000 B _ebss
20000000 D _edata
20010000 R _estack
00000774 T _etext
20000000 B _sbss
20000000 D _sdata
00000774 A _sidata

FIRMWARE SIZE
arm-none-eabi-size build/firmware.elf
   text    data     bss     dec     hex filename
   1908       0       0    1908     774 build/firmware.elf
   
QEMU OUTPUT 1 - USAGE FAULT TRIGGERED - DIVIDE BY ZERO:
make qemu
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Tlinker/cortex_m3.ld -nostdlib -Wl,--gc-sections -Wl,-Map=build/firmware.map -Wl,--print-memory-usage  build/src/main.o  build/startup/startup.o  build/src/fault_handlers.o -o build/firmware.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:        1908 B       256 KB      0.73%
             RAM:           0 B        64 KB      0.00%
qemu-system-arm \
        -machine lm3s6965evb \
        -cpu cortex-m3 \
        -nographic \
        -kernel build/firmware.elf
Timer with period zero, disabling
=== Cortex-M3 Fault Demo ===
Fault handlers enabled
Divide-by-zero trap enabled

Triggering divide by zero...

=== USAGE FAULT ===
CFSR: 0x02000000
Cause: Divide by zero
Halting.
QEMU: Terminated

QEMU OUTPUT 2 - USAGE FAULT TRIGGERED - UNDEFINED INSTRUCTION:
make qemu
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Tlinker/cortex_m3.ld -nostdlib -Wl,--gc-sections -Wl,-Map=build/firmware.map -Wl,--print-memory-usage  build/src/main.o  build/startup/startup.o  build/src/fault_handlers.o -o build/firmware.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:        1884 B       256 KB      0.72%
             RAM:           0 B        64 KB      0.00%
qemu-system-arm \
        -machine lm3s6965evb \
        -cpu cortex-m3 \
        -nographic \
        -kernel build/firmware.elf
Timer with period zero, disabling
=== Cortex-M3 Fault Demo ===
Fault handlers enabled
Divide-by-zero trap enabled

Triggering undefined instruction...

=== USAGE FAULT ===
CFSR: 0x00010000
Cause: Undefined instruction
Halting.
QEMU: Terminated

QEMU OUTPUT 3 - HARD FAULT TRIGGERED - UNDEFINED INSTRUCTION:
make qemu
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Tlinker/cortex_m3.ld -nostdlib -Wl,--gc-sections -Wl,-Map=build/firmware.map -Wl,--print-memory-usage  build/src/main.o  build/startup/startup.o  build/src/fault_handlers.o -o build/firmware.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:        1900 B       256 KB      0.72%
             RAM:           0 B        64 KB      0.00%
qemu-system-arm \
        -machine lm3s6965evb \
        -cpu cortex-m3 \
        -nographic \
        -kernel build/firmware.elf
Timer with period zero, disabling
=== Cortex-M3 Fault Demo ===
Fault handlers enabled
Divide-by-zero trap enabled

Triggering undefined instruction...

=== HARD FAULT ===
HFSR: 0x40000000
CFSR: 0x00010000
Cause: Escalated from lower fault
UsageFault: Undefined instruction
Halting.
QEMU: Terminated
