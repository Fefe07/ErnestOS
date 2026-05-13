#include <stdint.h>
void terminal_initialize();
void terminal_writestring(const char *data);
void terminal_putchar(char c);
void terminal_clear();
void terminal_write_int(uint32_t);
