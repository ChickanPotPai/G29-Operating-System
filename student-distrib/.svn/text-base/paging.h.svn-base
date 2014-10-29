#ifndef PAGING_H
#define PAGING_H

#define MB128	134217728
#define MB4		4194304
#define KB4		4096
#define TABLE_SIZE	1024
#define ADDR_SHIFT 12
#define	UPPER_4MB_MASK	0xFFC00000
#define PD_ENTRY_BITS	0x0098BDFF

#include "types.h"
#include "terminal.h"
#include "syscalls.h"

extern void paging_init();
void page_4kb(void * physaddr, void * virtualaddr, int32_t pid);
void page_4mb(void * physaddr, void * virtualaddr, int32_t pid);

typedef struct __attribute__((packed)){
	uint32_t present		: 1;
	uint32_t readwrite		: 1;
	uint32_t usersupervisor	: 1;
	uint32_t write_through	: 1;
	uint32_t cache			: 1;
	uint32_t accessed		: 1;
	uint32_t dirty 			: 1;
	uint32_t reserved		: 1;
	uint32_t global			: 1;
	uint32_t avail1			: 1;
	uint32_t avail2			: 1;
	uint32_t avail3			: 1;
	uint32_t page_addr		: 20;
} page_table_entry_desc;

typedef struct __attribute__((packed)){
	uint32_t present		: 1;
	uint32_t readwrite		: 1;
	uint32_t usersupervisor	: 1;
	uint32_t write_through	: 1;
	uint32_t cache			: 1;
	uint32_t accessed		: 1;
	uint32_t reserved		: 1;
	uint32_t pagesize		: 1;
	uint32_t global			: 1;
	uint32_t avail1			: 1;
	uint32_t avail2			: 1;
	uint32_t avail3			: 1;
	uint32_t page_addr		: 20;
} page_dir_entry_desc;


page_dir_entry_desc page_directory[MAX_PROGS][TABLE_SIZE] __attribute__((aligned(KB4)));
page_table_entry_desc page_table[MAX_PROGS][TABLE_SIZE] __attribute__((aligned(KB4)));

void add_page(uint32_t physical_addr, int32_t pid);
void swap_tables(int32_t pid);

#endif /* PAGING_H */
