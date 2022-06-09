#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "usart.h"

void assert_failed(char* file, uint32_t line) {
    char buffer[256] = "Assertion failed: ";

    strncat(buffer, file, 255);
    strncat(buffer, ":", 255);

    size_t end = strlen(buffer);
    int number[10] = {0,};
    int n = 0;
    while (line > 0) {
        number[n] = line % 10;
        line /= 10;
        n++;
    }
    // Assertion: n is signed
    for (n = 9; n > 0 && number[n] == 0; n--);

    for (; n >= 0; n--) {
        buffer[end] = '0' + number[n];
        end++;
    }
    buffer[end] = 0;

    strncat(buffer, "\r\n", 255);

    USART2_Transmit_IRQ(buffer, strlen(buffer), NULL);
    while (1);
}
