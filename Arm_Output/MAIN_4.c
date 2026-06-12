//MAIN-4
/* Bare metal UART output on QEMU lm3s6965evb (Stellaris Cortex-M3)
 * UART0 base: 0x4000C000
 * DR  (Data Register):    offset 0x000  → write byte here to transmit
 * FR  (Flag Register):    offset 0x018  → bit 5 = TXFF (TX FIFO full)
 */

#include <stdint.h>

/* UART0 Registers (Stellaris/TM4C style) */
#define UART0_BASE      (0x4000C000UL)
#define UART0_DR        (*((volatile uint32_t *)(UART0_BASE + 0x000)))
#define UART0_FR        (*((volatile uint32_t *)(UART0_BASE + 0x018)))
#define UART_FR_TXFF    (1U << 5)   /* TX FIFO full flag — wait when set */

/* Send one character */
static void uart_putchar(char c)
{
    /* Wait until TX FIFO has space */
    while (UART0_FR & UART_FR_TXFF);
    UART0_DR = (uint32_t)c;
}

/* Send a string */
static void uart_print(const char *str)
{
    while (*str) {
        uart_putchar(*str++);
    }
}

int main(void)
{
    uart_print("Boot OK\r\n");
    uart_print("Cortex-M3 bare metal running\r\n");
    uart_print("Vector table: 0x00000000\r\n");
    uart_print("Stack top:    0x20010000\r\n");

    uint32_t counter = 0;
    while (1) {
        counter++;

        /* Print every 1,000,000 iterations so you can see it's alive */
        if (counter % 1000000 == 0) {
            uart_print("still alive...\r\n");
        }
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
           FLASH:         476 B       256 KB      0.18%
             RAM:           0 B        64 KB      0.00%
arm-none-eabi-objcopy -O binary build/firmware.elf build/firmware.bin

SECTION LAYOUT (verify VMA/LMA)
arm-none-eabi-objdump -h build/firmware.elf

build/firmware.elf:     file format elf32-littlearm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .isr_vector   00000040  00000000  00000000  00001000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  1 .text         0000019c  00000040  00000040  00001040  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  2 .data         00000000  20000000  20000000  000011dc  2**0
                  CONTENTS, ALLOC, LOAD, DATA
  3 .bss          00000000  20000000  20000000  00000000  2**0
                  ALLOC
  4 .debug_info   00000234  00000000  00000000  000011dc  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  5 .debug_abbrev 000001a0  00000000  00000000  00001410  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  6 .debug_aranges 00000058  00000000  00000000  000015b0  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  7 .debug_rnglists 00000038  00000000  00000000  00001608  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  8 .debug_line   00000196  00000000  00000000  00001640  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  9 .debug_str    000001ac  00000000  00000000  000017d6  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 10 .comment      00000026  00000000  00000000  00001982  2**0
                  CONTENTS, READONLY
 11 .ARM.attributes 0000002d  00000000  00000000  000019a8  2**0
                  CONTENTS, READONLY
 12 .debug_frame  000000c4  00000000  00000000  000019d8  2**2
                  CONTENTS, READONLY, DEBUGGING, OCTETS

LINKER SYMBOLS (verify addresses)
arm-none-eabi-nm build/firmware.elf | grep -E "_sdata|_edata|_sidata|_sbss|_ebss|_estack|_etext"
20000000 B _ebss
20000000 D _edata
20010000 R _estack
000001dc T _etext
20000000 B _sbss
20000000 D _sdata
000001dc A _sidata

FIRMWARE SIZE
arm-none-eabi-size build/firmware.elf
   text    data     bss     dec     hex filename
    476       0       0     476     1dc build/firmware.elf

QEMU OUTPUT:
make qemu
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Tlinker/cortex_m3.ld -nostdlib -Wl,--gc-sections -Wl,-Map=build/firmware.map -Wl,--print-memory-usage  build/src/main.o  build/startup/startup.o -o build/firmware.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:         476 B       256 KB      0.18%
             RAM:           0 B        64 KB      0.00%
qemu-system-arm \
        -machine lm3s6965evb \
        -cpu cortex-m3 \
        -nographic \
        -kernel build/firmware.elf
Timer with period zero, disabling
Boot OK
Cortex-M3 bare metal running
Vector table: 0x00000000
Stack top:    0x20010000
still alive...
still alive...
still alive...
still alive...
still alive...
still alive...
still alive...
still alive...
still alive...
still alive...
still alive...
still alive...
still alive...
still alive...
still alive...
still alive...
still alive...
QEMU: Terminated
