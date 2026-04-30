#include <stdint.h>
void unsigned_int_to_string(uint32_t x, char *res) {
  uint32_t len = 1;
  uint32_t curr = 10;
  while (curr <= x) {
    curr = curr * 10;
    len++;
  };
  uint32_t pos = 0;
  while (pos < len) {
    curr = curr / 10;
    res[pos++] = '0' + x / curr;
    x = x % curr;
  };
  res[len] = 0;
};

uint32_t string_to_unsigned_int(char *string) {
  int i = 0;
  uint32_t res = 0;
  while (string[i] != 0) {
    // Oui on devrait vérifier si le charactère n'est pas un chiffre, mais il
    // faudrait d'abord configurer des exceptions dans le shell
    res = res * 10 + (string[i++] - '0');
  };
  return res;
}
