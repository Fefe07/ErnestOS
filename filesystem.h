#include <stdint.h>
void init_filesystem();
struct inode_s {
  uint16_t type_perm;
  uint16_t uid;
  uint32_t size;
  uint32_t access_time;
  uint32_t creation_time;
  uint32_t modification_time;
  uint32_t deletion_time;
  uint16_t gid;
  uint16_t links_count;
  uint32_t blocks;
  uint32_t flags;
  uint32_t oss1;
  uint32_t block[15];
  uint32_t generation;
  uint32_t file_acl;
  uint32_t dir_acl;
  uint32_t frag_addr;
  uint32_t oss2[3];
} __attribute__((packed));
void list_dir(struct inode_s dir);
uint32_t inode_by_name(struct inode_s dir, char *name);
struct inode_s inode_by_id(uint32_t id);
void mkdir(uint32_t argc, char* argv[], struct inode_s working_directory);
