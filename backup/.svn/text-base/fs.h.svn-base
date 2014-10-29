#ifndef FS_H
#define FS_H

#include "types.h"

#define NAME_SIZE 32
#define RESERVED_DENTRY 24
#define INODE_BLOCKS 1023 // 1024 minus one spot for length of file
#define DIRECTORY_ENTRIES 63 // 64 minus spot for #'s and reserved
#define KB4 4096
#define RESERVED_BOOTBLOCK 52

typedef struct dentry {
	uint8_t name[NAME_SIZE];
	uint32_t type;
	uint32_t inode;
	uint8_t reserved[RESERVED_DENTRY];
} dentry_t;

typedef struct inode {
	uint32_t length;
	uint32_t blocks[INODE_BLOCKS];
} inode_t;

typedef struct data_block
{
	uint8_t data[KB4];
} data_block_t;


typedef struct boot_block
{
	uint32_t num_directory_entry;
	uint32_t num_inodes;
	uint32_t num_data_block;
	uint8_t reserved[RESERVED_BOOTBLOCK];
	dentry_t directory_entry[DIRECTORY_ENTRIES];
} boot_block_t;

int32_t fs_open();
int32_t fs_close();
int32_t fs_write(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);




#endif /* FS_H */
