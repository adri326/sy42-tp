#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#include <stm32f4xx.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "usart.h"
#include "utils.h"

/*
## 3.2 Priorité et attentes passives

1) Les taches T3 ne tournent plus, en effet
2) La LED devrait clignoter *et* les hello world devraient s'afficher.
Cette hypothèse s'avère fausse: ce qui se passe sûrement est que T2 est appelé juste avant T1, ce qui cause la LED à s'allumer et à s'éteindre
3) Pas facile, mais voilà le résultat
4) Rien ne devrait changer si RTOS gère bien la chose
*/

// L'exercice en cours peut être choisi en modifiant EXERCICE dans application.h
#include "application.h"


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

// Sets up USART
void setup_usart() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // SystemInit();
    USART2_Init(115200);

    set_mode(GPIOA, 2, MODE_ALTERNATE);
    set_mode(GPIOA, 3, MODE_ALTERNATE);
    set_alternate(GPIOA, 2, 0b0111);
    set_alternate(GPIOA, 3, 0b0111);
}

int main() {
    setup_usart();
    NVIC_SetPriorityGrouping(3);

    start_application();
}

// void vApplicationIdleHook(void) {

//     // no tasks to run go in light sleep until next interrupt
//     __WFI();

// }

/**
 * Tick Hook function, called from ISR SysTick
 */
void vApplicationTickHook(void) {
    // noop
}
