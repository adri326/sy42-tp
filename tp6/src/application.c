#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include <utils.h>
#include "usart.h"

#include "application.h"

#define STACK_SIZE 512

SemaphoreHandle_t EXTI15_10_sem;
void EXTI15_10_IRQHandler() {
    if (read_bit(&EXTI->PR, EXTI_PR_PR13_Pos)) {
        set_bit(&EXTI->PR, EXTI_PR_PR13_Pos, true);

        static BaseType_t higher_priority;

        xSemaphoreGiveFromISR(EXTI15_10_sem, &higher_priority);

        portYIELD_FROM_ISR(higher_priority);
    }
}

TaskHandle_t EXTI15_10_handle = NULL;
void EXTI15_10_callback(void* arg) {
    void (*cb)(void) = arg;

    while (true) {
        assert(xSemaphoreTake(EXTI15_10_sem, portMAX_DELAY) == pdTRUE);
        (cb)();
    }
}

void wait_for_pin13(void (*callback)(void)) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    set_mode(GPIOC, 13, MODE_READ);

    // Set EXTI13 to GPIOC
    set_nbit(&SYSCFG->EXTICR[3], SYSCFG_EXTICR4_EXTI13_Pos, 4, 0b0010);
    // Enable falling trigger for input line 13
    set_bit(&EXTI->FTSR, EXTI_FTSR_TR13_Pos, true);

    // Don't mask input line 13
    set_bit(&EXTI->IMR, EXTI_IMR_MR13_Pos, true);

    NVIC_SetPriority(EXTI15_10_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    EXTI15_10_sem = xSemaphoreCreateBinary();

    assert(
        xTaskCreate(
            EXTI15_10_callback,
            "Pin 13 callback",
            STACK_SIZE,
            (void*)callback,
            tskIDLE_PRIORITY+5,
            &EXTI15_10_handle
        ) == pdPASS
    );
}

#if EXERCICE == 1

bool state = true;
int n_activation = 0;
void pin13_cb(void) {
    printf("Beep, boop! Activation N°%d\r\n", ++n_activation);
}

TaskHandle_t blink_handle = NULL;
void blink_callback(void* arg) {
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t delay = configTICK_RATE_HZ;
    while (true) {
        set_bit(&GPIOA->ODR, 5, state = !state);

        vTaskDelayUntil(&last_wake, delay / 2);
    }
}

int idle_count = 0;
void vApplicationIdleHook(void) {
    idle_count++;
    printf("Idle hook, tick: %lu; count: %d\r\n", (unsigned long)xTaskGetTickCount(), idle_count);
}

extern void pwm();

void start_application() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN; // Enable the clock for the GPIOA and GPIOC peripherals
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    set_mode(GPIOA, 5, MODE_WRITE); // write
    set_bit(&GPIOA->ODR, 5, true);

    printf("Hello, world!\r\n");
    // Uncomment to test deep sleep
    // pwm();
    wait_for_pin13(pin13_cb);

    assert(
        xTaskCreate(
            blink_callback,
            "Blink callback",
            STACK_SIZE,
            (void*)0,
            tskIDLE_PRIORITY+5,
            &blink_handle
        ) == pdPASS
    );

    vTaskStartScheduler();
}

#endif // EXERCICE
