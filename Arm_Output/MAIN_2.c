//MAIN-2
#include <stdint.h>

volatile uint32_t initialized_var = 0x12345678;
volatile uint32_t uninitialized_var;

int main(void)
{
    if (initialized_var != 0x12345678) {
        while (1);   // .data copy failed
    }

    if (uninitialized_var != 0) {
        while (1);   // .bss zero failed
    }

    while (1) {
        // still nothing
    }
}

OUTPUT:
make
mkdir -p build/src
mkdir -p build/startup
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -std=c11 -Wall -Wextra -O0 -g -ffunction-sections -fdata-sections -nostdlib -ffreestanding -c src/main.c -o build/src/main.o
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Tlinker/cortex_m3.ld -nostdlib -Wl,--gc-sections -Wl,-Map=build/firmware.map -Wl,--print-memory-usage  build/src/main.o  build/startup/startup.o -o build/firmware.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:         212 B       256 KB      0.08%
             RAM:           8 B        64 KB      0.01%
arm-none-eabi-objcopy -O binary build/firmware.elf build/firmware.bin

SECTION LAYOUT (verify VMA/LMA)
arm-none-eabi-objdump -h build/firmware.elf

build/firmware.elf:     file format elf32-littlearm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .isr_vector   00000040  00000000  00000000  00001000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  1 .text         00000090  00000040  00000040  00001040  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  2 .data         00000004  20000000  000000d0  00002000  2**2
                  CONTENTS, ALLOC, LOAD, DATA
  3 .bss          00000004  20000004  000000d4  00002004  2**2
                  ALLOC
  4 .debug_info   000001ef  00000000  00000000  00002004  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  5 .debug_abbrev 00000157  00000000  00000000  000021f3  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  6 .debug_aranges 00000048  00000000  00000000  0000234a  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  7 .debug_rnglists 0000002c  00000000  00000000  00002392  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  8 .debug_line   0000013c  00000000  00000000  000023be  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  9 .debug_str    0000019e  00000000  00000000  000024fa  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 10 .comment      00000026  00000000  00000000  00002698  2**0
                  CONTENTS, READONLY
 11 .ARM.attributes 0000002d  00000000  00000000  000026be  2**0
                  CONTENTS, READONLY
 12 .debug_frame  00000070  00000000  00000000  000026ec  2**2
                  CONTENTS, READONLY, DEBUGGING, OCTETS

LINKER SYMBOLS (verify addresses)
arm-none-eabi-nm build/firmware.elf | grep -E "_sdata|_edata|_sidata|_sbss|_ebss|_estack|_etext"
20000008 B _ebss
20000004 D _edata
20010000 R _estack
000000d0 T _etext
20000004 B _sbss
20000000 D _sdata
000000d0 A _sidata

FIRMWARE SIZE
arm-none-eabi-size build/firmware.elf
   text    data     bss     dec     hex filename
    208       4       4     216      d8 build/firmware.elf
