#include "utils.h"

// Toggles on and off a mask
void set_mask(volatile uint32_t* target, uint32_t mask, bool state) {
    if (state) *target |= mask;
    else *target &= ~mask;
}

// Sets `bits` bits, offsetted `offset_low` to the left, to the value in `state` into `target`
void set_nbit(volatile uint32_t* target, uint8_t offset_low, uint8_t bits, uint32_t state) {
    uint32_t mask = ((1 << bits) - 1);
    *target = (*target & ~(mask << offset_low)) | (state & mask) << offset_low;
}

// Sets the GPIO AF flag of a port
void set_alternate(GPIO_TypeDef* gpio, uint8_t number, uint32_t state) {
    if (number <= 7) {
        set_nbit(&gpio->AFR[0], 4 * number, 4, state);
    } else {
        set_nbit(&gpio->AFR[1], 4 * (number - 8), 4, state);
    }
}

// Read a single bit from a register
bool read_bit(volatile uint32_t* target, uint8_t bit) {
    return (*target >> bit) & 1;
}
