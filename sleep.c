#include "terminal.h"
#include "utilities.h"
#include <stdint.h>
extern volatile uint32_t timer;

int sleep_main(int argc, char **argv) {
  // on pourrait vérifier le nombre d'arguments
  volatile uint32_t start = timer;
  uint32_t duration = string_to_unsigned_int(argv[1]);
  while ((timer - start) < duration) {
    asm volatile("sti"); // On force l'ouverture des vannes
  };
  return 1;
}
