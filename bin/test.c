void _start() {
  volatile int a = 42;
  volatile int b = 58;
  volatile int c = a + b;

  while (1)
    ;
}
