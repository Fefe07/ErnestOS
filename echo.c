#include "terminal.h"

int echo_main(int argc, char **argv) {
  if (argc != 2) {
    terminal_writestring("echo prend exactement un argument.\n");
  } else {
    terminal_writestring(argv[1]);
    terminal_writestring("\n");
  }
  return 1;
}
