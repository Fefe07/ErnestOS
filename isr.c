#include "keyboard_handler.h"
#include "terminal.h"
#include "utilities.h"
#include <stdint.h>

uint32_t timer = 0;
uint32_t maj = 0;
uint32_t alt = 0;
unsigned char kbd_map[128] = {
    0,    27,  '&', 0x82, '"',  '\'', '(',  '-',  0x8a, '_', 0x87,
    0x85, ')', '=', '\b', '\t', 'a',  'z',  'e',  'r',  't', 'y',
    'u',  'i', 'o', 'p',  '^',  '$',  '\n', 0,    'q',  's', 'd',
    'f',  'g', 'h', 'j',  'k',  'l',  'm',  0x97, 0xfd, 0,   '*', // 42 : lmaj
    'w',  'x', 'c', 'v',  'b',  'n',  ',',  ';',  ':',  '!', 0,
    '*',  0,   ' ', 0,    0, // 54 : rmaj, 56 : lalt
    0,    0,   0,   0,    0,    0,    0,    0,    0,    0,   0,
    '7',  '8', // 62 : larrow, 66:uparrow, 64 : rarrow, 61:darrow
    '9',  '-', '4', '5',  '6',  '+',  '1',  '2',  '3',  '0', '.',
    0,    0,   '<'};
unsigned char kbd_shift_map[128] = {
    0,    27,  '1',  '2',  '3',  '4',  '5', '6', '7', '8',  '9', '0',
    0xf8, '+', '\b', '\t', 'A',  'Z',  'E', 'R', 'T', 'Y',  'U', 'I',
    'O',  'P', 0xf9, 0x9C, '\n', 0,    'Q', 'S', 'D', 'F',  'G', 'H',
    'J',  'K', 'L',  'M',  '%',  0x9C, 0,   'u', // 0x9C = £ (CP437)
    'W',  'X', 'C',  'V',  'B',  'N',  '?', '.', '/', 0x15, 0,   '*',
    0,    ' ', 0,    0,    0,    0,    0,   0,   0,   0,    0,   0,
    0,    0,   0,    '7',  '8',  '9',  '-', '4', '5', '6',  '+', '1',
    '2',  '3', '0',  '.',  0,    0,    '>'};
unsigned char kbd_altgr_map[128] = {
    0, 0,    0, '~', '#', '{', '[', '|', '`', '\\', '^', '@', ']', '}', 0, 0, 0,
    0, 0xEE, 0, 0,   0, // 0xEE = € (approximatif)
    0, 0,    0, 0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0, 0, 0,
    0, 0,    0, 0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0, 0, 0,
    0, 0,    0, 0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0,   0, 0, 0,
    0, 0,    0, 0,   0,   0,   0,   0,   0,   0,    0,   0,   0,   0};

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

void isr_handler(registers_t regs) {

  if (regs.int_no == 0) { // Zero Division Error
    terminal_writestring("EXCEPTION: Division par zero !\n");
    for (;;)
      ;
  };

  if (regs.int_no == 32) { // Timer
    timer++;
  };

  if (regs.int_no == 33) { // Keyboard
    uint8_t scancode = inb(0x60);
    if (scancode == 0x2A || scancode == 0x36) {
      maj = 1;
    } else if (scancode == 0xAA || scancode == 0xB6) {
      maj = 0;
    } else if (scancode == 0x38) {
      alt = 1;
    } else if (scancode == 0xB8) {
      alt = 0;
    } else {
      // char res[256] = {0};
      // unsigned_int_to_string(scancode, res);
      // terminal_writestring(res);
      // terminal_writestring("\n");
      if (!(scancode & 0x80)) {
        unsigned char *current_table = kbd_map;
        if (maj)
          current_table = kbd_shift_map;
        else if (alt)
          current_table = kbd_altgr_map;
        if (scancode < 128 && current_table[scancode] != 0) {
          char c = current_table[scancode];
          keypress(c);
        };
      };
    }
  };

  if (regs.int_no >= 32) {
    // Si l'interruption vient de l'esclave (IRQ 8-15)
    if (regs.int_no >= 40) {
      outb(0xA0, 0x20);
    };
    // Dans tous les cas, envoyer au maître
    outb(0x20, 0x20);
  };
};

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
  idt[num].isr_low = (base & 0xFFFF);
  idt[num].isr_high = (base >> 16) & 0xFFFF;
  idt[num].kernel_cs = sel;
  idt[num].reserved = 0;
  idt[num].attributes = flags;
};

extern void idt_load(uint32_t);
// declare interruptions
extern void isr0();
extern void isr32();
extern void isr33();

void init_idt() {
  idtr.limit = (sizeof(idt_entry_t) * 256) - 1;
  idtr.base = (uint32_t)&idt;

  // On initialise toute la table à zéro
  // (Utilisez votre propre fonction memset si disponible)

  // On enregistre notre handler pour l'exception 0
  // 0x08 est le segment de code de votre GDT
  // 0x8E : Présent, Ring 0, Interrupt Gate
  idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
  idt_set_gate(32, (uint32_t)isr32, 0x08, 0x8E);
  idt_set_gate(33, (uint32_t)isr33, 0x08, 0x8E);

  // On charge l'IDT
  idt_load((uint32_t)&idtr);
  pic_remap(0x20, 0x28);
  asm volatile("sti");
}
