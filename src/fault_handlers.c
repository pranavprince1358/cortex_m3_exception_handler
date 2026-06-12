/* fault_handlers.c;
 Real fault handlers that read SCB registers and report fault cause via UART */

#include <stdint.h>

/* SCB Registers - defines System Control Block register addresses*/
#define SCB_BASE    (0xE000ED00UL)
#define SCB_HFSR    (*((volatile uint32_t *)(SCB_BASE + 0x2C)))
#define SCB_CFSR    (*((volatile uint32_t *)(SCB_BASE + 0x28)))
#define SCB_MMFAR   (*((volatile uint32_t *)(SCB_BASE + 0x34)))
#define SCB_BFAR    (*((volatile uint32_t *)(SCB_BASE + 0x38)))

/* CFSR sub-register masks - bits that must be set for particular faults to trigger*/
#define UFSR_DIVBYZERO   (1UL << 25)
#define UFSR_UNALIGNED   (1UL << 24)
#define UFSR_UNDEFINSTR  (1UL << 16)
#define BFSR_BFARVALID   (1UL << 15)
#define BFSR_PRECISERR   (1UL <<  9)
#define BFSR_IBUSERR     (1UL <<  8)
#define HFSR_FORCED      (1UL << 30)
#define HFSR_VECTTBL     (1UL <<  1)

/* UART0 (QEMU lm3s6965evb) */
#define UART0_BASE      (0x4000C000UL)
#define UART0_DR        (*((volatile uint32_t *)(UART0_BASE + 0x000)))
#define UART0_FR        (*((volatile uint32_t *)(UART0_BASE + 0x018)))
#define UART_FR_TXFF    (1U << 5)

/* UART helpers */
static void uart_putchar(char c) {
    while (UART0_FR & UART_FR_TXFF);
    UART0_DR = (uint32_t)c;
}

static void uart_print(const char *s) {
    while (*s) uart_putchar(*s++);
}

/* Print a hex value without printf */
static void uart_print_hex(uint32_t val) {
    uart_print("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uint8_t nibble = (val >> i) & 0xF;
        uart_putchar(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
    }
}

/* HardFault Handler;
 Most faults escalate here if their specific handler isn't enabled or is itself faulting */
void HardFault_Handler(void)
{
    volatile uint32_t hfsr = SCB_HFSR;
    volatile uint32_t cfsr = SCB_CFSR;
    volatile uint32_t bfar = SCB_BFAR;
    volatile uint32_t mmfar = SCB_MMFAR;

    uart_print("\r\n=== HARD FAULT ===\r\n");

    uart_print("HFSR: ");
    uart_print_hex(hfsr);
    uart_print("\r\n");

    uart_print("CFSR: ");
    uart_print_hex(cfsr);
    uart_print("\r\n");

    /* Decode HFSR */
    if (hfsr & HFSR_FORCED) {
        uart_print("Cause: Escalated from lower fault\r\n");
    }
    if (hfsr & HFSR_VECTTBL) {
        uart_print("Cause: Vector table read error\r\n");
    }

    /* Decode CFSR - Usage Fault */
    if (cfsr & UFSR_DIVBYZERO) {
        uart_print("UsageFault: Divide by zero\r\n");
    }
    if (cfsr & UFSR_UNDEFINSTR) {
        uart_print("UsageFault: Undefined instruction\r\n");
    }
    if (cfsr & UFSR_UNALIGNED) {
        uart_print("UsageFault: Unaligned access\r\n");
    }

    /* Decode CFSR - Bus Fault */
    if (cfsr & BFSR_PRECISERR) {
        uart_print("BusFault: Precise error — BFAR: ");
        uart_print_hex(bfar);
        uart_print("\r\n");
    }
    if (cfsr & BFSR_IBUSERR) {
        uart_print("BusFault: Instruction bus error\r\n");
    }

    uart_print("Halting.\r\n");
    while (1);
}

/* UsageFault Handler;
 Triggered by: undefined instr, divide by zero, unaligned access, invalid state;
Must be enabled via SCB_SHCSR first;*/
void UsageFault_Handler(void)
{
    volatile uint32_t cfsr = SCB_CFSR;

    uart_print("\r\n=== USAGE FAULT ===\r\n");
    uart_print("CFSR: ");
    uart_print_hex(cfsr);
    uart_print("\r\n");

    if (cfsr & UFSR_DIVBYZERO)  uart_print("Cause: Divide by zero\r\n");
    if (cfsr & UFSR_UNDEFINSTR) uart_print("Cause: Undefined instruction\r\n");
    if (cfsr & UFSR_UNALIGNED)  uart_print("Cause: Unaligned access\r\n");

    /* Clear the fault bits by writing 1 to them */
    SCB_CFSR = cfsr;

    uart_print("Halting.\r\n");
    while (1);
}

/* BusFault Handler - hard to trigger in QEMU due to absence of bus error response path*/
void BusFault_Handler(void)
{
    volatile uint32_t cfsr = SCB_CFSR;
    volatile uint32_t bfar = SCB_BFAR;

    uart_print("\r\n=== BUS FAULT ===\r\n");
    uart_print("CFSR: ");
    uart_print_hex(cfsr);
    uart_print("\r\n");

    if (cfsr & BFSR_PRECISERR) {
        uart_print("Cause: Precise bus error at BFAR: ");
        uart_print_hex(bfar);
        uart_print("\r\n");
    }

    uart_print("Halting.\r\n");
    while (1);
}

/* Remaining handlers — default traps */
void NMI_Handler(void)      { uart_print("\r\n=== NMI ===\r\n");      while(1); }
void MemManage_Handler(void){ uart_print("\r\n=== MEMMANAGE ===\r\n");while(1); }
void SVC_Handler(void)      { while(1); }
void DebugMon_Handler(void) { while(1); }
void PendSV_Handler(void)   { while(1); }
void SysTick_Handler(void)  { while(1); }
