void _start() {
  volatile char *vga = (volatile char *)0xB8000;

  vga[0] = 'O';
  vga[1] = 0x0A;
  vga[2] = 'K';
  vga[3] = 0x0A;
  while (1)
    ;
}
