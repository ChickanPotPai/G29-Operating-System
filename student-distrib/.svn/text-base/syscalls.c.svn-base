#include "syscalls.h"

#define ELF_SIZE 		40
#define ELF_MAGIC_0 	0x7f
#define ELF_MAGIC_1		0x45
#define ELF_MAGIC_2		0x4c
#define ELF_MAGIC_3		0x46
#define DOESNT_EXIST	0
#define EXISTS			1
#define KERNEL_MEM_END	0x800000
#define STACK_SIZE		0x2000
#define PAGE_SIZE		0x400000
#define FD_SIZE			16
#define PCB_SIZE		(2 * LINE_BUFFER) + (MAX_FD * FD_SIZE) + 12
#define IMAGE_ADDR		0x8048000
#define MEM_128MB		0x8000000
#define MEM_132MB		0x8400000
#define VIDMEM			0x00B8000
#define VBUFFER1		0x00B9000
#define VBUFFER2		0x00BA000
#define VBUFFER3		0x00BB000
#define VIDMEM_SIZE		4000

#define V_ADDR_0		24
#define V_ADDR_1		25
#define V_ADDR_2		26
#define V_ADDR_3		27
#define ELF_0 			0
#define ELF_1 			1
#define ELF_2 			2
#define ELF_3 			3

#define OPEN_FUNCTION 	0
#define CLOSE_FUNCTION 	1
#define READ_FUNCTION 	2
#define WRITE_FUNCTION 	3

#define UPPER_MASK		0xF0
#define SET 			1
#define CLEAR 			0

#define SHELL2_IDX		1
#define	SHELL3_IDX		2

static uint8_t curr_terminal;				// current running terminal
static int8_t num_progs[MAX_TERMINALS];		// track number of programs in each terminal

static int32_t save_ebp[MAX_PROGS];			// save program ebp, esp
static int32_t save_esp[MAX_PROGS];

static save_regs_t save_prog_regs[MAX_PROGS];

static uint32_t shell_virtual_addr;			// shell program information
static int32_t shell_file_length;
static dentry_t shell_dentry;

static inode_t* inodes;						// pointer to inodes

static file_ops_t terminal_fops;			// set operation tables
static file_ops_t rtc_fops;
static file_ops_t file_fops;

static pcb_t* curr_pcb[MAX_TERMINALS];		// current program pcb

// track of available program spaces
static int pcb_track[MAX_PROGS] = {DOESNT_EXIST, DOESNT_EXIST, DOESNT_EXIST, DOESNT_EXIST, DOESNT_EXIST, DOESNT_EXIST};

// local functions
void new_pcb_init(pcb_t* new_pcb);
void start_shell();
void reload_program();
void relaunch_shell();


/*
*	init_terminals
*	Inputs: none
*	Return: void
*	Function: Called in kernel.c and sets up the first three shells 
*	for multiple terminals.
*/
void init_terminals() {
	uint8_t file_header[ELF_SIZE];	// file header buffer
	uint32_t program_address;		// physical address for program
	pcb_t* pcb_start;
	pcb_t shell2, shell3;

	inode_t* inode_ptr;

	curr_terminal = TERMINAL1;		// initialize cursor, number of progams and current terminal
	move_cursor(OFFSET_X);

	num_progs[0] = 1;
	num_progs[1] = 0;
	num_progs[2] = 0;

	new_pcb_init(&shell2);			// initialize shell pcbs
	new_pcb_init(&shell3);

	// read shell dentry and retrieve file header
	read_dentry_by_name ((uint8_t*) "shell", &shell_dentry);
	read_data(shell_dentry.inode, 0, file_header, ELF_SIZE);
	
	// extract virtual address
	shell_virtual_addr = file_header[V_ADDR_0] + 
					(file_header[V_ADDR_1] << 8) +
					(file_header[V_ADDR_2] << 16) +
					(file_header[V_ADDR_3] << 24);

	// set inodes pointer to boot block
	inodes = (inode_t*) (boot);
	inode_ptr = &(inodes[shell_dentry.inode + 1]);

	// retrieve shell file length
	shell_file_length = inode_ptr->length;

	// set pcb_start ptr to top of program stack and copy
	pcb_start = (pcb_t*) (KERNEL_MEM_END - STACK_SIZE  - (SHELL2_IDX * STACK_SIZE));
	memcpy((void*) pcb_start, &shell2, PCB_SIZE);

	// update pcb_track and set pcb values
	pcb_track[SHELL2_IDX] = EXISTS;
	pcb_start->physical_addr = KERNEL_MEM_END + (SHELL2_IDX * PAGE_SIZE);
	pcb_start->pid = SHELL2_IDX;
	strcpy((int8_t*) pcb_start->name, (int8_t*) "shell");

	// set current pcb of respective terminal
	curr_pcb[TERMINAL2] = pcb_start;

	// page program based on pid and program address (SHELL2_IDX)
	swap_tables(SHELL2_IDX);
	program_address = pcb_start->physical_addr;
	add_page(program_address, SHELL2_IDX);

	// copy shell into memory
	read_data(shell_dentry.inode, 0, (uint8_t*) IMAGE_ADDR, shell_file_length);

	// save esp and ebp
	asm volatile("movl %%esp, %0"
		:"=a"(save_esp[SHELL2_IDX])
		:
		:"cc"
	);

	asm volatile("movl %%ebp, %0"
		:"=a"(save_ebp[SHELL2_IDX])
		:
		:"cc"
	);

	// repeat previous for second shell
	pcb_start = (pcb_t*) (KERNEL_MEM_END - STACK_SIZE  - (SHELL3_IDX * STACK_SIZE));
	memcpy((void*) pcb_start, &shell3, PCB_SIZE);

	pcb_track[SHELL3_IDX] = EXISTS;
	pcb_start->physical_addr = KERNEL_MEM_END + (SHELL3_IDX * PAGE_SIZE);
	pcb_start->pid = SHELL3_IDX;

	curr_pcb[TERMINAL3] = pcb_start;

	swap_tables(SHELL3_IDX);
	program_address = pcb_start->physical_addr;
	add_page(program_address, SHELL3_IDX);

	read_data(shell_dentry.inode, 0, (uint8_t*) IMAGE_ADDR, shell_file_length);

	asm volatile("movl %%esp, %0"
		:"=a"(save_esp[SHELL3_IDX])
		:
		:"cc"
	);

	asm volatile("movl %%ebp, %0"
		:"=a"(save_ebp[SHELL3_IDX])
		:
		:"cc"
	);
}

/*
*	system_execute
*	Inputs: cmd - command from terminal
*	Return: error/success code
*	Function: Executes a program based on command passed
*	in.
*/
int32_t system_execute(const char * cmd) {
	int i, j;
	uint8_t file_header[ELF_SIZE];		// file header buffer
	uint32_t file_length;				// program file length
	uint32_t program_address;			// program physical address
	uint32_t virtual_addr;				// program virutal entry address
	char name[LINE_BUFFER];				// program name buffer
	pcb_t* pcb_start;					// pcb pointers for current and parent pcbs
	pcb_t* curr_parent_ptr;

	dentry_t dentry;
	inode_t* inode_ptr;
	
	pcb_t new_pcb;

	// intialize new pcb for program
	new_pcb_init(&new_pcb);

	// retrieve name
	for (i = 0; i < LINE_BUFFER; i++) {
		if ((cmd[i] == ' ') || (cmd[i] == '\0')) {
			name[i] = '\0';
			break;
		}

		name[i] = cmd[i];
	}

	// skip through extra spaces
	while (cmd[i] == ' ') {
		i++;
	}

	// retrieve arguments
	for (j = i; j < (LINE_BUFFER - i); j++) {
		new_pcb.args[j - i] = cmd[j];
		
		if (cmd[j] == '\0') {break;}
	}

	// retrieve program dentry
	if (read_dentry_by_name ((uint8_t*) name, &dentry) == -1) {return -1;}
	inode_ptr = &(inodes[dentry.inode + 1]);

	// set file length
	file_length = inode_ptr->length;

	// retrieve file header
	read_data(dentry.inode, 0, file_header, ELF_SIZE);

	// check if program header matches magic number
	if (file_header[ELF_0] != ELF_MAGIC_0 || 
		file_header[ELF_1] != ELF_MAGIC_1 ||
		file_header[ELF_2] != ELF_MAGIC_2 ||
		file_header[ELF_3] != ELF_MAGIC_3) {return -1;}
	
	// extract virtual address
	virtual_addr = file_header[V_ADDR_0] + 
					(file_header[V_ADDR_1] << 8) +
					(file_header[V_ADDR_2] << 16) +
					(file_header[V_ADDR_3] << 24);

	// search for open program spot
	for (i = 0; i < MAX_PROGS; i++) {
		if (pcb_track[i] == DOESNT_EXIST) {
			// set pointer to top of program stack and copy new pcb
			pcb_start = (pcb_t*) (KERNEL_MEM_END - STACK_SIZE  - (i * STACK_SIZE));
			memcpy((void*) pcb_start, &new_pcb, PCB_SIZE);

			// update pcb_track and pcb values
			pcb_track[i] = EXISTS;
			pcb_start->physical_addr = KERNEL_MEM_END + (i * PAGE_SIZE);
			pcb_start->pid = i;
			strcpy((int8_t*) pcb_start->name, (int8_t*) name);

			// page program based on pid and physical memory location
			swap_tables(i);
			program_address = KERNEL_MEM_END + i * PAGE_SIZE;
			add_page(program_address, i);

			break;
		} else if (i == (MAX_PROGS - 1) && pcb_track[i] == EXISTS) {
			// return if no program spots available
			printf("max processes reached\n");
			return 0;
		}
	}

	// set parent pointer and current pcb
	pcb_start->parent_ptr = (uint32_t) curr_pcb[curr_terminal];
	curr_pcb[curr_terminal] = pcb_start;

	// save esp and ebp
	asm volatile("movl %%esp, %0"
		:"=a"(save_esp[curr_pcb[curr_terminal]->pid])
		:
		:"cc"
	);

	asm volatile("movl %%ebp, %0"
		:"=a"(save_ebp[curr_pcb[curr_terminal]->pid])
		:
		:"cc"
	);

	// set kernel esp, copy program into memory, and update number of programs running
	(&tss)->esp0 = (uint32_t)curr_pcb[curr_terminal] + STACK_SIZE - 4;
	read_data(dentry.inode, 0, (uint8_t*) IMAGE_ADDR, file_length);
	num_progs[curr_terminal]++;

	// set up stack and iret
	asm volatile(" 			\n\
		movw	%0, %%ax 	\n\
		movw	%%ax, %%ds 	\n\
		pushl	%0 			\n\
		pushl	%1 			\n\
		pushf 				\n\
		popl 	%%eax		\n\
		orl 	$0x200, %%eax   \n\
		pushl 	%%eax		\n\
		pushl 	%2 			\n\
		pushl 	%3 			\n\
		iret				\n\
		return_syscall:		\n\
		"
		:
		: "g" (USER_DS),  "g" (MEM_132MB - 4), "g" (USER_CS), "g" (virtual_addr)
		: "eax", "memory"
	);

	// return from halt: update parent pointer and number of programs
	curr_parent_ptr = (pcb_t*) curr_pcb[curr_terminal]->parent_ptr;
	num_progs[curr_terminal]--;

	// relaunch shell if needed
	if (curr_parent_ptr == NULL)
	{
		relaunch_shell();
		return 0;
	}

	// swap tables back to parent process
	(&tss)->esp0 = (uint32_t)curr_parent_ptr + STACK_SIZE - 4;
	swap_tables(curr_parent_ptr->pid);

	// update current pcb and pcb track
	pcb_track[curr_pcb[curr_terminal]->pid] = DOESNT_EXIST;
	curr_pcb[curr_terminal] = curr_parent_ptr;

	return 0;
}

/*
*	relaunch_shell
*	Inputs: none
*	Return: void
*	Function: Relaunches shell if user calls halt on 
*	base shell.
*/
void relaunch_shell()
{
	// update current pcb and pcb_track
	pcb_track[curr_pcb[curr_terminal]->pid] = DOESNT_EXIST;
	curr_pcb[curr_terminal] = NULL;

	// execute shell
	printf("relaunching\n");
	system_execute("shell");
}

/*
*	system_halt
*	Inputs: status - program status
*	Return: error/success code
*	Function: Halts current program in terminal
*   and jumps back to label in system execute.
*/
int32_t system_halt(uint8_t status) {
	int i;

	// close all current file descriptors
	for (i = 0; i < MAX_FD; i++)
	{
		system_close(i);
	}

	// restore ebp and esp
	asm volatile("			\n\
			movl %0, %%ebp 	\n\
			"
			:
			: "r" (save_ebp[curr_pcb[curr_terminal]->pid])
			: "memory");

	asm volatile("			\n\
			movl %0, %%esp 	\n\
			"
			:
			: "r" (save_esp[curr_pcb[curr_terminal]->pid])
			: "memory");

	// jump back to label in system execute
	asm volatile("jmp return_syscall");

	return 0;
}

/*
*	system_read
*	Inputs: fd - file descriptor number
			buf - buffer to read into
			nbytes - number of bytes to read
*	Return: Number of bytes read.
*	Function: Executes corresponding read function and returns number of bytes read.
*/
int32_t system_read(int32_t fd, void* buf, int32_t nbytes) {
	// error checking
	if (fd == 1) {return -1;}
	if ((fd < 0) || (fd > (MAX_FD - 1))) {return -1;}
	if (curr_pcb[curr_terminal]->file_array[fd].flags == 0) {return -1;}

	return curr_pcb[curr_terminal]->file_array[fd].fops_ptr->read_ptr(fd, buf, nbytes);
}

/*
*	system_write
*	Inputs: fd - file descriptor number
			buf - buffer with data to write
			nbytes - number of bytes to write
*	Return: Number of bytes written.
*	Function: Executes corresponding write function and returns number of bytes written.
*/
int32_t system_write(int32_t fd, const void* buf, int32_t nbytes) {
	// error checking
	if ((fd < 1) || (fd > (MAX_FD - 1))) {return -1;}
	if (curr_pcb[curr_terminal]->file_array[fd].flags == 0) {return -1;}

	return curr_pcb[curr_terminal]->file_array[fd].fops_ptr->write_ptr(fd, buf, nbytes);
}

/*
*	system_open
*	Inputs: filename - name of file to open
*	Return: success/error code
*	Function: Opens file based on type and sets up file descriptor
* 	for newly opened file.
*/
int32_t system_open(const uint8_t* filename) {
	int i;
	int32_t check;

	dentry_t curr_dentry;		// file dentry

	// error checking
	if(strncmp((int8_t*) filename, "", NAME_SIZE) == 0) {return -1;}
	if (read_dentry_by_name(filename, &curr_dentry) == -1) {return -1;}

	// get next available file descriptor number
	for (i = FD_BEGIN; i < MAX_FD; i++) {
		if (curr_pcb[curr_terminal]->file_array[i].flags == 0) {
			break;
		}
	}

	if (i >= MAX_FD) {return -1;}

	// set pcb and run open functions based on file type
	switch(curr_dentry.type) {
		case 0:
			{
				// set rtc file operations table
				curr_pcb[curr_terminal]->file_array[i].fops_ptr = get_rtc_fops();
				curr_pcb[curr_terminal]->file_array[i].inode_ptr = NULL;

				// run rtc open
				check = rtc_fops.open_ptr(filename, curr_pcb[curr_terminal]);
				if (check != 0)
					return -1;
				else {
					curr_pcb[curr_terminal]->file_array[i].flags = 1;
					return i;
				}
			}
		case 1:
			{	
				// set file operations table and initialize file position
				curr_pcb[curr_terminal]->file_array[i].inode_ptr = NULL;
				curr_pcb[curr_terminal]->file_array[i].fops_ptr = get_file_fops();
				curr_pcb[curr_terminal]->file_array[i].file_position = 0;

				// run file system open
				check = file_fops.open_ptr(filename, curr_pcb[curr_terminal]);
				if (check != 0)
					return -1;
				else{
					curr_pcb[curr_terminal]->file_array[i].flags = 1;
					return i;
				}

			}
		case 2:
			{	
				// set file operations table and initialize file position and inode_ptr
				curr_pcb[curr_terminal]->file_array[i].inode_ptr = (inode_t*) (boot + (curr_dentry.inode + 1));
				curr_pcb[curr_terminal]->file_array[i].fops_ptr = get_file_fops();
				curr_pcb[curr_terminal]->file_array[i].file_position = 0;

				// run file system open
				check = file_fops.open_ptr(filename, curr_pcb[curr_terminal]);
				if (check != 0)
					return -1;
				else{
					curr_pcb[curr_terminal]->file_array[i].flags = 1;
					return i;
				}
			}
	}

	return 0;
}

/*
*	system_close
*	Inputs: fd - file descriptor number to close
*	Return: success/error code
*	Function: Runs corresponding close function based on passed in
	file descriptor number.
*/
int32_t system_close(int32_t fd) {
	// error checking
	if ((fd < FD_BEGIN) || (fd > (MAX_FD - 1))) {return -1;}
	if (curr_pcb[curr_terminal]->file_array[fd].flags == 0) {return -1;}

	// run corresponding close function
	if ((uint32_t*) (curr_pcb[curr_terminal]->file_array[fd].fops_ptr->close_ptr) == (uint32_t*) &rtc_close) {
		return rtc_fops.close_ptr(fd, curr_pcb[curr_terminal]);
	} else {
		return file_fops.close_ptr(fd, curr_pcb[curr_terminal]);
	}	
}

/*
*	system_getargs
*	Inputs: buf - buffer for arguments
			nbytes - number of bytes to copy
*	Return: success/error code
*	Function: Retrieves arguments from current pcb and copies
	into passed in buffer.
*/
int32_t system_getargs(uint8_t *buf, int32_t nbytes) {
	if (!buf)
		return -1;

	memset(buf, 0, nbytes);
	memcpy(buf, curr_pcb[curr_terminal]->args, nbytes);
	buf[nbytes] = '\0';

	return 0;
}

/*
*	system_vidmap
*	Inputs: screen_start - pointer to pointer to video memory
*	Return: success/error code
*	Function: Points passed in pointer to video memory.
*/
int32_t system_vidmap(uint8_t** screen_start) {
	// error checking
	if (screen_start == NULL) {return -1;}
	if ((int32_t) screen_start < MEM_128MB || (int32_t) screen_start > MEM_132MB) {return -1;}

	// set pointer to video memory
    *screen_start = (uint8_t *) VIDMEM;
	return 0;
}

/*
*	system_set_handler
*	Function: Extra credit
*/
int32_t system_set_handler(int32_t signum, void* handler_address)
{
	return -1;
}

/*
*	system_sigreturn
*	Function: Extra credit
*/
int32_t system_sigreturn()
{
	return -1;
}

/*
*	switch_terminal
*	Inputs: t_num - terminal number to switch to
			regs - registers for current running program
*	Return: void
*	Function: Swaps video memory, saves old program, and reloads
	the program for new terminal.
*/
void switch_terminal(uint8_t t_num, save_regs_t regs) {
	int i;
	uint8_t old_terminal = curr_terminal;		// save old terminal number

	// check if new terminal and current terminal are the same
	if (t_num == curr_terminal) {return;}

	// save current terminal video memory
	switch(curr_terminal) {
		case TERMINAL1: 
			memcpy((uint8_t*) VBUFFER1, (uint8_t*) VIDMEM, VIDMEM_SIZE);
			break;
		case TERMINAL2: 
			memcpy((uint8_t*) VBUFFER2, (uint8_t*) VIDMEM, VIDMEM_SIZE);
			break;
		case TERMINAL3: 
			memcpy((uint8_t*) VBUFFER3, (uint8_t*) VIDMEM,  VIDMEM_SIZE);
			break;
	}

	// load new terminal video memory
	switch(t_num) {
		case TERMINAL1: 
			memcpy((uint8_t*) VIDMEM, (uint8_t*) VBUFFER1, VIDMEM_SIZE);
			break;
		case TERMINAL2: 
			memcpy((uint8_t*) VIDMEM, (uint8_t*) VBUFFER2, VIDMEM_SIZE);
			break;
		case TERMINAL3: 
			memcpy((uint8_t*) VIDMEM, (uint8_t*) VBUFFER3,  VIDMEM_SIZE);
			break;
	}

	// poke video memory
	for (i = 0; i < (NUM_ROWS * NUM_COLS); i++) {
		*(uint8_t *)(VIDMEM + (i << 1) + 1) = ATTRIB;
	}

	// set new current terminal and save registers
	curr_terminal = t_num;
	save_prog_regs[old_terminal] = regs;

	// block keyboard interrupts
	send_eoi(KEYBOARD_IRQ);

	// load shell if terminal hasn't been visited yet, otherwise reload current program
	if (num_progs[t_num] == 0) {
		start_shell(old_terminal);
	} else {
		reload_program();
	}
}

/*
*	start_shell
*	Inputs: none
*	Return: void
*	Function: Loads first shell in a terminal if the
	terminal has no running programs.
*/
void start_shell() {
	// set kernel esp
	(&tss)->esp0 = (uint32_t)curr_pcb[curr_terminal] + STACK_SIZE - 4;
	
	// renable keyboard interrupts
	enable_irq(KEYBOARD_IRQ);

	// swap tables to shell page table and load program to memory
	swap_tables(curr_terminal);
	read_data(shell_dentry.inode, 0, (uint8_t*) IMAGE_ADDR, shell_file_length);
	num_progs[curr_terminal]++;

	// set up stack and iret to shell
	asm volatile(" 			\n\
		movw	%0, %%ax 	\n\
		movw	%%ax, %%ds 	\n\
		pushl	%0 			\n\
		pushl	%1 			\n\
		pushf 				\n\
		pushl 	%2 			\n\
		pushl 	%3 			\n\
		iret				\n\
		"
		:
		: "g" (USER_DS),  "g" (MEM_132MB - 4), "g" (USER_CS), "g" (shell_virtual_addr)
		: "eax", "memory"
	);
}

/*
*	reload_program
*	Inputs: none
*	Return: void
*	Function: Restores the saved program for current terminal. 
*/
void reload_program() {
	uint32_t file_length;		// program file length

	dentry_t dentry;
	inode_t* inode_ptr;

	// retrieve program dentry
	read_dentry_by_name ((uint8_t*) curr_pcb[curr_terminal]->name, &dentry);
	inode_ptr = &(inodes[dentry.inode + 1]);

	// retrieve file length and set kernel esp
	file_length = inode_ptr->length;
	(&tss)->esp0 = (uint32_t) save_prog_regs[curr_terminal].esp;

	// reenable interrupts
	enable_irq(KEYBOARD_IRQ);

	// swap tables to program page table and load program
	swap_tables(curr_pcb[curr_terminal]->pid);
	read_data(dentry.inode, 0, (uint8_t*) IMAGE_ADDR, file_length);

	// restore registers
	asm volatile("          \n\
		movw 	%0, %%gs 	\n\
		movw 	%1, %%fs 	\n\
		movw 	%2, %%es 	\n\
		movw 	%3, %%ds 	\n\
		"
		:
		: "g"(save_prog_regs[curr_terminal].gs), "g"(save_prog_regs[curr_terminal].fs), "g"(save_prog_regs[curr_terminal].es), "g"(save_prog_regs[curr_terminal].ds)
	);

	asm volatile("         \n\
		movl 	%0, %%esp  \n\
		movl 	%1, %%eax  \n\
		movl 	%2, %%ebx  \n\
		movl 	%3, %%ecx  \n\
		movl	%4, %%edx  \n\
		movl 	%5, %%edi  \n\
		movl 	%6, %%esi  \n\
		"
		:
		: "g"(save_prog_regs[curr_terminal].esp), "g"(save_prog_regs[curr_terminal].eax), "g"(save_prog_regs[curr_terminal].ebx), "g"(save_prog_regs[curr_terminal].ecx), "g"(save_prog_regs[curr_terminal].edx), "g"(save_prog_regs[curr_terminal].edi), "g"(save_prog_regs[curr_terminal].esi)
	);


	// set up stack and iret to program
	asm volatile(" 			\n\
		movw	%0, %%ax 	\n\
		movw	%%ax, %%ds 	\n\
		pushl	%1 			\n\
		pushl	%2 			\n\
		pushl	%3 			\n\
		orl   	$0x200,(%%esp)	\n\
		pushl 	%4 			\n\
		pushl 	%5 			\n\
		movl	%6, %%eax	\n\
		movl	%7, %%ebp 	\n\
		iret				\n\
		"
		:
		: "g" (save_prog_regs[curr_terminal].ds), "g" (save_prog_regs[curr_terminal].ss), "g" (save_prog_regs[curr_terminal].user_esp), "g" (save_prog_regs[curr_terminal].eflags), "g" (save_prog_regs[curr_terminal].cs), "g" (save_prog_regs[curr_terminal].eip), "g" (save_prog_regs[curr_terminal].eax), "g" (save_prog_regs[curr_terminal].ebp)
		: "eax", "memory"
	);

}

/*
*	fops_init
*	Inputs: none
*	Return: void
*	Function: Initializes file operations tables.
*/
void fops_init() {
	terminal_fops.open_ptr = (open_func_ptr)terminal_open;
	terminal_fops.close_ptr = (close_func_ptr)terminal_close;
	terminal_fops.read_ptr = (read_func_ptr) terminal_read;
	terminal_fops.write_ptr = (write_func_ptr) terminal_write;

	rtc_fops.open_ptr = (open_func_ptr)rtc_open;
	rtc_fops.close_ptr = (close_func_ptr)rtc_close;
	rtc_fops.read_ptr = (read_func_ptr)rtc_read;
	rtc_fops.write_ptr = (write_func_ptr)rtc_write;

	file_fops.open_ptr = (open_func_ptr)fs_open;
	file_fops.close_ptr = (close_func_ptr)fs_close;  
	file_fops.read_ptr = (read_func_ptr)fs_read;
	file_fops.write_ptr = (write_func_ptr)fs_write;
}

/*
*	new_pcb_init
*	Inputs: new_pcb - pointer to pcb to initialize
*	Return: void
*	Function: Initializes a new pcb.
*/
void new_pcb_init(pcb_t* new_pcb) {
	int i;

	new_pcb->file_array[0].fops_ptr = (file_ops_t*) &terminal_fops;
	new_pcb->file_array[0].inode_ptr = DOESNT_EXIST;
	new_pcb->file_array[0].file_position = DOESNT_EXIST;
	new_pcb->file_array[0].flags = EXISTS;

	new_pcb->file_array[1].fops_ptr = (file_ops_t*) &terminal_fops;
	new_pcb->file_array[1].inode_ptr = DOESNT_EXIST;
	new_pcb->file_array[1].file_position = DOESNT_EXIST;
	new_pcb->file_array[1].flags = EXISTS;
	
	for(i = 2; i < MAX_FD; i++) {
		new_pcb->file_array[i].fops_ptr = DOESNT_EXIST;
		new_pcb->file_array[i].inode_ptr = DOESNT_EXIST;
		new_pcb->file_array[i].file_position = DOESNT_EXIST;
		new_pcb->file_array[i].flags = DOESNT_EXIST;
	}

	new_pcb->parent_ptr = DOESNT_EXIST;
}

/*
*	get_curr_terminal
*	Inputs: none
*	Return: current terminal number
*	Function: Returns current terminal.
*/
uint8_t get_curr_terminal() {return curr_terminal;}

/*
*	get_curr_pcb
*	Inputs: none
*	Return: current pcb pointer
*	Function: Returns pointer to current pcb.
*/
pcb_t* get_curr_pcb() {return curr_pcb[curr_terminal];}

/*
*	get_terminal_fops
*	Inputs: none
*	Return: terminal fops pointer
*	Function: Returns pointer to terminal file operations table.
*/
file_ops_t* get_terminal_fops() {return &terminal_fops;}

/*
*	get_rtc_fops
*	Inputs: none
*	Return: rtc fops pointer
*	Function: Returns pointer to rtc file operations table.
*/
file_ops_t* get_rtc_fops() {return &rtc_fops;}

/*
*	get_file_fops
*	Inputs: none
*	Return: file fops pointer
*	Function: Returns pointer to file file operations table.
*/
file_ops_t* get_file_fops() {return &file_fops;}
