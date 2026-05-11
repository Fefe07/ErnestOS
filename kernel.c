#include "pmm.h"
#include "terminal.h"
#include "utilities.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern void init_gdt();
extern void init_idt();
extern int shell();

void kernel_main(multiboot_info_t *mbd, unsigned int magic) {
  terminal_initialize();
  init_gdt();
  init_idt();
  pmm_init(mbd);

  terminal_writestring("Bienvenue sur ErnestOS.\n");

  shell();
}
