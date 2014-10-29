#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_IRQ		1
#define KEY_BUFFER_PORT		0x60
#define KEY_RELEASED_MASK	0x80

#define LINE_BUFFER			128

uint8_t keyboard_read;

position_t start_position;
uint32_t cur_buffer_position;
uint32_t cur_buffer_size;

int8_t key_line_buffer[LINE_BUFFER];

int8_t key_read_buffer[LINE_BUFFER];

void keyboard_handler();

void do_enter();
void do_backspace();

void print_line_buffer();
void clear_buffer();

int32_t read_keyboard(int32_t fd, char* buf, int32_t nbytes);
int32_t write_keyboard(int32_t fd, const char* buf, int32_t nbytes);

#endif /* KEYBOARD_H */
