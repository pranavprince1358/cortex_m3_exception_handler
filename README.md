# ARM Cortex-M3 Bare-Metal Startup, Fault Handling, and QEMU Development Environment

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

### Fault Handling Features

* HardFault handler implementation
* UsageFault handler implementation
* BusFault handler implementation
* SCB fault register decoding
* UART-based fault diagnostics
* Divide-by-zero trap support
* Undefined instruction fault testing
* Fault escalation analysis
* Cortex-M3 exception debugging
* CFSR, HFSR, BFAR, and MMFAR inspection

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

## Cortex-M3 Fault Handling

This project extends the startup framework with real Cortex-M3 exception and fault handlers.

Implemented fault handlers:

* HardFault_Handler
* UsageFault_Handler
* BusFault_Handler

The handlers read and decode System Control Block (SCB) fault status registers and report diagnostic information over UART.

### SCB Registers Used

| Register | Address |
|----------|----------|
| CFSR | 0xE000ED28 |
| HFSR | 0xE000ED2C |
| MMFAR | 0xE000ED34 |
| BFAR | 0xE000ED38 |

The project demonstrates direct register-level access to Cortex-M3 system registers without CMSIS or vendor libraries.

---

## Exception Configuration

Configurable fault handlers are enabled through the System Handler Control and State Register (SHCSR).

```c
SCB_SHCSR |=
    SHCSR_USGFAULTENA |
    SHCSR_BUSFAULTENA |
    SHCSR_MEMFAULTENA;

````md
## Fault Status Register Decoding

The project decodes information from Cortex-M3 fault registers.

### Hard Fault Status Register (HFSR)

| Bit | Meaning |
|------|---------|
| FORCED | Escalated configurable fault |
| VECTTBL | Vector table read fault |

### Usage Fault Status Register (UFSR)

| Bit | Meaning |
|------|---------|
| DIVBYZERO | Divide-by-zero execution |
| UNDEFINSTR | Undefined instruction execution |
| UNALIGNED | Unaligned memory access |

### Bus Fault Status Register (BFSR)

| Bit | Meaning |
|------|---------|
| PRECISERR | Precise data bus fault |
| IBUSERR | Instruction bus fault |
| BFARVALID | BFAR contains valid address |

Fault causes can therefore be diagnosed without requiring a debugger.

---

## Fault Injection Tests

The project intentionally generates processor exceptions to verify fault handling logic.

### Divide By Zero

Enabled using:

```c
SCB_CCR |= CCR_DIV_0_TRP;
```

Test:

```c
volatile uint32_t result = a / b;
```

Expected output:

```text
=== USAGE FAULT ===
Cause: Divide by zero
```

---

### Undefined Instruction

Generated using:

```c
__asm volatile(".hword 0xDE00");
```

Expected output:

```text
=== USAGE FAULT ===
Cause: Undefined instruction
```

---

### Fault Escalation

If UsageFault handling is disabled:

```c
SCB_SHCSR &= ~SHCSR_USGFAULTENA;
```

the UsageFault escalates into a HardFault.

Expected output:

```text
=== HARD FAULT ===
Cause: Escalated configurable fault
```

This demonstrates Cortex-M3 fault escalation behavior.

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

### Advanced Topics

* Cortex-M3 fault architecture
* HardFault analysis
* UsageFault analysis
* BusFault analysis
* SCB register programming
* Fault status register decoding
* Fault escalation mechanisms
* Divide-by-zero trapping
* Undefined instruction faults
* Exception debugging using UART

---

## Future Improvements

* Stack frame extraction from exception entry
* Automatic register dump (R0-R3, R12, LR, PC, xPSR)
* MemManage fault demonstrations
* Recovery-oriented fault handlers
* Fault logging to RAM
* Persistent crash reporting
* Post-mortem crash analysis
* SVC implementation
* PendSV context switching
* SysTick scheduling
* Cooperative scheduler
* RTOS-style task switching

## License

Educational project for learning ARM Cortex-M3 bare-metal programming, startup code development, exception handling, and embedded debugging techniques.
