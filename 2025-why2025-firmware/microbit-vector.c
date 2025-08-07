#include <stdint.h>

void Default_Handler(void) {
    while (1) {}
}

extern uintptr_t __stack_pointer[0];

void Reset_Handler             (void);
void NMI_Handler               (void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void MemManage_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void MemoryManagement_Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler          (void) __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SecureFault_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler               (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler          (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler           (void) __attribute__ ((weak, alias("Default_Handler")));

__attribute__((section(".vectors")))
const uintptr_t __vector_table[16] = {
  (uintptr_t)(&__stack_pointer),   // Initial stack pointer
  (uintptr_t)Reset_Handler,        // Reset Handler
  (uintptr_t)NMI_Handler,          // NMI Handler
  (uintptr_t)HardFault_Handler,    // Hard Fault Handler
  (uintptr_t)MemManage_Handler,    // MPU Fault Handler
  (uintptr_t)BusFault_Handler,     // Bus Fault Handler
  (uintptr_t)UsageFault_Handler,   // Usage Fault Handler
  (uintptr_t)SecureFault_Handler,  // Secure Fault Handler
  0,                               // Reserved
  0,                               // Reserved
  0,                               // Reserved
  (uintptr_t)SVC_Handler,          // SVC Handler
  (uintptr_t)DebugMon_Handler,     // Debug Monitor Handler
  0,                               // Reserved
  (uintptr_t)PendSV_Handler,       // PendSV Handler
  (uintptr_t)SysTick_Handler,      // SysTick Handler

  // more interrupt handlers follow
};
