#include "terminal.h"
#include "utilities.h"
#include <stdint.h>

extern uint32_t timer;

int time_main(int argc, char **argv) {
  char res[256];
  unsigned_int_to_string(timer, res);
  terminal_writestring(res);
  terminal_writestring("\n");
  return 1;
}
