#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error                                                                         \
    "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

/* Hardware text mode color constants. */
enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15,
};

struct gdt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

extern void gdt_flush(struct gdt_ptr *);

typedef struct {
  uint16_t isr_low;   // The lower 16 bits of the ISR's address
  uint16_t kernel_cs; // The GDT segment selector that the CPU will load into CS
                      // before calling the ISR
  uint8_t reserved;   // Set to zero
  uint8_t attributes; // Type and attributes; see the IDT page
  uint16_t isr_high;  // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

typedef struct {
  uint32_t ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

typedef struct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) idtr_t;

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
  return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}

size_t strlen(const char *str) {
  size_t len = 0;
  while (str[len])
    len++;
  return len;
}

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t *terminal_buffer = (uint16_t *)VGA_MEMORY;

void terminal_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_MAGENTA);

  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
  }
}

void terminal_setcolor(uint8_t color) { terminal_color = color; }

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
  size_t index;
  size_t index_next;
  if (terminal_row == VGA_HEIGHT) {
    terminal_row--;
    for (int i = 0; i < VGA_HEIGHT - 1; i++) {
      for (int j = 0; j < VGA_WIDTH; j++) {
        index = i * VGA_WIDTH + j;
        index_next = (i + 1) * VGA_WIDTH + j;
        terminal_buffer[index] = terminal_buffer[index_next];
      };
    };
    for (int j = 0; j < VGA_WIDTH; j++) {
      terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + j] =
          vga_entry(' ', terminal_color);
    };
  }

  if (c == '\n') {
    terminal_column = 0;
    terminal_row++;
  } else {
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
      terminal_column = 0;
      terminal_row++;
    }
  }
}

void terminal_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++)
    terminal_putchar(data[i]);
}

void terminal_writestring(const char *data) {
  terminal_write(data, strlen(data));
}

struct gdt_entry gdt[3];
struct gdt_ptr gp;

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access,
                  uint8_t gran) {
  gdt[num].base_low = (base & 0xFFFF);
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;
  gdt[num].limit_low = (limit & 0xFFFF);
  gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
  gdt[num].access = access;
}

void init_gdt() {
  gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
  gp.base = (uint32_t)&gdt;

  // Null descriptor
  gdt_set_gate(0, 0, 0, 0, 0);

  // Kernel Code Segment
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

  // Kernel Data Segment
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

  // Appel de la fonction assembleur pour charger la GDT
  gdt_flush(&gp);
}

void outb(uint16_t port, uint8_t val) {
  asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

void pic_remap(int offset1, int offset2) {
  uint8_t a1, a2;

  // Sauvegarder les masques actuels
  a1 = inb(0x21);
  a2 = inb(0xA1);

  // Initialisation en mode cascade
  outb(0x20, 0x11);
  outb(0xA0, 0x11);

  // ICW2 : Les vecteurs d'interruption (Offset)
  outb(0x21, offset1); // Master PIC -> 0x20 (32)
  outb(0xA1, offset2); // Slave PIC  -> 0x28 (40)

  // ICW3 : Configuration de la cascade
  outb(0x21, 0x04); // Le maître a un esclave sur l'IRQ2
  outb(0xA1, 0x02); // L'esclave est connecté à l'IRQ2 du maître

  // ICW4 : Mode 8086
  outb(0x21, 0x01);
  outb(0xA1, 0x01);

  // Restaurer les masques (ou mettre 0 pour tout activer)
  outb(0x21, a1);
  outb(0xA1, a2);
}
__attribute__((aligned(0x10))) static idt_entry_t idt[256];
static idtr_t idtr;
extern void idt_load(uint32_t);
extern void isr0();

void isr_handler(registers_t regs) {
  if (regs.int_no == 0) {
    terminal_writestring("EXCEPTION: Division par zero !\n");
    for (;;)
      ;
  };

  if (regs.int_no == 32) {
    terminal_writestring("a");
  }

  if (regs.int_no >= 32) {
    // Si l'interruption vient de l'esclave (IRQ 8-15)
    if (regs.int_no >= 40) {
      outb(0xA0, 0x20);
    }
    // Dans tous les cas, envoyer au maître
    outb(0x20, 0x20);
  }
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
  idt[num].isr_low = (base & 0xFFFF);
  idt[num].isr_high = (base >> 16) & 0xFFFF;
  idt[num].kernel_cs = sel;
  idt[num].reserved = 0;
  idt[num].attributes = flags;
}
extern void irq0(); // Déclaration de la fonction ASM
void init_idt() {
  idtr.limit = (sizeof(idt_entry_t) * 256) - 1;
  idtr.base = (uint32_t)&idt;

  // On initialise toute la table à zéro
  // (Utilisez votre propre fonction memset si disponible)

  // On enregistre notre handler pour l'exception 0
  // 0x08 est le segment de code de votre GDT
  // 0x8E : Présent, Ring 0, Interrupt Gate
  idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
  idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);

  // On charge l'IDT
  idt_load((uint32_t)&idtr);
}

void kernel_main(void) {
  /* Initialize terminal interface */
  terminal_initialize();
  init_gdt();
  init_idt();
  pic_remap(0x20, 0x28);
  asm volatile("sti");

  terminal_writestring("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\nl\nm\nn\no\np\nq\nr\ns"
                       "\nt\nu\nv\nw\nx\ny\nz\n");
  terminal_writestring("test\n");
  while (1)
    ;
}
