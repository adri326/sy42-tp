#include "stm32f4xx.h"
#include <stdbool.h>
#include <stddef.h>
#include "usart.h"
#include "utils.h"

bool usart_locked = false;

void usart_lock() {
    usart_locked = true;
}

void usart_unlock() {
    usart_locked = false;
}

void usart_wait() {
    while (usart_locked);
    usart_locked = true;
}

/**
  * @brief  Init USART2 @ baud,8,1,n
  * @param  baud rate
  * @retval None
  * */
void USART2_Init(uint32_t baud) {
    uint32_t tmp = 0, divmantissa, divfraction, apbclk;

    /* initialisation de l'USART2 : baud,8,1,n */

    /* reset/de-reset USART2 */
    RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;
    /* enable USART2 clk */
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /*-------------- UART parameters configuration --------------- */
    USART2->CR1 &= ~USART_CR1_UE;
    /* USART CR1 Configuration : tx and rx enable; oversampling = 16 */
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE;
    /* USART CR2 Configuration : 1 stop bit*/
    USART2->CR2 = 0;
    /* USART CR3 Configuration : no flow control*/
    USART2->CR3 = 0;
    /* USART BRR Configuration : depend on the bus frequency*/
    /* get APB1 prescaler to determine the USART clock frequency apbclk*/
    tmp = (RCC->CFGR & RCC_CFGR_PPRE1)>>10;
    if (tmp & 4){
        tmp = (tmp & 3) + 1;
        apbclk = SystemCoreClock >> tmp;
    }
    else {
        apbclk = SystemCoreClock;
    }

    /* compute baud rate from the apbclock frequency
     *  baud = apbclk /(8*(2-OVER8)*USARTDIV) */

    tmp = (USART2->CR1 & USART_CR1_OVER8)>>15;
    if (tmp == 0) {
        /* OVER8 = 0, div by 16 */
        divmantissa = (apbclk/baud) >> 4;
        divfraction = (apbclk/baud) & 0xF;
    }
    else {
        /* OVER8 = 0, div by 8 */
        divmantissa = (apbclk/baud) >> 3;
        divfraction = (apbclk/baud) & 3;
    }
    /*USART2->BRR = mantissa and fraction part*/
    USART2->BRR = (divmantissa << 4) | divfraction ;


    /* enable USART */
    USART2->CR1 |= USART_CR1_UE;

    /*-------------- interrupt --------------- */
    //NVIC_SetPriority(USART2_IRQn,0x15); /*  priority */
    //NVIC_EnableIRQ(USART2_IRQn);
}

#define read_bit(target, bit) (((target) >> bit) & 1)

// Declared in syscalls.c
// Writes a single character to the UART TX port
// Returns 1 on success
int __io_putchar(int ch) {
    usart_wait();

    while (!read_bit(USART2->SR, USART_SR_TXE_Pos));
    USART2->DR = ch;

    usart_unlock();

    return 1;
}

// Declared in syscalls.c
// Read a single char from the UART RX port
// Returns -2 on error (overrun, frame error or parity error)
int __io_getchar(void) {
    uint32_t status = USART2->SR;
    while (true) {
        if (
            read_bit(status, USART_SR_ORE_Pos) ||
            read_bit(status, USART_SR_FE_Pos) ||
            read_bit(status, USART_SR_PE_Pos)
        ) return -2;
        if (read_bit(status, USART_SR_RXNE_Pos)) break;
        status = USART2->SR;
    }

    int res = USART2->DR & 0xff;
    return res;
}

// Writes `len` bytes of `data` to the UART TX port. Stops and returns the return value of `__io_putchar` if an error is encountered
int USART2_Transmit(uint8_t* data, uint32_t len) {
    usart_wait();

    uint32_t offset = 0;
    while (offset < len) {
        int res = __io_putchar(data[offset++]);
        if (res != 1) {
            usart_unlock();
            return res;
        }
    }

    usart_unlock();
    return 1;
}

// Reads at most `max` chars from the UART RX port, and writes them into `data`.
// Returns the number of bytes read
uint32_t USART2_Receive(uint8_t* data, uint32_t max) {
    uint32_t offset = 0;
    while (offset < max) {
        int c = __io_getchar();
        if (c < 0) {
            return offset;
        }
        data[offset++] = c;
    }
    return offset;
}

const uint8_t* transmit_queue;
uint32_t transmit_len;
void (*transmit_callback)(void) = NULL;

void USART2_IRQHandler(void) {
    if (read_bit(USART2->CR1, USART_CR1_TXEIE_Pos) && read_bit(USART2->SR, USART_SR_TXE_Pos)) {
        // Ack is done by writing to DR
        if (transmit_len > 0) {
            USART2->DR = *transmit_queue;
            transmit_queue++;
            transmit_len--;
        }
        if (transmit_len == 0) {
            // 4. Set TXEIE to 0 once transmission has ended
            set_bit(&USART2->CR1, USART_CR1_TXEIE_Pos, false);

            // 5. Call callback, if not null
            if (transmit_callback != NULL) transmit_callback();

            usart_unlock();
        }
    }
}

int USART2_Transmit_IRQ(const uint8_t* data, uint32_t len, void (*callback)(void)) {
    // 1. Check the tmp variables
    if (usart_locked) return -1;

    usart_lock();

    transmit_queue = data;
    transmit_len = len;
    transmit_callback = callback;

    // 2. Set TXEIE to 1
    set_bit(&USART2->CR1, USART_CR1_TXEIE_Pos, true);

    // 3. Enable the IRQHandler in NVIC
    NVIC_SetPriority(USART2_IRQn, 15); // Set the priority to a low one
    NVIC_EnableIRQ(USART2_IRQn); // Enable the TIM3 interrupt on the NVIC side

    // USART2->DR = *transmit_queue;
    // transmit_queue++;
    // transmit_len--;

    return 0;
}
