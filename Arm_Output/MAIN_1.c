MAIN-1
#include <stdint.h>

int main(void){
    while(1){
        /*User data in here*/
    }
    return 0;
}

OUTPUT:
pranav@pranav-Vivobook-ASUSLaptop-M1605YA-M1605YA:~/VSCode/cortex-m3-bare-metal$ make
mkdir -p build/src
mkdir -p build/startup
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -std=c11 -Wall -Wextra -O0 -g -ffunction-sections -fdata-sections -nostdlib -ffreestanding -c src/main.c -o build/src/main.o
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Tlinker/cortex_m3.ld -nostdlib -Wl,--gc-sections -Wl,-Map=build/firmware.map -Wl,--print-memory-usage  build/src/main.o  build/startup/startup.o -o build/firmware.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:         172 B       256 KB      0.07%
             RAM:           0 B        64 KB      0.00%
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
  1 .text         0000006c  00000040  00000040  00001040  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  2 .data         00000000  20000000  20000000  000010ac  2**0
                  CONTENTS, ALLOC, LOAD, DATA
  3 .bss          00000000  20000000  20000000  00000000  2**0
                  ALLOC
  4 .debug_info   000001be  00000000  00000000  000010ac  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  5 .debug_abbrev 0000012c  00000000  00000000  0000126a  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  6 .debug_aranges 00000048  00000000  00000000  00001396  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  7 .debug_rnglists 0000002c  00000000  00000000  000013de  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  8 .debug_line   000000f4  00000000  00000000  0000140a  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
  9 .debug_str    0000018c  00000000  00000000  000014fe  2**0
                  CONTENTS, READONLY, DEBUGGING, OCTETS
 10 .comment      00000026  00000000  00000000  0000168a  2**0
                  CONTENTS, READONLY
 11 .ARM.attributes 0000002d  00000000  00000000  000016b0  2**0
                  CONTENTS, READONLY
 12 .debug_frame  00000070  00000000  00000000  000016e0  2**2
                  CONTENTS, READONLY, DEBUGGING, OCTETS

════════════════════════════════════════════
  LINKER SYMBOLS (verify addresses)
════════════════════════════════════════════
arm-none-eabi-nm build/firmware.elf | grep -E "_sdata|_edata|_sidata|_sbss|_ebss|_estack|_etext"
20000000 B _ebss
20000000 D _edata
20010000 R _estack
000000ac T _etext
20000000 B _sbss
20000000 D _sdata
000000ac A _sidata

════════════════════════════════════════════
  FIRMWARE SIZE
════════════════════════════════════════════
arm-none-eabi-size build/firmware.elf
   text    data     bss     dec     hex filename
    172       0       0     172      ac build/firmware.elf
