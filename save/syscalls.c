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
#define VIDMEM_SIZE		0x0001000

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

static uint8_t curr_terminal;
static int8_t num_progs[MAX_TERMINALS];

static int32_t save_ebp[MAX_PROGS];
static int32_t save_esp[MAX_PROGS];

static int32_t save_prog_esp[MAX_PROGS];
static int32_t save_prog_vaddr[MAX_PROGS];

static uint32_t shell_virtual_addr;
static int32_t shell_file_length;
static dentry_t shell_dentry;
static inode_t* inodes;

static file_ops_t terminal_fops;
static file_ops_t rtc_fops;
static file_ops_t file_fops;

static pcb_t* curr_pcb[MAX_TERMINALS];
static int pcb_track[MAX_PROGS] = {DOESNT_EXIST, DOESNT_EXIST, DOESNT_EXIST, DOESNT_EXIST, DOESNT_EXIST, DOESNT_EXIST};

void new_pcb_init(pcb_t* new_pcb);
void start_shell(uint8_t old_terminal);
void save_program(uint8_t t_num);
void load_program();
void relaunch_shell();

void init_terminals() {
	uint8_t file_header[ELF_SIZE];
	uint32_t program_address;
	pcb_t* pcb_start;
	pcb_t shell2, shell3;

	inode_t* inode_ptr;

	curr_terminal = TERMINAL1;

	num_progs[0] = 1;
	num_progs[1] = 0;
	num_progs[2] = 0;

	new_pcb_init(&shell2);
	new_pcb_init(&shell3);

	read_dentry_by_name ((uint8_t*) "shell", &shell_dentry);

	read_data(shell_dentry.inode, 0, file_header, ELF_SIZE);
	
	shell_virtual_addr = file_header[V_ADDR_0] + 
					(file_header[V_ADDR_1] << 8) +
					(file_header[V_ADDR_2] << 16) +
					(file_header[V_ADDR_3] << 24);

	inodes = (inode_t*) (boot);
	inode_ptr = &(inodes[shell_dentry.inode + 1]);

	shell_file_length = inode_ptr->length;

	pcb_start = (pcb_t*) (KERNEL_MEM_END - STACK_SIZE  - (SHELL2_IDX * STACK_SIZE));

	memcpy((void*) pcb_start, &shell2, PCB_SIZE);

	pcb_track[SHELL2_IDX] = EXISTS;
	pcb_start->physical_addr = KERNEL_MEM_END + (SHELL2_IDX * PAGE_SIZE);
	pcb_start->pid = SHELL2_IDX;
	strcpy((int8_t*) pcb_start->name, (int8_t*) "shell");

	curr_pcb[TERMINAL2] = pcb_start;

	swap_tables(SHELL2_IDX);
	program_address = KERNEL_MEM_END + SHELL2_IDX * PAGE_SIZE;
	add_page(program_address, SHELL2_IDX);

	read_data(shell_dentry.inode, 0, (uint8_t*) IMAGE_ADDR, shell_file_length);

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

	pcb_start = (pcb_t*) (KERNEL_MEM_END - STACK_SIZE  - (SHELL3_IDX * STACK_SIZE));

	memcpy((void*) pcb_start, &shell3, PCB_SIZE);

	pcb_track[SHELL3_IDX] = EXISTS;
	pcb_start->physical_addr = KERNEL_MEM_END + (SHELL3_IDX * PAGE_SIZE);
	pcb_start->pid = SHELL3_IDX;

	curr_pcb[TERMINAL3] = pcb_start;

	swap_tables(SHELL3_IDX);
	program_address = KERNEL_MEM_END + SHELL3_IDX * PAGE_SIZE;
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

int32_t system_execute(const char * cmd) {
	int i, j;
	uint8_t file_header[ELF_SIZE];
	uint32_t file_length;
	uint32_t program_address;
	uint32_t virtual_addr;
	char name[LINE_BUFFER];
	pcb_t* pcb_start;
	pcb_t* curr_parent_ptr;

	dentry_t dentry;
	inode_t* inode_ptr;
	
	pcb_t new_pcb;

	new_pcb_init(&new_pcb);

	for (i = 0; i < LINE_BUFFER; i++) {
		if ((cmd[i] == ' ') || (cmd[i] == '\0')) {
			name[i] = '\0';
			break;
		}

		name[i] = cmd[i];
	}

	while (cmd[i] == ' ') {
		i++;
	}

	for (j = i; j < (LINE_BUFFER - i); j++) {
		new_pcb.args[j - i] = cmd[j];
		
		if (cmd[j] == '\0') {break;}
	}

	if (read_dentry_by_name ((uint8_t*) name, &dentry) == -1) {return -1;}

	inode_ptr = &(inodes[dentry.inode + 1]);

	file_length = inode_ptr->length;

	read_data(dentry.inode, 0, file_header, ELF_SIZE);

	if (file_header[ELF_0] != ELF_MAGIC_0 || 
		file_header[ELF_1] != ELF_MAGIC_1 ||
		file_header[ELF_2] != ELF_MAGIC_2 ||
		file_header[ELF_3] != ELF_MAGIC_3) {return -1;}
	
	virtual_addr = file_header[V_ADDR_0] + 
					(file_header[V_ADDR_1] << 8) +
					(file_header[V_ADDR_2] << 16) +
					(file_header[V_ADDR_3] << 24);

	for (i = 0; i < MAX_PROGS; i++) {
		if (pcb_track[i] == DOESNT_EXIST) {

			pcb_start = (pcb_t*) (KERNEL_MEM_END - STACK_SIZE  - (i * STACK_SIZE));

			memcpy((void*) pcb_start, &new_pcb, PCB_SIZE);

			pcb_track[i] = EXISTS;
			pcb_start->physical_addr = KERNEL_MEM_END + (i * PAGE_SIZE);
			pcb_start->pid = i;
			strcpy((int8_t*) pcb_start->name, (int8_t*) name);

			swap_tables(i);
			program_address = KERNEL_MEM_END + i * PAGE_SIZE;
			add_page(program_address, i);

			break;
		}
		if (i == 7 && pcb_track[i] == EXISTS)
		{
			printf("max processes reached\n");
			return 0;
		}
	}
	pcb_start->parent_ptr = (uint32_t) curr_pcb[curr_terminal];
	curr_pcb[curr_terminal] = pcb_start;

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

	(&tss)->esp0 = (uint32_t)curr_pcb[curr_terminal] + STACK_SIZE - 4;
	read_data(dentry.inode, 0, (uint8_t*) IMAGE_ADDR, file_length);

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

	curr_parent_ptr = (pcb_t*) curr_pcb[curr_terminal]->parent_ptr;

	if (curr_parent_ptr == NULL)
	{
		relaunch_shell();
		return 0;
	}

	(&tss)->esp0 = (uint32_t)curr_parent_ptr + STACK_SIZE - 4;
	swap_tables(curr_parent_ptr->pid);

	pcb_track[curr_pcb[curr_terminal]->pid] = DOESNT_EXIST;
	curr_pcb[curr_terminal] = curr_parent_ptr;

	return 0;
}

void relaunch_shell()
{
	pcb_track[curr_pcb[curr_terminal]->pid] = DOESNT_EXIST;
	curr_pcb[curr_terminal] = NULL;
	printf("relaunching\n");
	system_execute("shell");
}

int32_t system_halt(uint8_t status) {
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

	asm volatile("jmp return_syscall");

	return 0;
}


int32_t system_read(int32_t fd, void* buf, int32_t nbytes) {
	if (fd == 1) {return -1;}
	if ((fd < 0) || (fd > (MAX_FD - 1))) {return -1;}
	if (curr_pcb[curr_terminal]->file_array[fd].flags == 0) {return -1;}

	return curr_pcb[curr_terminal]->file_array[fd].fops_ptr->read_ptr(fd, buf, nbytes);
}


int32_t system_write(int32_t fd, const void* buf, int32_t nbytes) {
	if ((fd < 1) || (fd > (MAX_FD - 1))) {return -1;}
	if (curr_pcb[curr_terminal]->file_array[fd].flags == 0) {return -1;}

	return curr_pcb[curr_terminal]->file_array[fd].fops_ptr->write_ptr(fd, buf, nbytes);
}

int32_t system_open(const uint8_t* filename) {
	dentry_t curr_dentry;
	int i;

	if(strncmp((int8_t*) filename, "", NAME_SIZE) == 0) {return -1;}
	if (read_dentry_by_name(filename, &curr_dentry) == -1) {return -1;}

	for (i = FD_BEGIN; i < MAX_FD; i++) {
		if (curr_pcb[curr_terminal]->file_array[i].flags == 0) {
			break;
		}
	}

	if (i >= MAX_FD) {return -1;}

	int32_t check;

	switch(curr_dentry.type) {
		case 0:
		{

			curr_pcb[curr_terminal]->file_array[i].fops_ptr = get_rtc_fops();
			curr_pcb[curr_terminal]->file_array[i].inode_ptr = NULL;

			check = rtc_fops.open_ptr(filename, curr_pcb[curr_terminal]);
			if (check != 0)
				return -1;
			else{
				curr_pcb[curr_terminal]->file_array[i].flags = 1;
				return i;
			}
		}
		case 1:
		{
			curr_pcb[curr_terminal]->file_array[i].inode_ptr = NULL;
			curr_pcb[curr_terminal]->file_array[i].fops_ptr = get_file_fops();
			curr_pcb[curr_terminal]->file_array[i].file_position = 0;

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
			curr_pcb[curr_terminal]->file_array[i].inode_ptr = (inode_t*) (boot + (curr_dentry.inode + 1));
			curr_pcb[curr_terminal]->file_array[i].fops_ptr = get_file_fops();
			curr_pcb[curr_terminal]->file_array[i].file_position = 0;

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


int32_t system_close(int32_t fd) {
	if ((fd < FD_BEGIN) || (fd > (MAX_FD - 1))) {return -1;}
	if (curr_pcb[curr_terminal]->file_array[fd].flags == 0) {return -1;}

	if ((uint32_t*) (curr_pcb[curr_terminal]->file_array[fd].fops_ptr->close_ptr) == (uint32_t*) &rtc_close) {
		return rtc_fops.close_ptr(fd, curr_pcb[curr_terminal]);
	} else {
		return file_fops.close_ptr(fd, curr_pcb[curr_terminal]);
	}	
}

int32_t system_getargs(uint8_t *buf, int32_t nbytes) {
	if (!buf)
		return -1;

	memset(buf, 0, nbytes);
	memcpy(buf, curr_pcb[curr_terminal]->args, nbytes);
	buf[nbytes] = '\0';

	return 0;
}

int32_t system_vidmap(uint8_t** screen_start) {
	if (screen_start == NULL) {return -1;}
	if ((int32_t) screen_start < MEM_128MB || (int32_t) screen_start > MEM_132MB) {return -1;}

	*screen_start = (uint8_t*) VIDMEM;
	return 0;
}

int32_t system_set_handler(int32_t signum, void* handler_address)
{
	return -1;
}

int32_t system_sigreturn()
{
	return -1;
}

void switch_terminal(uint8_t t_num) {
	uint8_t old_terminal = curr_terminal;

	if (t_num == curr_terminal) {return;}

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

	curr_terminal = t_num;
	save_program(old_terminal);

	if (num_progs[t_num] == 0) {
		send_eoi(KEYBOARD_IRQ);
		start_shell(old_terminal);
	} else {
		load_program();
	}
}

void start_shell(uint8_t old_terminal) {
	(&tss)->esp0 = (uint32_t)curr_pcb[curr_terminal] + STACK_SIZE - 4;

	enable_irq(KEYBOARD_IRQ);

	swap_tables(curr_terminal);
	read_data(shell_dentry.inode, 0, (uint8_t*) IMAGE_ADDR, shell_file_length);
	num_progs[curr_terminal]++;

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

void save_program(uint8_t t_num) {
	save_prog_esp[t_num] = (&tss)->esp0;

	asm volatile("		\n\
		call get_eip 	\n\
						\n\
		get_eip: 		\n\
		pop 		%0 	\n\
		"
		:
		: "g" (save_prog_vaddr[curr_pcb[t_num]->pid])
	);
}

void load_program() {
	uint32_t file_length;

	dentry_t dentry;
	inode_t* inode_ptr;

	read_dentry_by_name ((uint8_t*) curr_pcb[curr_terminal]->name, &dentry);
	inode_ptr = &(inodes[dentry.inode + 1]);

	file_length = inode_ptr->length;
	(&tss)->esp0 = (uint32_t)curr_pcb[curr_terminal] + STACK_SIZE - 4;

	swap_tables(curr_terminal);
	read_data(dentry.inode, 0, (uint8_t*) IMAGE_ADDR, file_length);
	num_progs[curr_terminal]++;

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
		: "g" (USER_DS),  "g" (MEM_132MB - 4), "g" (USER_CS), "g" (save_prog_vaddr[curr_terminal])
		: "eax", "memory"
	);
}

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

uint8_t get_curr_terminal() {return curr_terminal;}
pcb_t* get_curr_pcb() {return curr_pcb[curr_terminal];}
file_ops_t* get_terminal_fops() {return &terminal_fops;}
file_ops_t* get_rtc_fops() {return &rtc_fops;}
file_ops_t* get_file_fops() {return &file_fops;}
