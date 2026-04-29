typedef void (*keyboard_handler_t)(char);

void set_keyboard_handler(keyboard_handler_t handler);
void keypress(char c);
