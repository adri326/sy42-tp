#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32f4xx.h>
#include <math.h>
#include "usart.h"
#include "utils.h"

// Function that sets up the clock: sets its frequency (in Hz), the ARR register and the one shot mode, then resets and starts the clock
void setup_clock(volatile TIM_TypeDef* tim, uint32_t frequency, uint32_t count, bool one_shot) {
    uint16_t divider = SystemCoreClock / frequency;
    tim->PSC = divider - 1;
    tim->ARR = count - 1;

    set_bit(&tim->CR1, TIM_CR1_OPM_Pos, one_shot); // Disable/enable one-shot mode

    tim->SR = 0; // Reset the status register
    set_bit(&tim->EGR, 0, true); // Reset the counters

    set_bit(&tim->CR1, 0, true); // Enable the clock
}

// Sets up USART
void setup_usart() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    SystemInit();
    USART2_Init(115200);

    set_mode(GPIOA, 2, MODE_ALTERNATE);
    set_mode(GPIOA, 3, MODE_ALTERNATE);
    set_alternate(GPIOA, 2, 0b0111);
    set_alternate(GPIOA, 3, 0b0111);
}


// Exercise 1: blinky; turn a LED on and off every second
void blinky() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Enable the clock for the GPIOA and GPIOC peripherals
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // Setup clock 3 to 10kHz, count up to 10k-1, meaning that UIF is set to 1 every second
    setup_clock(TIM3, 10000, 10000, false);

    // Setup LED
    set_mode(GPIOA, 5, MODE_WRITE); // write

    bool state = false;
    while (true) {
        // UIF flag
        if (read_bit(&TIM3->SR, 0)) {
            state = !state;
            set_bit(&TIM3->SR, 0, false);
        }

        set_mask(&GPIOA->ODR, GPIO_ODR_ODR_5, state);
    }
}

#define EDGE_RISING 0b00
#define EDGE_FALLING 0b01
#define EDGE_BOTH 0b11
#define timer_edge(timer, device, bits) set_bit(&timer->CCER, TIM_CCER_CC##device##NP_Pos, ((bits) & 0b10) != 0); \
    set_bit(&timer->CCER, TIM_CCER_CC##device##P_Pos, ((bits) & 0b01) != 0);

// Exercise 2: measure the time between a button press and a button release
void press_length() {
    // We set up a jumper between PB7 and PC13, so that we can redirect PB7 to the CH1 capture device
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // Enable the GPIOB clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; // Enable the TIM4 clock

    // Set PB7 to AF2
    set_mode(GPIOB, 7, MODE_ALTERNATE);
    set_alternate(GPIOB, 7, 0x2); // AF2

    // Setup the clock on TIM4
    const uint16_t ROLLOVER = 10000;
    const uint16_t FREQUENCY = 10000;
    setup_clock(TIM4, FREQUENCY, ROLLOVER, false);

    // Disable capture
    set_bit(&TIM4->CCER, TIM_CCER_CC1E_Pos, false);
    set_bit(&TIM4->CCER, TIM_CCER_CC2E_Pos, false);

    // Link capture device to the TI2 input
    set_nbit(&TIM4->CCMR1, TIM_CCMR1_CC1S_Pos, 2, 0b10);
    set_nbit(&TIM4->CCMR1, TIM_CCMR1_CC2S_Pos, 2, 0b01);

    timer_edge(TIM4, 1, EDGE_RISING);
    timer_edge(TIM4, 2, EDGE_FALLING);

    // Disable the prescaler and filter
    set_nbit(&TIM4->CCMR1, TIM_CCMR1_IC1PSC_Pos, 2, 0b00);
    set_nbit(&TIM4->CCMR1, TIM_CCMR1_IC1F_Pos, 4, 0b1001); // f_sampling = f_DTS/8, N=8
    set_nbit(&TIM4->CCMR1, TIM_CCMR1_IC2PSC_Pos, 2, 0b00);
    set_nbit(&TIM4->CCMR1, TIM_CCMR1_IC2F_Pos, 4, 0b1001); // f_sampling = f_DTS/8, N=8

    // Enable capture
    set_bit(&TIM4->CCER, TIM_CCER_CC1E_Pos, true);
    set_bit(&TIM4->CCER, TIM_CCER_CC2E_Pos, true);

    uint16_t start = 0;
    uint16_t end = 0;
    uint32_t rollovers = 0;

    bool pressed = false;

    // Note: the button is inverted, so CC2 captures the press and CC1 captures the release

    while (true) {
        if (!pressed && read_bit(&TIM4->SR, TIM_SR_CC2IF_Pos)) {
            pressed = true;
            start = TIM4->CCR2;
            rollovers = 0;
        }

        if (read_bit(&TIM4->SR, TIM_SR_UIF_Pos)) {
            rollovers += 1;
            set_bit(&TIM4->SR, TIM_SR_UIF_Pos, false);
        }

        if (pressed && read_bit(&TIM4->SR, TIM_SR_CC1IF_Pos)) {
            pressed = false;
            end = TIM4->CCR1;

            uint32_t res = ROLLOVER * rollovers + end - start;
            printf("%d.%d ms\r\n", res / 10, res % 10);
        }
    }
}

void set_pwm(volatile uint32_t* reg, float ratio, uint16_t arr) {
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    *reg = (uint16_t)(ratio * (float)arr);
}

// Exercise 3: Control a LED using PWM
void pwm() {
    // When PA5 is on AF01, the channel 1 of TIM2 goes to the led on PA5
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Enable the GPIOA clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable the TIM2 clock

    set_mode(GPIOA, 5, MODE_ALTERNATE);
    set_alternate(GPIOA, 5, 0x1);

    const uint16_t ROLLOVER = 500;
    const uint32_t FREQUENCY = 100000;
    setup_clock(TIM2, FREQUENCY, ROLLOVER, false);

    // Disable compare
    set_bit(&TIM2->CCER, TIM_CCER_CC1E_Pos, false);

    // Set the compare device to PWM mode 1 (0b110) and set its initial value to 0
    set_nbit(&TIM2->CCMR1, TIM_CCMR1_OC1M_Pos, 3, 0b110);
    TIM2->CCR1 = 0;

    // Enable compare
    set_bit(&TIM2->CCER, TIM_CCER_CC1E_Pos, true);

    uint32_t rollovers = 0;

    while (true) {
        if (read_bit(&TIM2->SR, TIM_SR_UIF_Pos)) {
            rollovers += 1;
            set_bit(&TIM2->SR, TIM_SR_UIF_Pos, false);

            float ratio = (float)(rollovers % 100) / 100.0 * 2.0 - 1.0;
            if (ratio < 0.0) ratio = -ratio;

            if (rollovers > 100) rollovers -= 100;

            set_pwm(&TIM2->CCR1, powf(ratio, 2.1), ROLLOVER);
        }
    }
}

int main() {
    setup_usart();

    // blinky();
    press_length();
    // pwm();
}
