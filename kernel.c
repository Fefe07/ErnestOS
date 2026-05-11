#include "pmm.h"
#include "process.h"
#include "terminal.h"
#include "utilities.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern void init_gdt();
extern void init_idt();
extern void shell();
extern void switch_to(uint32_t *old_esp, uint32_t new_esp);
typedef struct {
  uint32_t esp;
  uint32_t stack_low;
  uint32_t pid;
  int state;
} pcb_t;

pcb_t process_table[8];

void kernel_main(multiboot_info_t *mbd, unsigned int magic) {
  terminal_initialize();
  init_gdt();
  init_idt();
  pmm_init(mbd);

  terminal_writestring("Bienvenue sur ErnestOS.\n");

  create_task(0, shell);

  uint32_t dummy_esp;
  switch_to(&dummy_esp, process_table[0].esp);
  while (1)
    ;
}
