#include "filesystem.h"
#include "disk.h"
#include "terminal.h"
#include <stdint.h>
#include <stdbool.h>


/* EXT2 filesystem */
/* See https://wiki.osdev.org/Ext2 */


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

// block group descriptor
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
  /* Copie caractère par caractère */
  uint8_t *d = (uint8_t *)dest;
  const uint8_t *s = (const uint8_t *)src;

  for (uint32_t i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return dest;
}

uint32_t find_free_inode(struct bgd_s bgd){
  uint32_t i = 0 ;
  //uint8_t* blocks = (uint8_t*)bgd.block_bitmap ;
  uint8_t* inodes = (uint8_t*)bgd.inode_bitmap ;
  while(inodes[i]== 0xFF){
    i ++ ;
  }
  uint32_t j = 0 ;
  while((*(inodes+i) & (1<<j)) == (1<<j) ){
    j ++ ;
  }
  //if(blocks[i/(superblock.inode])
  *(inodes+i) = *(inodes + i) | (1 << j) ;
  bgd.nb_unallocated_inodes -= 1 ;
  struct inode_s* table_inodes = (struct inode_s*)bgd.inode_table ;
  // ca renvoie une vraie adresse, c'est bien ce qu'on veut ?
  return 8 * i + j + 1 ;

}

void list_dir(struct inode_s dir) {
  // lists_subdirs of a dir
  // ls command
  uint8_t buffer[1024];
  bool finished = false ;
  for(uint32_t i_block = 0 ; !finished &&  i_block< superblock.nb_blocks_group ; i_block++){
    ide_read_sectors(dir.block[i_block] * sect_per_block, sect_per_block,
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
        finished = true ;
        break;
      }
      offset += entry->size;
      entry = (struct entry_s *)(buffer + offset);
    }
    if(finished){
      break;
    }
  }
}

uint32_t inode_by_name(struct inode_s dir, char *name) {
  // gets a subdir with its name
  // returns its id, not the structure 
  // -> if returns 0, then none was found

  uint8_t buffer[1024];
  // reads the block of dir, puts this in buffer
  // remark : buffer becomes the address of the first element
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

uint8_t string_length(char* s){
  uint8_t i = 0 ;
  while(s[i] != '\0'){
    i ++ ;
  }
  return i ;
} 



void mkdir(uint32_t argc, char **argv, struct inode_s working_directory){
  // creates a new directory in the working directory

  // checks input is ok
  if (argc != 2) {
    terminal_writestring("mkdir prend exactement un argument\n");
    return ;
  }
  uint32_t new_node = inode_by_name(working_directory, argv[1]);
  if (new_node) {
    terminal_writestring(argv[1]);
    terminal_writestring(" already exists.\n");
    return;
  }
  char* name = argv[1] ;
  
  // searches for a free place
  uint8_t buffer[1024];
  // reads the block of dir, puts this in buffer
  // remark : buffer becomes the address of the first element

  int i = 0 ;
  bool found = false;
  struct entry_s *entry ;
  while(!found){
    ide_read_sectors(working_directory.block[i] * sect_per_block, sect_per_block,
                   (uint16_t *)buffer);
     entry = (struct entry_s *)buffer;
    uint32_t offset = 0;
    terminal_writestring("block_size : ") ;
    terminal_write_int(block_size);
    while (offset < block_size) {
      terminal_writestring("offset : ") ;
      terminal_write_int(offset);
      terminal_writestring("\nentry->size : ");
      terminal_write_int(entry->size);
      terminal_writestring("\nentry->inode : ");
      terminal_write_int(entry->inode);
      terminal_writestring("\nentry->type : ");
      terminal_write_int(entry->type);
      terminal_writestring("\nentry->name_len : ");
      terminal_write_int(entry->name_len);
      terminal_writestring("\nentry->name :");
      terminal_writestring(entry->name);
      terminal_writestring("\n");
      if (entry->size == 0 || entry->inode == 0) {
        terminal_writestring("found empty room \n");
        found = true ;
        break;
      }
      offset += entry->size;
      entry = (struct entry_s *)(buffer + offset);
    }
    if (offset >= block_size){
      terminal_writestring(" bloc plein\n");
    }
    i ++ ;
  }
  
  
  // memcpy(new_entry.name, name, new_entry.name_len) ;
  // new_entry.type = 2 ;
  // new_entry.size = 8 + new_entry.name_len ; 
  //new_entry.inode = (uint32_t)find_free_inode(bgd) ; 
  uint8_t name_len = string_length(name);
  uint32_t inode = (uint32_t)find_free_inode(bgd);
  terminal_write_int(inode);
  *((uint32_t *) entry) =  inode ; 
  *((uint16_t *) entry + 2) = (uint16_t)name_len + 8 ;
  *((uint8_t *) entry + 6) = name_len ;
  *((uint8_t *) entry + 7) = 2 ;
  memcpy((void*)((uint8_t *) entry + 8), name, name_len);
  bgd.nb_dir ++ ;

  //TODO : add . and .. in the directory


  // bgd.nbdir ++ ; ???????
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
