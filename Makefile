# ─────────────────────────────────────────────────────────────
# Makefile - ARM Cortex-M3 Bare Metal
# Structure:
#   src/      → application code
#   startup/  → vector table + reset handler
#   linker/   → linker script
#   build/    → all generated files
# ─────────────────────────────────────────────────────────────

# ── Toolchain ──────────────────────────────────────────────
CC      = arm-none-eabi-gcc
OBJDUMP = arm-none-eabi-objdump
OBJCOPY = arm-none-eabi-objcopy
NM      = arm-none-eabi-nm
SIZE    = arm-none-eabi-size

# ── Project ────────────────────────────────────────────────
TARGET  = firmware
LINKER  = linker/cortex_m3.ld

# ── Sources ────────────────────────────────────────────────
SRCS    = src/main.c \
          startup/startup.c

# ── Object files → all land in build/ ──────────────────────
OBJS    = $(patsubst %.c, build/%.o, $(SRCS))

# ── CPU Flags ──────────────────────────────────────────────
CPU_FLAGS = -mcpu=cortex-m3 \
            -mthumb

# ── Compiler Flags ─────────────────────────────────────────
CFLAGS  = $(CPU_FLAGS)       \
          -std=c11            \
          -Wall               \
          -Wextra             \
          -O0                 \
          -g                  \
          -ffunction-sections \
          -fdata-sections     \
          -nostdlib           \
          -ffreestanding

# ── Linker Flags ───────────────────────────────────────────
LDFLAGS = $(CPU_FLAGS)                      \
          -T$(LINKER)                        \
          -nostdlib                          \
          -Wl,--gc-sections                  \
          -Wl,-Map=build/$(TARGET).map       \
          -Wl,--print-memory-usage

# ─────────────────────────────────────────────────────────────
# RULES
# ─────────────────────────────────────────────────────────────

# ── Default: build everything then analyze ─────────────────
all: build_dirs $(TARGET).elf $(TARGET).bin analyze

# ── Create build subdirectories mirroring source layout ────
build_dirs:
	mkdir -p build/src
	mkdir -p build/startup

# ── Compile src/*.c → build/src/*.o ────────────────────────
build/src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ── Compile startup/*.c → build/startup/*.o ────────────────
build/startup/%.o: startup/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ── Link all objects → firmware.elf ────────────────────────
$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o build/$@

# ── Convert firmware.elf → firmware.bin ────────────────────
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary build/$(TARGET).elf build/$(TARGET).bin

# ── Analyze: verify your memory layout is correct ──────────
analyze: $(TARGET).elf
	@echo ""
	@echo "════════════════════════════════════════════"
	@echo "  SECTION LAYOUT (verify VMA/LMA)"
	@echo "════════════════════════════════════════════"
	$(OBJDUMP) -h build/$(TARGET).elf

	@echo ""
	@echo "════════════════════════════════════════════"
	@echo "  LINKER SYMBOLS (verify addresses)"
	@echo "════════════════════════════════════════════"
	$(NM) build/$(TARGET).elf | grep -E "_sdata|_edata|_sidata|_sbss|_ebss|_estack|_etext"

	@echo ""
	@echo "════════════════════════════════════════════"
	@echo "  FIRMWARE SIZE"
	@echo "════════════════════════════════════════════"
	$(SIZE) build/$(TARGET).elf

# ── Disassemble: see ARM Thumb instructions ─────────────────
disasm: $(TARGET).elf
	$(OBJDUMP) -d -S build/$(TARGET).elf | less

# ── Clean all generated files ───────────────────────────────
clean:
	rm -rf build/
	rm -f $(TARGET).elf $(TARGET).bin

# ── Run in QEMU Cortex-M3 emulator ─────────────────────────
qemu: $(TARGET).elf
	qemu-system-arm \
		-machine lm3s6965evb \
		-cpu cortex-m3 \
		-nographic \
		-kernel build/$(TARGET).elf

# ── Run in QEMU with GDB server (for debugging) ─────────────
qemu-gdb: $(TARGET).elf
	qemu-system-arm \
		-machine lm3s6965evb \
		-cpu cortex-m3 \
		-nographic \
		-kernel build/$(TARGET).elf \
		-S -gdb tcp::3333

# ── Phony targets ───────────────────────────────────────────
.PHONY: all build_dirs analyze disasm clean qemu qemu-gdb