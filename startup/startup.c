#include <stdint.h>

/*External variables from linker script - contains addresses*/
extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

/*Function Declarations*/
void Reset_Handler(void);
void Default_Handler(void);
extern int main(void);

/*Default weak exception handlers - all points to Default_Handler - Declaration*/
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

/*Vector Table - stores the initial SP value and addresses of exception handlers*/
__attribute__((section(".isr_vector")))
void (* const vector_table[])(void) =
{
    (void (*)(void))(&_estack), /*Initial SP Value*/
    Reset_Handler,       
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0,0,0,0,            /*Reserved by ARM*/
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler
    /*Entries 16+: Chip-specific External IRQs*/
};

/*Reset Handler Function*/
void Reset_Handler(void){
    /*Copy .data from FLASH to RAM*/
    uint32_t *src = &_sidata; // Source - FLASH
    uint32_t *dst = &_sdata;  // Destination - RAM
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /*Zeroes .bss*/
    dst = &_sbss;
    while (dst < &_ebss){
        *dst++ = 0;
    }

    /*Breakpoint - To check the system state before main() starts
    #ifdef DEBUG
    __asm volatile("bkpt #0");
    #endif*/

    /*Calls main*/
    main();

    /*If main returns (error), keep it hanging*/
    while(1);
}

/*Default Handler - all unhandled exceptions come here*/
void Default_Handler(void){
    while(1);
}
