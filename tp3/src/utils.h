#ifndef UTILS_H
#define UTILS_H

#include <stm32f4xx.h>
#include <inttypes.h>
#include <stdbool.h>

/// Utility functions, to manipulate bits

// Toggles on and off a mask
void set_mask(volatile uint32_t* target, uint32_t mask, bool state);
// Sets `bits` bits, offsetted `offset_low` to the left, to the value in `state` into `target`
void set_nbit(volatile uint32_t* target, uint8_t offset_low, uint8_t bits, uint32_t state);
// Sets the GPIO AF flag of a port
void set_alternate(GPIO_TypeDef* gpio, uint8_t number, uint32_t state);

// Read a single bit of a register
bool read_bit(volatile uint32_t* target, uint8_t bit);

#define set_twobit(target, offset_low, state) set_nbit(target, offset_low, 2, state)
#define set_bit(target, offset_low, state) set_nbit(target, offset_low, 1, state)

// Sets the GPIO mode of a port
#define MODE_READ 0b00
#define MODE_WRITE 0b01
#define MODE_ALTERNATE 0b10
#define set_mode(gpio, number, state) set_twobit(&gpio->MODER, GPIO_MODER_MODER##number##_Pos, state)

// Set a timer capture device's edge detection method
#define EDGE_RISING 0b00
#define EDGE_FALLING 0b01
#define EDGE_BOTH 0b11
#define timer_edge(timer, device, bits) set_bit(&timer->CCER, TIM_CCER_CC##device##NP_Pos, ((bits) & 0b10) != 0); \
    set_bit(&timer->CCER, TIM_CCER_CC##device##P_Pos, ((bits) & 0b01) != 0);

#define PUPD_NONE 0b00
#define PUPD_UP 0b01
#define PUPD_DOWN 0b10
#define set_pupd(gpio, number, state) set_twobit(&gpio->PUPDR, GPIO_PUPDR_PUPD##number##_Pos, state)

#endif // UTILS_H
