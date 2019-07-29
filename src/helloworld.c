#include "inc/serial.h"

void start(void)
{
    serial_init();

    char s[] = "Hello, World.";
    int i;

    for (i = 0; i < 13; i++)
        serial_putc(s[i]);

    while (1);
}