#ifndef TERMINAL_H
#define TERMINAL_H

#include "lib.h"
#include "i8259.h"
#include "syscalls.h"

#define KEYBOARD_IRQ		1
#define KEY_BUFFER_PORT		0x60
#define KEY_RELEASED_MASK	0x80

#define LINE_BUFFER			128
#define MAX_TERMINALS		3

position_t start_position[MAX_TERMINALS];
uint32_t cur_buffer_position[MAX_TERMINALS];
uint32_t cur_buffer_size[MAX_TERMINALS];

int8_t key_line_buffer[MAX_TERMINALS][LINE_BUFFER];
int8_t key_read_buffer[MAX_TERMINALS][LINE_BUFFER];

void keyboard_handler();

int32_t terminal_read(int32_t fd, char* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const char* buf, int32_t nbytes);
int32_t terminal_open(const uint8_t* filename, void* curr_pcb);
int32_t terminal_close(int32_t fd, void* curr_pcb);

#endif /* TERMINAL_H */
