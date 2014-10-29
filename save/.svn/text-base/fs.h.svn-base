#ifndef FS_H
#define FS_H

#include "lib.h"
#include "types.h"
#include "syscalls.h"
#include "tasks.h"

#define DIRECTORY_ENTRIES 63 // 64 minus spot for #'s and reserved
#define RESERVED_BOOTBLOCK 52

typedef struct boot_block
{
	uint32_t num_directory_entry;
	uint32_t num_inodes;
	uint32_t num_data_block;
	uint8_t reserved[RESERVED_BOOTBLOCK];
	dentry_t directory_entry[DIRECTORY_ENTRIES];
} boot_block_t;

int32_t fs_open(const uint8_t* filename, pcb_t* curr_pcb);
int32_t fs_close(int32_t fd, pcb_t* curr_pcb);
int32_t fs_write(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t fs_read(uint32_t fd, uint8_t* buf, int32_t nbytes);
int32_t fs_read_name(uint8_t* fname, uint8_t* buf, int32_t nbytes);

int32_t read_dir(fd_t* fd, uint8_t* buf);
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
void setup_fs(uint32_t boot);

boot_block_t * boot;
data_block_t * data;


#endif /* FS_H */
