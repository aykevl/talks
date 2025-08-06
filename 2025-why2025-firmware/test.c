#include <stdint.h>

#define PORTB *(volatile uint8_t*)(0x23)
#define DDRB  *(volatile uint8_t*)(0x24)

int main() {
    DDRB = 1 << 5;
    //DDRB = 1 << 4;
    while (1) {
        for (long i=0; i<1000000; i++) {
            PORTB = 1 << 5;
        }
        for (long i=0; i<1000000; i++) {
            PORTB = 0;
        }
    }
    return 0;
}
