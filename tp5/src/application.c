#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include <utils.h>

#include "application.h"

#if EXERCICE == 1

TaskHandle_t T1_handle = NULL;
void T1_callback(void* arg) {
    bool state = false;
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t frequency = configTICK_RATE_HZ * 2;
    while (true) {
        if (state = !state) {
            GPIOA->BSRR = GPIO_BSRR_BS5;
        } else {
            GPIOA->BSRR = GPIO_BSRR_BR5;
        }
        vTaskDelayUntil(&last_wake, frequency / 2);
    }
}

SemaphoreHandle_t T3_sem;
TaskHandle_t T3_handle = NULL, T3_handle2 = NULL;
void T3_callback(void* arg) {
    while (true) {
        assert(xSemaphoreTake(T3_sem, portMAX_DELAY) == pdTRUE);
        printf("Hello world from T3 n° %d!\r\n", (int)arg);
        xSemaphoreGive(T3_sem);
    }
}

// In 4-byte words
#define STACK_SIZE 512

void start_application() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN; // Enable the clock for the GPIOA and GPIOC peripherals

    set_mode(GPIOA, 5, MODE_WRITE); // write

    set_mode(GPIOC, 13, MODE_READ); // read

    assert(
        xTaskCreate(
            T1_callback,
            "Led 1 on/off",
            STACK_SIZE,
            (void*)1,
            tskIDLE_PRIORITY+5,
            &T1_handle
        ) == pdPASS
    );

    assert(
        xTaskCreate(
            T3_callback,
            "Hello world 1",
            STACK_SIZE,
            (void*)1,
            tskIDLE_PRIORITY+5,
            &T3_handle
        ) == pdPASS
    );

    assert(
        xTaskCreate(
            T3_callback,
            "Hello world 2",
            STACK_SIZE,
            (void*)2,
            tskIDLE_PRIORITY+5,
            &T3_handle2
        ) == pdPASS
    );

    T3_sem = xSemaphoreCreateBinary();
    xSemaphoreGive(T3_sem);

    vTaskStartScheduler();
}

#elif EXERCICE == 2

struct msg {
    uint32_t id;
    uint32_t value;
};
typedef struct msg msg_t;

#define STACK_SIZE 512

TaskHandle_t T0_handle = NULL;
QueueHandle_t queue;
void T0_callback(void* args) {
    msg_t msg;
    while (true) {
        if (xQueueReceive(queue, &msg, 10) == pdTRUE) {
            // We received a message!
            printf("Message: %" PRIu32 ", value: %" PRIu32 "\r\n", msg.id, msg.value);
        }
    }
}

TaskHandle_t T1_handle[3] = {NULL, NULL, NULL};
void T1_callback(void* args) {
    uint32_t id = 0;
    while (true) {
        vTaskDelay((TickType_t)args);
        msg_t msg = {
            .id = id,
            .value = rand()
        };
        id += 1;
        xQueueSend(queue, &msg, portMAX_DELAY);
    }
}

void start_application() {
    queue = xQueueCreate(128, sizeof(msg_t));

    assert(
        xTaskCreate(
            T0_callback,
            "Receiver",
            STACK_SIZE,
            NULL,
            tskIDLE_PRIORITY+6,
            &T0_handle
        ) == pdPASS
    );

    for (size_t n = 0; n < 3; n++) {
        char name[16];
        sprintf(name, "Sender %zu", n);

        assert(
            xTaskCreate(
                T1_callback,
                name,
                STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY+5,
                &T1_handle[n]
            ) == pdPASS
        );
    }

    vTaskStartScheduler();
}

#endif // EXERCICE
