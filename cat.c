#include "filesystem.h"
#include "terminal.h"

extern struct inode_s working_directory;

int cat_main(int argc, char **argv) {
  if (argc != 2) {
    terminal_writestring("cat takes exactly one argument");
    return 2;
  }
  char buffer[1025];
  struct file_buffer fb = open_file(working_directory, argv[1]);
  while (1) {
    read_file(&fb, (uint8_t *)buffer);
    buffer[1024] = 0;
    if (buffer[0] == 0) {
      return 1;
    } else {
      terminal_writestring(buffer);
      terminal_writestring("\n");
    }
  }
  return 2;
}
