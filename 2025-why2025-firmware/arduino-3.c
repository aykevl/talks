#include <stdint.h>

// Memory mapped I/O (values from the datasheet).
#define DDRB  *(volatile uint8_t*)(0x24)
#define PORTB *(volatile uint8_t*)(0x25)

void __vector_RESET() {
    DDRB |= (1<<5); // set pin PB5 as an output

    // main loop
    while (1) {
        for (long i=1000000; i != 0; i--) {
            PORTB |= (1<<5); // set pin PB5 high (set bit)
        }
        for (long i=1000000; i != 0; i--) {
            PORTB &= ~(1<<5); // set pin PB5 low (clear bit)
        }
    }
}
