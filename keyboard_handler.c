
typedef void (*keyboard_handler_t)(char);

keyboard_handler_t current_keyboard_handler = 0;

void set_keyboard_handler(keyboard_handler_t handler) {
  current_keyboard_handler = handler;
};

void keypress(char c) { current_keyboard_handler(c); }
