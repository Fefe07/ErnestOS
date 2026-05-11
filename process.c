#include "process.h"
#include "pmm.h"
#include <stdint.h>

pcb_t *current_process = NULL;

pcb_t *create_task(uint32_t pid, void (*entrypoint)()) {
  pcb_t *task = (pcb_t *)pmm_alloc_page();
  uint32_t stack_phys = (uint32_t)pmm_alloc_page();
  uint32_t *stack = (uint32_t *)(stack_phys + 4096);

  *(--stack) = 0x202;
  *(--stack) = 0x08;
  *(--stack) = (uint32_t)entrypoint;

  *(--stack) = 0;
  *(--stack) = 0;
  *(--stack) = 0;
  *(--stack) = 0;
  *(--stack) = 0;
  *(--stack) = 0;
  *(--stack) = 0;
  *(--stack) = 0;
  *(--stack) = 0;
  *(--stack) = 0;

  *(--stack) = 0x10;

  task->esp = (uint32_t)stack;
  task->stack_low = stack_phys;
  task->pid = pid;
  if (current_process == NULL) {
    task->next = task;
    current_process = task;
  } else {
    task->next = current_process->next;
    current_process->next = task;
  }
  return task;
}

extern void switch_to(uint32_t *old_esp, uint32_t new_esp);

void yield() { asm volatile("int $0x81"); }
