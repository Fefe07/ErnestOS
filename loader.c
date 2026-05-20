#include "filesystem.h"
#include "terminal.h"
#include <stdint.h>

uint32_t exec_program(struct inode_s dir, char *filename) {
  struct file_buffer fb = open_file(dir, filename);
  if (inode_by_id(fb.file).size == 0) {
    terminal_writestring("exec: binaire introuvable\n");
    return -1;
  }

  uint8_t *program_dest = (uint8_t *)0x400000;

  uint8_t chunk[1024];
  uint32_t bytes_read;
  uint32_t total_copied = 0;

  while ((bytes_read = read_file(&fb, chunk)) > 0) {
    memcpy(program_dest + total_copied, chunk, 1024);
    total_copied += bytes_read;
  }

  terminal_writestring("Programme charge avec succes en memoire.\n");

  // 4. Lancer l'exécution
  // On crée un pointeur de fonction qui pointe sur le début de la mémoire du
  // programme
  void (*entry_point)() = (void (*)())0x400000;

  // On saute dans le programme utilisateur !
  entry_point();

  // Ce point ne sera jamais atteint si le programme fait un while(1)
  return 0;
}
