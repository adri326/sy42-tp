#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

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
    NVIC_SetPriorityGrouping(3);

    start_application();
}

void vApplicationIdleHook(void) {

    // no tasks to run go in light sleep until next interrupt
    // __WFI();

}

/**
 * Tick Hook function, called from ISR SysTick
 */
void vApplicationTickHook(void) {
    // noop
}
