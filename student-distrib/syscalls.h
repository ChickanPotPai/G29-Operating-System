#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "lib.h"
#include "x86_desc.h"
#include "terminal.h"
#include "rtc.h"
#include "tasks.h"
#include "paging.h"
#include "fs.h"
#include "types.h"

#define TERMINAL1	0
#define TERMINAL2	1
#define TERMINAL3	2

#define OFFSET_X	7
#define START_X		0
#define START_Y		0

void init_terminals();

int32_t system_execute(const char * cmd);
int32_t system_halt(uint8_t status);
int32_t system_read(int32_t fd, void* buf, int32_t nbytes);
int32_t system_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t system_open(const uint8_t* filename);
int32_t system_close(int32_t fd);
int32_t system_getargs(uint8_t *buf, int32_t nbytes);
int32_t system_vidmap(uint8_t** screen_start);
int32_t system_set_handler(int32_t signum, void* handler_address);
int32_t system_sigreturn();

void fops_init();
void switch_terminal(uint8_t t_num, save_regs_t regs);

uint8_t get_curr_terminal();
pcb_t* get_curr_pcb();
file_ops_t* get_terminal_fops();
file_ops_t* get_rtc_fops();
file_ops_t* get_file_fops();

#endif	/* SYSCALLS_H */