#include "terminal.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void init_gdt();
void init_idt();

void kernel_main(void) {
  terminal_initialize();
  init_gdt();
  init_idt();

  terminal_writestring("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\nl\nm\nn\no\np\nq\nr\ns"
                       "\nt\nu\nv\nw\nx\ny\nz\n");
  terminal_writestring("test\n");
  while (1)
    ;
}
