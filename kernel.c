#include "terminal.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void init_gdt();
void init_idt();
int shell();

void kernel_main(void) {
  terminal_initialize();
  init_gdt();
  init_idt();

  terminal_writestring("Bienvenue sur ErnestOS.\n");
  shell();
}
