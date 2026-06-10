MAIN-3
#include <stdint.h>

volatile uint32_t initialized_var = 0x12345678;
volatile uint32_t uninitialized_var;

#define GPIO_BASE 0x40020000  
#define GPIO_ODR  (*(volatile uint32_t*)(GPIO_BASE + 0x14))

int main(void)
{
    if (initialized_var != 0x12345678) {
        while(1); // .data copy FAILED
    }
    
    if (uninitialized_var != 0) {
        while(1);
    }
    
    uint32_t counter = 0;
    while (1) {
        GPIO_ODR = counter++;
        for (volatile int i = 0; i < 100000; i++);
    }
    
    return 0;
}

OUTPUT:
pranav@pranav-Vivobook-ASUSLaptop-M1605YA-M1605YA:~/VSCode/cortex-m3-bare-metal$ make
mkdir -p build/src
mkdir -p build/startup
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Tlinker/cortex_m3.ld -nostdlib -Wl,--gc-sections -Wl,-Map=build/firmware.map -Wl,--print-memory-usage  build/src/main.o  build/startup/startup.o -o build/firmware.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:         256 B       256 KB      0.10%
             RAM:           8 B        64 KB      0.01%
arm-none-eabi-objcopy -O binary build/firmware.elf build/firmware.bin

════════════════════════════════════════════
  SECTION LAYOUT (verify VMA/LMA)
════════════════════════════════════════════
arm-none-eabi-objdump -h build/firmware.elf

build/firmware.elf:     file format elf32-littlearm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .isr_vector   00000040  00000000  00000000  00001000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  1 .text         000000bc  00000040  00000040  00001040  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  2 .data         00000004  20000000  000000fc  00002000  2**2
                  CONTENTS, ALLOC, LOAD, DATA
  3 .bss          00000004  20000004  00000100  00002004  2**2
                  ALLOC
  4 .debug_info   0000021b  00000000  00000000  00002004  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  5 .debug_abbrev 00000182  00000000  00000000  0000221f  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  6 .debug_aranges 00000048  00000000  00000000  000023a1  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  7 .debug_rnglists 0000002c  00000000  00000000  000023e9  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  8 .debug_line   0000015c  00000000  00000000  00002415  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  9 .debug_str    000001a6  00000000  00000000  00002571  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 10 .comment      00000026  00000000  00000000  00002717  2**0
                  CONTENTS, READONLY
 11 .ARM.attributes 0000002d  00000000  00000000  0000273d  2**0
                  CONTENTS, READONLY
 12 .debug_frame  00000074  00000000  00000000  0000276c  2**2
                  CONTENTS, READONLY, DEBUGGING, OCTETS

════════════════════════════════════════════
  LINKER SYMBOLS (verify addresses)
════════════════════════════════════════════
arm-none-eabi-nm build/firmware.elf | grep -E "_sdata|_edata|_sidata|_sbss|_ebss|_estack|_etext"
20000008 B _ebss
20000004 D _edata
20010000 R _estack
000000fc T _etext
20000004 B _sbss
20000000 D _sdata
000000fc A _sidata

════════════════════════════════════════════
  FIRMWARE SIZE
════════════════════════════════════════════
arm-none-eabi-size build/firmware.elf
   text    data     bss     dec     hex filename
    252       4       4     260     104 build/firmware.elf
