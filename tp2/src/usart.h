#ifndef USART_H
#define USART_H

void USART2_Init(uint32_t baud);

// Declared in syscalls.c
// Writes a single character to the UART TX port
// Returns 1 on success
int __io_putchar(int ch);

// Declared in syscalls.c
// Read a single char from the UART RX port
// Returns -2 on error (overrun, frame error or parity error)
int __io_getchar(void);

// Writes `len` bytes of `data` to the UART TX port. Stops and returns the return value of `__io_putchar` if an error is encountered
int USART2_Transmit(uint8_t* data, uint32_t len);

// Reads at most `max` chars from the UART RX port, and writes them into `data`.
// Returns the number of bytes read
uint32_t USART2_Receive(uint8_t* data, uint32_t max);

#endif // USART_H
