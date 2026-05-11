#include "pmm.h"
#include <stdint.h>

typedef struct {
  uint32_t esp;
  uint32_t stack_low;
  uint32_t pid;
  int state;
} pcb_t;

pcb_t process_table[8];
uint32_t current_process = 0;
uint32_t nb_tasks = 0;

void create_task(int slot, void (*entrypoint)()) {
  uint32_t stack_phys = (uint32_t)pmm_alloc_page();
  uint32_t *stack =
      (uint32_t *)(stack_phys + 4096); // On pointe en HAUT de la page

  *(--stack) = (uint32_t)entrypoint;

  *(--stack) = 0;
  *(--stack) = 0;
  *(--stack) = 0;
  *(--stack) = 0;

  process_table[slot].esp = (uint32_t)stack;
  process_table[slot].stack_low = stack_phys;
  process_table[slot].state = 0;
}

extern void switch_to(uint32_t *old_esp, uint32_t new_esp);

void yield() {
  int old = current_process;
  current_process = (current_process + 1) % nb_tasks;

  switch_to(&process_table[old].esp, process_table[current_process].esp);
}
