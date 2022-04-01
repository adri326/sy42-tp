#include <inttypes.h>
#include <stdbool.h>
#include <stm32f4xx.h>
#include <stdio.h>
#include "usart.h"
#include "utils.h"

int main() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Enable the clock for the GPIOA and GPIOC peripherals

    // The ports are 2 × 3, and NOT 1 × 2
    set_mode(GPIOA, 2, MODE_ALTERNATE);
    set_mode(GPIOA, 3, MODE_ALTERNATE);
    set_alternate(GPIOA, 2, 0b0111);
    set_alternate(GPIOA, 3, 0b0111);

    SystemInit();
    USART2_Init(115200);

    while (true) {
        __io_putchar(__io_getchar()); // cat
    }
}
