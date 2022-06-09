#include "timer.h"
#include "utils.h"
#include <stddef.h>

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

void noop(void) {}
#define noop4 noop, noop, noop, noop
#define noop16 noop4, noop4, noop4, noop4
timcb_t TIM_periodic_callbacks[16] = {noop16};
timcb_t TIM_CC1_callbacks[16] = {noop16};
timcb_t TIM_CC2_callbacks[16] = {noop16};

void TIMx_set_periodic_interrupt(volatile TIM_TypeDef* timer, uint32_t index, uint32_t irq_n, uint32_t ms, timcb_t cb) {
    if (cb != NULL) TIM_periodic_callbacks[index] = cb;

    NVIC_SetPriority(irq_n, 15); // Set the priority to a low one
    NVIC_EnableIRQ(irq_n); // Enable the TIM3 interrupt on the NVIC side

    set_mask(&timer->DIER, TIM_DIER_UIE, true); // Enable the TIM3 interrupt on the on the timer side

    setup_clock(timer, 1000, ms, false);
}

void TIMx_set_CC1_callback(volatile TIM_TypeDef* timer, uint32_t index, timcb_t callback) {
    set_bit(&timer->DIER, TIM_DIER_CC1IE_Pos, true);
    TIM_CC1_callbacks[index] = callback;
}

void TIMx_set_CC2_callback(volatile TIM_TypeDef* timer, uint32_t index, timcb_t callback) {
    set_bit(&timer->DIER, TIM_DIER_CC2IE_Pos, true);
    TIM_CC2_callbacks[index] = callback;
}

// Define a handler for every timer
#define TIMx_IRQHandler(x) \
void TIM##x##_IRQHandler(void) { \
    /* Step 1: Identify (verify that UIE is enabled and if so, check UIF) */ \
    if (read_bit(&TIM##x->DIER, TIM_DIER_UIE_Pos) && read_bit(&TIM##x->SR, TIM_SR_UIF_Pos)) { \
        /* Step 2: Acknowledge (set UIF back to 0) */ \
        set_bit(&TIM##x->SR, TIM_SR_UIF_Pos, false); \
        /* Step 3: have fun! */ \
        TIM_periodic_callbacks[x](); \
    } \
    if (read_bit(&TIM##x->DIER, TIM_DIER_CC1IE_Pos) && read_bit(&TIM##x->SR, TIM_SR_CC1IF_Pos)) { \
        set_bit(&TIM##x->SR, TIM_SR_CC1IF_Pos, false); \
        TIM_CC1_callbacks[x](); \
    } \
    if (read_bit(&TIM##x->DIER, TIM_DIER_CC2IE_Pos) && read_bit(&TIM##x->SR, TIM_SR_CC2IF_Pos)) { \
        set_bit(&TIM##x->SR, TIM_SR_CC2IF_Pos, false); \
        TIM_CC2_callbacks[x](); \
    } \
}

TIMx_IRQHandler(1);
TIMx_IRQHandler(2);
TIMx_IRQHandler(3);
TIMx_IRQHandler(4);
TIMx_IRQHandler(5);
TIMx_IRQHandler(9);
TIMx_IRQHandler(10);
TIMx_IRQHandler(11);
