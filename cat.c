#include "filesystem.h"
#include "terminal.h"

extern struct inode_s working_directory;

int cat_main(int argc, char **argv) {
  if (argc != 2) {
    terminal_writestring("cat takes exactly one argument");
    return 2;
  }
  char buffer[1025];
  uint32_t size;
  struct file_buffer fb = open_file(working_directory, argv[1]);
  while (1) {
    size = read_file(&fb, (uint8_t *)buffer);
    buffer[size] = 0;
    if (size == 0) {
      return 1;
    } else {
      terminal_writestring(buffer);
    }
  }
  return 2;
}
