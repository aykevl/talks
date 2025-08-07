#include <stdint.h>

typedef struct {
    volatile uint32_t OUT;
    volatile uint32_t OUTSET;
    volatile uint32_t OUTCLR;
    volatile uint32_t IN;
    volatile uint32_t DIR;
    volatile uint32_t DIRSET;
    volatile uint32_t DIRCLR;
    volatile uint32_t LATCH;
    volatile uint32_t DETECTMODE;
} GPIO_Type;

#define P0 ((GPIO_Type*)(0x50000504))

void Reset_Handler(void) {
    P0->DIRSET = 1 << 21; // set P0.21 (ROW1) as output
    P0->DIRSET = 1 << 28; // set P0.28 (COL1) as output
    P0->OUTSET = 1 << 21; // set P0.21 (ROW1) high

    while (1) {}
}
