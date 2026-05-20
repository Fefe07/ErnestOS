#include "filesystem.h"
#include "disk.h"
#include "terminal.h"

struct superblock_s {
  uint32_t nb_inode;
  uint32_t nb_blocks;
  uint32_t nb_reserved_blocks;
  uint32_t nb_unallocated_blocks;
  uint32_t nb_unallocated_inodes;
  uint32_t superblock_index;
  uint32_t block_size;
  uint32_t fragment_size;
  uint32_t nb_blocks_group;
  uint32_t nb_frag_group;
  uint32_t nb_inodes_group;
  uint32_t mount_time;
  uint32_t written_time;
  uint16_t mount_count;
  uint16_t allowed_nb_mounts;
  uint16_t magic;
  uint16_t state;
  uint16_t errors;
  uint16_t version_low;
  uint32_t check_time;
  uint32_t check_interval;
  uint32_t os;
  uint32_t version_high;
  uint16_t user_id;
  uint16_t group_id;
  uint32_t first_ino;
  uint16_t inode_size;
  uint16_t block_group;
  uint32_t opt_feat;
  uint32_t req_feat;
  uint32_t write_req_feat;
  uint8_t id[16];
  uint8_t volume_name[16];
  uint8_t path_last_mount[64];
  uint32_t compression;
  uint8_t nb_blocks_preallocate_file;
  uint8_t nb_blocks_preallocate_dir;
} __attribute__((packed));

struct bgd_s {
  uint32_t block_bitmap;
  uint32_t inode_bitmap;
  uint32_t inode_table;
  uint16_t nb_unallocated_blocks;
  uint16_t nb_unallocated_inodes;
  uint16_t nb_dir;
  uint16_t pad;
  uint32_t reserved[3];
} __attribute__((packed));

struct entry_s {
  uint32_t inode;
  uint16_t size;
  uint8_t name_len;
  uint8_t type;
  char name[];
} __attribute__((packed));

struct superblock_s superblock;
uint32_t block_size;
uint32_t sect_per_block;
struct bgd_s bgd;
struct inode_s root;

void *memcpy(void *dest, const void *src, uint32_t n) {
  uint8_t *d = (uint8_t *)dest;
  const uint8_t *s = (const uint8_t *)src;

  for (uint32_t i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return dest;
}

void list_dir(struct inode_s dir) {
  // lists_subdirs of a dir

  uint8_t buffer[1024];
  ide_read_sectors(dir.block[0] * sect_per_block, sect_per_block,
                   (uint16_t *)buffer);
  struct entry_s *entry = (struct entry_s *)buffer;
  uint32_t offset = 0;
  while (offset < block_size) {
    if (entry->inode != 0) {
      for (uint32_t i = 0; i < entry->name_len; i++) {
        terminal_putchar(*(entry->name + i));
      }
      terminal_putchar('\n');
    }
    if (entry->size == 0) {
      break;
    }
    offset += entry->size;
    entry = (struct entry_s *)(buffer + offset);
  }
}

uint32_t inode_by_name(struct inode_s dir, char *name) {
  // gets a subdir with its name

  uint8_t buffer[1024];
  ide_read_sectors(dir.block[0] * sect_per_block, sect_per_block,
                   (uint16_t *)buffer);
  struct entry_s *entry = (struct entry_s *)buffer;
  uint32_t offset = 0;
  while (entry->inode != 0 && offset < block_size) {
    uint32_t same = 1;
    for (uint32_t i = 0; i < entry->name_len; i++) {
      if (entry->name[i] != name[i] || name[i] == 0) {
        same = 0;
        break;
      }
    }
    if (same && !name[entry->name_len]) {
      return entry->inode;
    }
    offset += entry->size;
    entry = (struct entry_s *)(buffer + offset);
  }
  return 0;
}

struct bgd_s get_bgd(uint32_t group_index) {
  struct bgd_s target_bgd;
  uint32_t bgdt_start_block = (block_size == 1024) ? 2 : 1;
  uint32_t byte_offset = group_index * sizeof(struct bgd_s);
  uint32_t block_to_read = bgdt_start_block + (byte_offset / block_size);
  uint32_t offset_in_block = byte_offset % block_size;

  uint8_t buffer[1024];
  ide_read_sectors(block_to_read * sect_per_block, sect_per_block,
                   (uint16_t *)buffer);

  memcpy(&target_bgd, buffer + offset_in_block, sizeof(struct bgd_s));

  return target_bgd;
}

struct inode_s inode_by_id(uint32_t id) {
  uint32_t group = (id - 1) / superblock.nb_inodes_group;
  uint32_t index = (id - 1) % superblock.nb_inodes_group;
  struct bgd_s bgd = get_bgd(group);
  uint8_t buffer[1024];
  ide_read_sectors(bgd.inode_table * sect_per_block, sect_per_block,
                   (uint16_t *)buffer);
  struct inode_s res;
  memcpy(&res, buffer + index * superblock.inode_size, sizeof(struct inode_s));
  return res;
}

void init_filesystem() {
  uint8_t buffer[1024];
  ide_read_sectors(2, 2, (uint16_t *)buffer);
  memcpy(&superblock, buffer, sizeof(struct superblock_s));
  if (superblock.magic != 0xef53) {
    terminal_writestring("Error: not an ext2 device\n");
    return;
  };
  block_size = 1024 << superblock.block_size;
  sect_per_block = block_size / 512;
  if (block_size != 1024) {
    return;
  }
  ide_read_sectors((superblock.superblock_index + 1) * sect_per_block,
                   sect_per_block, (uint16_t *)buffer);
  memcpy(&bgd, buffer, sizeof(struct bgd_s));
  ide_read_sectors(bgd.inode_table * sect_per_block, sect_per_block,
                   (uint16_t *)buffer);
  memcpy(&root, buffer + superblock.inode_size, sizeof(struct inode_s));

  if ((root.type_perm & 0x4000) != 0x4000) {
    terminal_writestring("Error: Inode 2 is not a directory\n");
  }
}
