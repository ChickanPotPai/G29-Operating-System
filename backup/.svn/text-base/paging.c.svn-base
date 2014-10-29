#include "paging.h"
#include "x86_desc.h"

extern void
paging_init()
{
	// turn on PSE (to allow 4mb pages)
	asm volatile("                  	\n\
			movl    %%cr4, %%eax      		\n\
			orl		$0x00000010, %%eax	\n\
			movl	%%eax, %%cr4			\n\
			"
			:
			:
			: "eax"
			);

	uint32_t page_dir_ptr;
	page_dir_ptr = (uint32_t)&page_directory;

	// point cr3 to page directory
	asm volatile("					\n\
			movl	%0, %%cr3		\n\
			"
			:
			: "r" (page_dir_ptr)
			); 

	int i;

	for (i = KB4; i < MB4; i += KB4)	// page 0mb-4mb in 4kb pieces
	{
		page_4kb((void*)i, (void*)i);
	}

	page_4mb((void*)MB4, (void*)MB4);		// page 4mb-8mb as one directory 



	// enable paging
	asm volatile("                  	\n\
			movl    %%cr0, %%eax      	\n\
			orl		$0x80000000, %%eax	\n\
			movl	%%eax, %%cr0			\n\
			"
			:
			:
			: "eax"
			);
}

void 
page_4kb(void * physaddr, void * virtualaddr)
{
	uint32_t table_index;
	table_index = (((uint32_t)virtualaddr % MB4) / KB4);		// find out where in the table to map page

	uint32_t shifted_addr = (uint32_t)physaddr >> ADDR_SHIFT;

	page_table[table_index].page_addr = shifted_addr;		// point this entry to the address
	page_table[table_index].present = 1;					// set options for kernel
	page_table[table_index].readwrite = 1;
	page_table[table_index].usersupervisor = 1;
	page_table[table_index].write_through = 1;
	page_table[table_index].cache = 1;
	page_table[table_index].accessed = 0;
	page_table[table_index].dirty = 0;
	page_table[table_index].reserved = 0;
	page_table[table_index].global = 0;
	page_table[table_index].avail1 = 0;
	page_table[table_index].avail2 = 0;
	page_table[table_index].avail3 = 0;

	uint32_t this_table_address = (uint32_t)&page_table >> ADDR_SHIFT;		// get address for the entry we just changed


	uint32_t directory_index;
	directory_index = ((uint32_t)virtualaddr / MB4);		// find out where in directory to change

	page_directory[directory_index].page_addr = this_table_address;	// point directory to table we changed
	page_directory[directory_index].present = 1;			// set options for kernel
	page_directory[directory_index].readwrite = 1;
	page_directory[directory_index].usersupervisor = 1;
	page_directory[directory_index].write_through = 1;
	page_directory[directory_index].cache = 1;
	page_directory[directory_index].accessed = 0;
	page_directory[directory_index].reserved = 0;
	page_directory[directory_index].pagesize = 0;
	page_directory[directory_index].global = 0;
	page_directory[directory_index].avail1 = 0;
	page_directory[directory_index].avail2 = 0;
	page_directory[directory_index].avail3 = 0;


}

void
page_4mb(void * physaddr, void * virtualaddr)
{
	uint32_t directory_index;
	directory_index = ((uint32_t)virtualaddr / MB4);

	uint32_t shifted_addr = (uint32_t)physaddr >> ADDR_SHIFT;

	page_directory[directory_index].page_addr = shifted_addr;
	page_directory[directory_index].present = 1;
	page_directory[directory_index].readwrite = 1;
	page_directory[directory_index].usersupervisor = 1;
	page_directory[directory_index].write_through = 1;
	page_directory[directory_index].cache = 1;
	page_directory[directory_index].accessed = 0;
	page_directory[directory_index].reserved = 0;
	page_directory[directory_index].pagesize = 1;
	page_directory[directory_index].global = 0;
	page_directory[directory_index].avail1 = 0;
	page_directory[directory_index].avail2 = 0;
	page_directory[directory_index].avail3 = 0;
}
