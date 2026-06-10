# ARM Cortex-M3 Bare-Metal Startup, Exception Handling, and QEMU Development Environment

A bare-metal ARM Cortex-M3 project demonstrating the complete boot process from reset to application execution, including custom startup code, exception handling, linker script development, UART communication, and execution/debugging using QEMU.

## Features

* Custom Cortex-M3 startup code
* Vector table implementation
* Reset handler implementation
* Weak exception handlers
* Custom linker script
* `.data` initialization
* `.bss` zero-initialization
* Bare-metal firmware build using GCC
* UART output without vendor libraries
* QEMU Cortex-M3 emulation support
* QEMU + GDB debugging support
* ELF, BIN, and MAP file generation
* Memory layout verification
* Freestanding embedded development without vendor SDKs

---

## Project Structure

```text
.
├── src/
│   └── main.c
│
├── startup/
│   └── startup.c
│
├── linker/
│   └── cortex_m3.ld
│
├── build/
│   └── (generated files)
│
└── Makefile
```

---

## Overview

When an ARM Cortex-M3 device resets, execution does not begin at `main()`.

The processor performs the following sequence:

```text
Reset
  │
  ▼
Read Initial Stack Pointer
  │
  ▼
Read Reset Handler Address
  │
  ▼
Execute Reset_Handler()
  │
  ├── Copy .data from FLASH to RAM
  ├── Zero initialize .bss
  └── Call main()
```

This project manually implements the complete startup process without relying on vendor libraries or runtime support code.

---

## Vector Table

The vector table is placed in FLASH using the `.isr_vector` section.

Implemented entries include:

* Initial Stack Pointer
* Reset Handler
* NMI Handler
* HardFault Handler
* MemManage Handler
* BusFault Handler
* UsageFault Handler
* SVC Handler
* Debug Monitor Handler
* PendSV Handler
* SysTick Handler

All unimplemented handlers are redirected to a common `Default_Handler` using weak aliases.

---

## Reset Handler

The reset handler performs the tasks normally handled by a C runtime library.

### Copy `.data`

Initialized global variables stored in FLASH are copied into RAM.

```text
FLASH (.data image)
        │
        ▼
RAM (.data section)
```

### Clear `.bss`

Uninitialized global variables are zero-initialized.

```text
RAM (.bss section)
    ↓
Set all bytes to 0
```

### Start Application

After memory initialization:

```c
main();
```

is executed.

If `main()` ever returns, execution enters an infinite loop.

---

## Weak Exception Handlers

Exception handlers are defined as weak aliases.

Example:

```c
void HardFault_Handler(void)
    __attribute__((weak, alias("Default_Handler")));
```

This allows application-specific handlers to override the defaults without modifying startup code.

---

## Memory Layout

The linker script defines:

```text
FLASH : 0x00000000
RAM   : 0x20000000
```

### Memory Regions

| Section     | Location | Purpose                    |
| ----------- | -------- | -------------------------- |
| .isr_vector | FLASH    | Vector table               |
| .text       | FLASH    | Program code and constants |
| .data       | RAM      | Initialized variables      |
| .bss        | RAM      | Zero-initialized variables |

### Linker Symbols

```text
_estack
_sidata
_sdata
_edata
_sbss
_ebss
_etext
```

These symbols are used by the reset handler to initialize memory correctly.

---

## UART Demonstration

The example application demonstrates direct register-level UART communication.

UART registers used:

| Register | Address    |
| -------- | ---------- |
| UART0_DR | 0x4000C000 |
| UART0_FR | 0x4000C018 |

The implementation demonstrates:

* Register-level peripheral access
* Character transmission
* String transmission
* Polling-based UART communication
* Bare-metal hardware interaction

No HAL, SDK, or vendor libraries are used.

---

## Build Requirements

### ARM GNU Toolchain

```bash
sudo apt install gcc-arm-none-eabi
```

Verify installation:

```bash
arm-none-eabi-gcc --version
```

### QEMU

```bash
sudo apt install qemu-system-arm
```

Verify installation:

```bash
qemu-system-arm --version
```

---

## Building the Project

```bash
make
```

Generated outputs:

```text
build/firmware.elf
build/firmware.bin
build/firmware.map
```

---

## Running in QEMU

The project can be executed entirely in software using QEMU's Cortex-M3 emulation.

Target board:

```text
LM3S6965EVB
ARM Cortex-M3
```

Run:

```bash
make qemu
```

Example UART output:

```text
Boot OK
Cortex-M3 bare metal running
Vector table initialized
still alive...
still alive...
still alive...
```

UART output is redirected to the terminal through QEMU.

---

## Debugging with GDB

Start QEMU with an embedded GDB server:

```bash
make qemu-gdb
```

QEMU starts and waits for a debugger connection.

Open a second terminal:

```bash
arm-none-eabi-gdb build/firmware.elf
```

Connect to QEMU:

```gdb
target remote localhost:3333
```

Useful debugging commands:

```gdb
monitor reset
break Reset_Handler
break main
continue
stepi
info registers
x/16wx 0x00000000
```

This enables inspection of:

* Vector table
* Reset sequence
* Register values
* Memory contents
* Exception flow
* Startup code execution

---

## Build Verification

### Section Layout

```bash
arm-none-eabi-objdump -h build/firmware.elf
```

Used to verify:

* FLASH placement
* RAM placement
* Vector table location
* Section sizes

### Linker Symbols

```bash
arm-none-eabi-nm build/firmware.elf
```

Displays:

```text
_estack
_sidata
_sdata
_edata
_sbss
_ebss
_etext
```

### Firmware Size

```bash
arm-none-eabi-size build/firmware.elf
```

Used to monitor memory consumption.

---

## Useful Commands

### Build

```bash
make
```

### Run in QEMU

```bash
make qemu
```

### Run with GDB Support

```bash
make qemu-gdb
```

### Disassemble

```bash
make disasm
```

### Clean

```bash
make clean
```

---

## Learning Objectives

This project was created to understand:

* ARM Cortex-M3 boot sequence
* Startup code implementation
* Linker script design
* Vector table construction
* Exception handling mechanisms
* Memory initialization
* UART peripheral programming
* QEMU-based embedded development
* GDB debugging of embedded targets
* Freestanding ARM development
* ARM GCC toolchain internals

---

## Future Improvements

* Dedicated HardFault handler
* Stack frame extraction
* Fault register decoding
* MemManage fault handling
* BusFault analysis
* UsageFault analysis
* SysTick timer support
* SVC implementation
* PendSV context switching
* Simple cooperative scheduler
* RTOS-style task switching

---

## License

Educational project for learning ARM Cortex-M3 bare-metal programming, startup code development, exception handling, and embedded debugging techniques.
