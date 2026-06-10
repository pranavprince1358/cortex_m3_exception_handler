# ARM Cortex-M3 Bare-Metal Startup and Exception Handling

A minimal bare-metal ARM Cortex-M3 project demonstrating:

* Custom linker script
* Vector table creation
* Reset handler implementation
* Exception handler setup
* `.data` initialization
* `.bss` zero-initialization
* Freestanding embedded build using GCC
* ELF and binary generation without vendor libraries

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

When an ARM Cortex-M3 device resets, the processor does not start by executing `main()`.

The startup sequence is:

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

This project implements that complete startup process manually.

---

## Features

### Vector Table

The vector table is placed in FLASH using the `.isr_vector` section.

It contains:

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

Unimplemented handlers are redirected to a common `Default_Handler`.

---

### Reset Handler

The reset handler performs the tasks normally done by a runtime library:

#### Copy `.data`

Initialized global variables are stored in FLASH and copied to RAM during startup.

```text
FLASH (.data image)
        │
        ▼
RAM (.data section)
```

#### Clear `.bss`

Uninitialized globals must start at zero.

```text
RAM (.bss section)
    ↓
Set all bytes to 0
```

#### Start Application

After memory initialization:

```c
main();
```

is executed.

---

### Weak Exception Handlers

Exception handlers are declared as weak aliases.

Example:

```c
void HardFault_Handler(void)
    __attribute__((weak, alias("Default_Handler")));
```

This allows user-defined handlers to override the default implementation without modifying startup code.

---

## Memory Layout

The linker script defines:

```text
FLASH : 0x00000000
RAM   : 0x20000000
```

Memory regions:

| Section     | Location | Purpose                    |
| ----------- | -------- | -------------------------- |
| .isr_vector | FLASH    | Vector table               |
| .text       | FLASH    | Code and constants         |
| .data       | RAM      | Initialized variables      |
| .bss        | RAM      | Zero-initialized variables |

Important linker symbols:

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

## Build Requirements

Install the ARM GNU Toolchain:

```bash
sudo apt install gcc-arm-none-eabi
```

Verify:

```bash
arm-none-eabi-gcc --version
```

---

## Build

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

## Useful Commands

### Build

```bash
make
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
* Startup code development
* Linker script design
* Memory sections (`.text`, `.data`, `.bss`)
* Vector tables
* Exception handling
* Freestanding embedded systems programming
* ARM GCC toolchain internals

---

## Future Improvements

* Dedicated HardFault handler
* Stack frame decoding
* Fault register analysis
* SVC implementation
* SysTick scheduler
* Context switching with PendSV
* QEMU execution support

---

## License

Educational project for learning ARM Cortex-M3 bare-metal programming.
