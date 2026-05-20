#include "filesystem.h"
#include "terminal.h"
#include <stdint.h>

extern struct inode_s working_directory;

uint32_t exec_program(struct inode_s dir, char *filename) {
  struct file_buffer fb = open_file(dir, filename);
  if (inode_by_id(fb.file).size == 0) {
    terminal_writestring("binaire introuvable\n");
    return -1;
  }

  uint8_t *program_dest = (uint8_t *)0x400000;

  uint8_t chunk[1024];
  uint32_t bytes_read;
  uint32_t total_copied = 0;

  while ((bytes_read = read_file(&fb, chunk)) > 0) {
    memcpy(program_dest + total_copied, chunk, bytes_read);
    total_copied += bytes_read;
  }

  terminal_writestring("Programme chargé memoire.\n");

  void (*entry_point)() = (void (*)())0x400000;

  entry_point();

  return 0;
}
int exec_main(int argc, char **argv) {
  if (argc != 2) {
    terminal_writestring("exec takes exactly one argument");
    return 2;
  };
  exec_program(working_directory, argv[1]);
  return 1;
}
