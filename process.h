#include <stdint.h>

#define NULL ((void *)0)

void yield();
typedef struct pcb {
  uint32_t esp;
  uint32_t stack_low;
  uint32_t pid;
  struct pcb *next;
} pcb_t;
pcb_t *create_task(uint32_t pid, void (*entrypoint)());
