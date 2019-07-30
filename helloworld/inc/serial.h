#ifndef __SERIAL_H__
#define __SERIAL_H__

void serial_init(void);
void serial_putc(char c);

static inline void
outb (unsigned short port, char data)
{
    asm volatile("outb %0,%1" : : "a" (data), "d" (port));
}

static inline unsigned char
inb (int port)
{
    unsigned char data;
    asm volatile("inb %w1,%0" : "=a" (data) : "d" (port));
    return data;
}

#endif //!__SERIAL_H__