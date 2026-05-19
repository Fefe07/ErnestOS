#include "pmm.h"

/* Array of bits representing page usage */
uint32_t *bitmap;
uint32_t total_pages;
uint32_t bitmap_size;

void *pmm_alloc_page() {
  for (uint32_t i = 0; i < bitmap_size; i++) {
    if (bitmap[i] != 0xFF) {
      for (int j = 0; j < 32; j++) {
        if ((bitmap[i] & (1 << j)) == 0) {
          bitmap[i] |= (1 << j);
          return (void *)((i * 32 + j) * 4096);
        }
      }
    }
  }
  return 0;
}

void pmm_lock_page(uint32_t addr) {
  uint32_t page_index = addr / 4096;
  bitmap[page_index / 32] |= (1 << (page_index % 32));
}

void pmm_free_page(uint32_t addr) {
  uint32_t page_index = addr / 4096;
  bitmap[page_index / 32] &= ~(1 << (page_index % 32));
}

void pmm_init(multiboot_info_t *mbd) {
  extern uint32_t end;
  bitmap = (uint32_t *)&end;

  uint32_t mem_size_kb = mbd->mem_upper + 1024;
  total_pages = mem_size_kb / 4;
  bitmap_size = total_pages / 32;

  multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)mbd->mmap_addr;

  while ((uint32_t)mmap < mbd->mmap_addr + mbd->mmap_length) {
    if (mmap->type == 1) {
      for (uint32_t addr = mmap->addr_low;
           addr < mmap->addr_low + mmap->len_low; addr += 4096) {
        pmm_free_page(addr);
      }
    } else {
      for (uint32_t addr = mmap->addr_low;
           addr < mmap->addr_low + mmap->len_low; addr += 4096) {
        pmm_lock_page(addr);
      }
    }
    mmap = (multiboot_memory_map_t *)((uint32_t)mmap + mmap->size +
                                      sizeof(mmap->size));
  }

  for (uint32_t addr = 0; addr < (uint32_t)&end; addr += 4096) {
    pmm_lock_page(addr);
  }
}
