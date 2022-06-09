#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32f4xx.h>
#include <math.h>
#include <string.h>
#include "usart.h"
#include "timer.h"
#include "utils.h"


float led_intensity = 0.0;
bool pressed = false;

void set_pwm(volatile TIM_TypeDef* reg, float ratio) {
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    reg->CCR1 = (uint16_t)(ratio * (float)reg->ARR);
}

void my_tim3_callback(void) {
    led_intensity += pressed ? 0.05 : 0.01;
    if (led_intensity >= 2.0) led_intensity -= 2.0;
    float ratio = led_intensity;
    if (ratio > 1.0) ratio = 2.0 - ratio;

    set_pwm(TIM2, powf(ratio, 2.1));
}

void pwm() {
    TIM3_set_periodic_interrupt(10, &my_tim3_callback);

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN; // Enable the clock for the GPIOA and GPIOC peripherals

    set_mode(GPIOA, 5, MODE_ALTERNATE);
    set_alternate(GPIOA, 5, 0x1);

    const uint16_t ROLLOVER = 500;
    const uint32_t FREQUENCY = 100000;
    setup_clock2(TIM2, FREQUENCY, ROLLOVER, false);

    // Disable compare
    set_bit(&TIM2->CCER, TIM_CCER_CC1E_Pos, false);

    // Set the compare device to PWM mode 1 (0b110) and set its initial value to 0
    set_nbit(&TIM2->CCMR1, TIM_CCMR1_OC1M_Pos, 3, 0b110);
    TIM2->CCR1 = 0;

    // Enable compare
    set_bit(&TIM2->CCER, TIM_CCER_CC1E_Pos, true);
}

uint32_t press_rollovers = 0;
uint32_t press_start = 0;
uint32_t press_end = 0;

void my_tim4_callback(void) {
    if (pressed) {
        press_rollovers += 1;

        char str[64];
        sprintf(str, "You've been pressing for %dms.\r\n", 1000 * press_rollovers - press_start);

        USART2_Transmit_IRQ((uint8_t*)str, strlen(str), NULL);
    }
}

void my_tim4_cc1(void) {
    if (pressed) {
        press_end = TIM4->CCR1;
        pressed = false;

        uint32_t res = 1000 * press_rollovers + press_end - press_start;
        printf("%d ms\r\n", res);
    }
}

void my_tim4_cc2(void) {
    if (!pressed) {
        press_start = TIM4->CCR2;
        pressed = true;
        press_rollovers = 0;
    }
}

void button_press() {
    // We set up a jumper between PB7 and PC13, so that we can redirect PB7 to the CH1 capture device
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // Enable the GPIOB clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; // Enable the TIM4 clock

    // Set PB2 to AF2
    set_mode(GPIOB, 7, MODE_ALTERNATE);
    set_alternate(GPIOB, 7, 0x2); // AF2

    // Setup the clock on TIM4
    const uint16_t ROLLOVER = 1000;
    const uint16_t FREQUENCY = 1000;
    TIM4_set_periodic_interrupt(1000, &my_tim4_callback);

    // Set the callback and enable event emission
    TIM4_set_CC1_callback(&my_tim4_cc1);
    TIM4_set_CC2_callback(&my_tim4_cc2);

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

int main() {
    setup_usart();

    pwm();
    button_press();

    set_bit(&SCB->SCR, SCB_SCR_SLEEPONEXIT_Pos, true);

    __WFI();

    printf("Never reached!\r\n");
}
