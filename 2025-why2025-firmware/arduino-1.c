#include <stdint.h>

// Memory mapped I/O (values from the datasheet).
#define DDRB  *(volatile uint8_t*)(0x24)
#define PORTB *(volatile uint8_t*)(0x25)

void _start() {
    DDRB  |= 1 << 5; // set pin PB5 as an output
    PORTB |= 1 << 5; // set pin PB5 high

    // wait forever
    while (1) {
    }
}
