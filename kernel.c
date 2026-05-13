#include "disk.h"
#include "filesystem.h"
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

void kernel_main(multiboot_info_t *mbd, unsigned int magic) {
  terminal_initialize();
  init_gdt();
  init_idt();
  pmm_init(mbd);
  init_filesystem();

  terminal_writestring("Bienvenue sur ErnestOS.\n");

  pcb_t *current_process = create_task(0, shell);

  asm volatile("mov %0, %%esp \n"
               "jmp isr_restore_context"
               :
               : "r"(current_process->esp));
  while (1)
    ;
}
