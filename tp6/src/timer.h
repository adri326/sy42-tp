#ifndef TIMER_H
#define TIMER_H

#include <inttypes.h>
#include <stdbool.h>
#include <stm32f4xx.h>

typedef void (*timcb_t)(void);

// Function that sets up the clock: sets its frequency (in Hz), the ARR register and the one shot mode, then resets and starts the clock
void setup_clock(volatile TIM_TypeDef* tim, uint32_t frequency, uint32_t count, bool one_shot);
// Same as setup_clock, but also enables the clock in the RCC->APB1ENR register
#define setup_clock2(clock, frequency, count, one_shot) RCC->APB1ENR |= RCC_APB1ENR_##clock##EN; setup_clock(clock, frequency, count, one_shot)

// Enables the emission of interrupt events for rollover events, and sets the clock to 1000Hz, with a rollover of `ms`
// Then, a callback is written to memory, which will be called whenever the clock rolls over.
// If NULL, the callback isn't stored.
void TIMx_set_periodic_interrupt(volatile TIM_TypeDef* timer, uint32_t index, uint32_t irq_n, uint32_t ms, timcb_t cb);

// Same as TIMx_set_periodic_interrupt, but also simplifying the syntax and enabling the timer peripheral beforehand
#define TIMx_set_periodic_interrupt2(index, ms, cb) RCC->APB1ENR |= RCC_APB1ENR_TIM##index##EN; \
    TIMx_set_periodic_interrupt(TIM##index, index, TIM##index##_IRQn, ms, cb)

// Enables the emission of an interrupt event for the CC1 device
void TIMx_set_CC1_callback(volatile TIM_TypeDef* timer, uint32_t index, timcb_t callback);

// Enables the emission of an interrupt event for the CC2 device
void TIMx_set_CC2_callback(volatile TIM_TypeDef* timer, uint32_t index, timcb_t callback);

// == Utility macros to call the different TIMx functions ==

#define TIM1_set_periodic_interrupt(ms, cb) TIMx_set_periodic_interrupt2(1, ms, cb)
#define TIM2_set_periodic_interrupt(ms, cb) TIMx_set_periodic_interrupt2(2, ms, cb)
#define TIM3_set_periodic_interrupt(ms, cb) TIMx_set_periodic_interrupt2(3, ms, cb)
#define TIM4_set_periodic_interrupt(ms, cb) TIMx_set_periodic_interrupt2(4, ms, cb)
#define TIM5_set_periodic_interrupt(ms, cb) TIMx_set_periodic_interrupt2(5, ms, cb)

#define TIM1_set_CC1_callback(cb) TIMx_set_CC1_callback(TIM1, 1, cb)
#define TIM2_set_CC1_callback(cb) TIMx_set_CC1_callback(TIM2, 2, cb)
#define TIM3_set_CC1_callback(cb) TIMx_set_CC1_callback(TIM3, 3, cb)
#define TIM4_set_CC1_callback(cb) TIMx_set_CC1_callback(TIM4, 4, cb)
#define TIM5_set_CC1_callback(cb) TIMx_set_CC1_callback(TIM5, 5, cb)

#define TIM1_set_CC2_callback(cb) TIMx_set_CC2_callback(TIM1, 1, cb)
#define TIM2_set_CC2_callback(cb) TIMx_set_CC2_callback(TIM2, 2, cb)
#define TIM3_set_CC2_callback(cb) TIMx_set_CC2_callback(TIM3, 3, cb)
#define TIM4_set_CC2_callback(cb) TIMx_set_CC2_callback(TIM4, 4, cb)
#define TIM5_set_CC2_callback(cb) TIMx_set_CC2_callback(TIM5, 5, cb)

#endif // TIMER_H
