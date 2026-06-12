//MAIN-5
#include <stdint.h>

volatile uint32_t initialized_var = 0x12345678;
volatile uint32_t uninitialized_var;

/* UART0 for QEMU visibility */
#define UART0_BASE   (0x4000C000UL)
#define UART0_DR     (*((volatile uint32_t *)(UART0_BASE + 0x000)))
#define UART0_FR     (*((volatile uint32_t *)(UART0_BASE + 0x018)))
#define UART_FR_TXFF (1U << 5)

static void uart_putchar(char c) {
    while (UART0_FR & UART_FR_TXFF);
    UART0_DR = (uint32_t)c;
}

static void uart_print(const char *s) {
    while (*s) uart_putchar(*s++);
}

int main(void)
{
    /* .data copy */
    if (initialized_var != 0x12345678) {
        uart_print("FAIL: .data copy broken\r\n");
        while(1);
    }
    uart_print("PASS: .data copy OK\r\n");

    /* .bss zeroing */
    if (uninitialized_var != 0) {
        uart_print("FAIL: .bss not zeroed\r\n");
        while(1);
    }
    uart_print("PASS: .bss zeroed OK\r\n");

    uart_print("Boot complete\r\n");

    uint32_t counter = 0;
    while (1) {
        counter++;
    }

    return 0;
}

OUTPUT:
make clean && make
rm -rf build/
rm -f firmware.elf firmware.bin
mkdir -p build/src
mkdir -p build/startup
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -std=c11 -Wall -Wextra -O0 -g -ffunction-sections -fdata-sections -nostdlib -ffreestanding -c src/main.c -o build/src/main.o
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -std=c11 -Wall -Wextra -O0 -g -ffunction-sections -fdata-sections -nostdlib -ffreestanding -c startup/startup.c -o build/startup/startup.o
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Tlinker/cortex_m3.ld -nostdlib -Wl,--gc-sections -Wl,-Map=build/firmware.map -Wl,--print-memory-usage  build/src/main.o  build/startup/startup.o -o build/firmware.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:         484 B       256 KB      0.18%
             RAM:           8 B        64 KB      0.01%
arm-none-eabi-objcopy -O binary build/firmware.elf build/firmware.bin

SECTION LAYOUT (verify VMA/LMA)
arm-none-eabi-objdump -h build/firmware.elf

build/firmware.elf:     file format elf32-littlearm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .isr_vector   00000040  00000000  00000000  00001000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  1 .text         000001a0  00000040  00000040  00001040  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  2 .data         00000004  20000000  000001e0  00002000  2**2
                  CONTENTS, ALLOC, LOAD, DATA
  3 .bss          00000004  20000004  000001e4  00002004  2**2
                  ALLOC
  4 .debug_info   00000257  00000000  00000000  00002004  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  5 .debug_abbrev 000001bc  00000000  00000000  0000225b  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  6 .debug_aranges 00000058  00000000  00000000  00002417  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  7 .debug_rnglists 00000038  00000000  00000000  0000246f  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  8 .debug_line   000001b3  00000000  00000000  000024a7  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  9 .debug_str    000001be  00000000  00000000  0000265a  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 10 .comment      00000026  00000000  00000000  00002818  2**0
                  CONTENTS, READONLY
 11 .ARM.attributes 0000002d  00000000  00000000  0000283e  2**0
                  CONTENTS, READONLY
 12 .debug_frame  000000c4  00000000  00000000  0000286c  2**2
                  CONTENTS, READONLY, DEBUGGING, OCTETS

LINKER SYMBOLS (verify addresses)
arm-none-eabi-nm build/firmware.elf | grep -E "_sdata|_edata|_sidata|_sbss|_ebss|_estack|_etext"
20000008 B _ebss
20000004 D _edata
20010000 R _estack
000001e0 T _etext
20000004 B _sbss
20000000 D _sdata
000001e0 A _sidata

FIRMWARE SIZE
arm-none-eabi-size build/firmware.elf
   text    data     bss     dec     hex filename
    480       4       4     488     1e8 build/firmware.elf
    
QEMU OUTPUT:
make qemu
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Tlinker/cortex_m3.ld -nostdlib -Wl,--gc-sections -Wl,-Map=build/firmware.map -Wl,--print-memory-usage  build/src/main.o  build/startup/startup.o -o build/firmware.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:         484 B       256 KB      0.18%
             RAM:           8 B        64 KB      0.01%
qemu-system-arm \
        -machine lm3s6965evb \
        -cpu cortex-m3 \
        -nographic \
        -kernel build/firmware.elf
Timer with period zero, disabling
PASS: .data copy OK
PASS: .bss zeroed OK
Boot complete
QEMU: Terminated
