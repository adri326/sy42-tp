#include <inttypes.h>
#include <stdbool.h>
#include <stm32f4xx.h>
#include "utils.h"

// This simple example reads the value of the "user" button on the board and outputs that value to the PC5 led

int main() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN; // Enable the clock for the GPIOA and GPIOC peripherals

    set_mode(GPIOA, 5, MODE_WRITE); // write

    set_mode(GPIOC, 13, MODE_READ); // read

    set_mode(GPIOA, 0, MODE_READ); // read
    set_mode(GPIOA, 1, MODE_WRITE); // write

    set_pupd(GPIOA, 0, PUPD_UP); // pull-up
    set_bit(&GPIOA->OTYPER, GPIO_OTYPER_OT_1, true); // open-drain

    while (true) {
        bool state = !read_bit(&GPIOC->IDR, 13);
        set_mask(&GPIOA->ODR, GPIO_ODR_ODR_5, state);

        bool state2 = read_bit(&GPIOA->IDR, 0);
        set_mask(&GPIOA->ODR, GPIO_ODR_ODR_1, state2);
    }
}
