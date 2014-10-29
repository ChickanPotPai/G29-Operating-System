/* types.h - Defines to use the familiar explicitly-sized types in this
 * OS (uint32_t, int8_t, etc.).  This is necessary because we don't want
 * to include <stdint.h> when building this OS
 * vim:ts=4 noexpandtab
 */

#ifndef _TYPES_H
#define _TYPES_H

#define NULL 0
#define NAME_SIZE 32
#define RESERVED_DENTRY 24
#define INODE_BLOCKS 1023 // 1024 minus one spot for length of file
#define KB4 4096
#define LINE_BUFFER			128

#ifndef ASM

/* Types defined here just like in <stdint.h> */
typedef int int32_t;
typedef unsigned int uint32_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef char int8_t;
typedef unsigned char uint8_t;

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

typedef struct save_regs
{
	uint16_t gs;
	uint16_t fs;
	uint16_t es;
	uint16_t ds;

    uint32_t edi;
	uint32_t esi;

	uint32_t ebp;
	uint32_t esp;

	uint32_t edx;
	uint32_t ecx;
	uint32_t ebx;
	uint32_t eax;

    uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t user_esp;
	uint32_t ss;
} save_regs_t;

#endif /* ASM */

#endif /* _TYPES_H */

