#include "keyboard_handler.h"
#include "terminal.h"

#define MAX_COMMAND_LEN 256

char shell_buffer[MAX_COMMAND_LEN];
int shell_index = 0;

volatile int continu = 1;
volatile int new = 1;

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s2 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return (*s1 == *s2);
};

void clear_buffer() {
  for (int i = 0; i < MAX_COMMAND_LEN; i++) {
    shell_buffer[i] = 0;
  };
  shell_index = 0;
}

void execute_command(char *command) {
  if (strcmp(command, "exit")) {
    continu = 0;
  } else {
    terminal_writestring("Unknown command : ");
    terminal_writestring(command);
    terminal_writestring("\n");
  };
}

void add_char(char c) {
  if (c == '\n') {
    terminal_putchar(c);
    execute_command(shell_buffer);
    new = 1;
  } else if (shell_index < MAX_COMMAND_LEN - 1) {
    shell_buffer[shell_index++] = c;
    terminal_putchar(c);
  };
}

void input_line() { set_keyboard_handler(add_char); };

int shell() {
  terminal_writestring(
      "Bienvenue sur ErnestShell. Comment ca la seule commande est exit ?\n");
  while (continu) {
    if (new) {
      new = 0;
      clear_buffer();
      terminal_writestring("ErnestOS >> ");
      input_line();
    };
  };
  set_keyboard_handler(0);
  return 1;
}
