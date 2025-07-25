#include <stdint.h>

// Memory mapped I/O (values from the datasheet).
#define PORTB *(volatile uint8_t*)(0x23)
#define DDRB  *(volatile uint8_t*)(0x24)

int main() {
    DDRB  |= 1; // set pin PB0 as an output
    PORTB |= 1; // set pin PB0 high

    // wait forever
    while (1) {
    }
    return 0;
}
