#include "paging.h"
#include "x86_desc.h"


/*
*	paging_init
*	Inputs: none
*	Return: void
*	Function called at system boot that turns on paging
*	and sets up the initial pages for the kernel and programs.
*/
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
	int i, j;

	for (j = 0; j < MAX_PROGS; j++)
	{
		for (i = KB4; i < MB4; i += KB4)	// page 0mb-4mb in 4kb pieces
		{
			page_4kb((void*)i, (void*)i, j);
		}

		page_4mb((void*)MB4, (void*)MB4, j);		// page 4mb-8mb as one directory 
	}


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

/*
*	page_4kb
*	Inputs: physical address and virtual address to be mapped, process id
*	Return: void
*	Sets up a page table entry in the pid's page table with
*	default options, mapping the given virtual address to the
*	given physical address
*/
void 
page_4kb(void * physaddr, void * virtualaddr, int32_t pid)
{
	uint32_t table_index;
	table_index = (((uint32_t)virtualaddr % MB4) / KB4);		// find out where in the table to map page

	uint32_t shifted_addr = (uint32_t)physaddr >> ADDR_SHIFT;

	page_table[pid][table_index].page_addr = shifted_addr;		// point this entry to the address
	page_table[pid][table_index].present = 1;					// set options for kernel
	page_table[pid][table_index].readwrite = 1;
	page_table[pid][table_index].usersupervisor = 1;
	page_table[pid][table_index].write_through = 1;
	page_table[pid][table_index].cache = 1;
	page_table[pid][table_index].accessed = 0;
	page_table[pid][table_index].dirty = 0;
	page_table[pid][table_index].reserved = 0;
	page_table[pid][table_index].global = 0;
	page_table[pid][table_index].avail1 = 0;
	page_table[pid][table_index].avail2 = 0;
	page_table[pid][table_index].avail3 = 0;

	uint32_t this_table_address = (uint32_t)&page_table >> ADDR_SHIFT;		// get address for the entry we just changed


	uint32_t directory_index;
	directory_index = ((uint32_t)virtualaddr / MB4);		// find out where in directory to change

	page_directory[pid][directory_index].page_addr = this_table_address;	// point directory to table we changed
	page_directory[pid][directory_index].present = 1;			// set options for kernel
	page_directory[pid][directory_index].readwrite = 1;
	page_directory[pid][directory_index].usersupervisor = 1;
	page_directory[pid][directory_index].write_through = 1;
	page_directory[pid][directory_index].cache = 1;
	page_directory[pid][directory_index].accessed = 0;
	page_directory[pid][directory_index].reserved = 0;
	page_directory[pid][directory_index].pagesize = 0;
	page_directory[pid][directory_index].global = 0;
	page_directory[pid][directory_index].avail1 = 0;
	page_directory[pid][directory_index].avail2 = 0;
	page_directory[pid][directory_index].avail3 = 0;
}

/*
*	page_4mb
*	Inputs: physical address and virtual address to be mapped, process id
*	Return: void
*	Sets up a page directory entry in the pid's page directory with
*	default options, mapping the given virtual address to the
*	given physical address
*/
void
page_4mb(void * physaddr, void * virtualaddr, int32_t pid)
{
	// find out where in directory to change
	uint32_t directory_index;
	directory_index = ((uint32_t)virtualaddr / MB4);

	uint32_t shifted_addr = (uint32_t)physaddr >> ADDR_SHIFT;
	// set options for kernel
	page_directory[pid][directory_index].page_addr = shifted_addr;
	page_directory[pid][directory_index].present = 1;
	page_directory[pid][directory_index].readwrite = 1;
	page_directory[pid][directory_index].usersupervisor = 1;
	page_directory[pid][directory_index].write_through = 1;
	page_directory[pid][directory_index].cache = 1;
	page_directory[pid][directory_index].accessed = 0;
	page_directory[pid][directory_index].reserved = 0;
	page_directory[pid][directory_index].pagesize = 1;
	page_directory[pid][directory_index].global = 0;
	page_directory[pid][directory_index].avail1 = 0;
	page_directory[pid][directory_index].avail2 = 0;
	page_directory[pid][directory_index].avail3 = 0;
}

/*
*	add_page
*	Inputs: physical address of program in memory, process id
*	Return: void
*	Sets up program mapping for a new program. Will map 128mb
*	virtual address to the give physical address, which is expected
*	to be the program's physical memory location.
*	Inputs: none
*/	
void add_page(uint32_t physical_addr, int32_t pid) {
	page_4mb((void*) physical_addr, (void*) MB128, pid);
}

/*
*	swap_tables
*	Inputs: process id
*	Return: void
*	Function to switch cr3 to hold the given program's respective
*	page directory, and flush TLB.
*/
void swap_tables(int32_t pid)
{
	uint32_t page_dir_ptr;
	page_dir_ptr = (uint32_t)&page_directory[pid];

	// point cr3 to page directory
	asm volatile("					\n\
			movl	%0, %%cr3		\n\
			"
			:
			: "r" (page_dir_ptr)
			); 
}
