// keyboard related declarations

#include "terminal.h"

#define CAPS_LOCK	0x3A
#define CTRL		0x1D
#define LEFT_SHIFT	0x2A
#define RIGHT_SHIFT	0x36
#define ALT			0x38

#define BACKSPACE 	0x0E
#define ENTER 		0x1C
#define SPACE 		0x39
#define F1			0x3B
#define F2			0x3C
#define F3			0x3D

#define L 			0x26

#define OFFSET_X	7
#define START_X		0
#define START_Y		0

static uint8_t shift_pressed = 0;
static uint8_t ctrl_pressed = 0;
static uint8_t alt_pressed = 0;
static uint8_t caps_activated = 0;
static uint8_t buf_offset = 0;

uint8_t keyboard_read = 0;

void keyboard_abstraction(uint8_t curr_terminal, unsigned char charArray[], unsigned scanword, position_t current_position);
void update_and_switch(uint8_t t_num);

void do_enter(uint8_t curr_terminal);
void do_backspace(uint8_t curr_terminal);

void print_line_buffer(uint8_t offset, uint8_t curr_terminal);
void clear_buffer();

unsigned char lowercase_char[64] = {
	0, 0, 
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 
	0, 0, 
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p','[', ']', 
	0, 0,  
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';','\'', '`', 
	0, 
	'\\',	
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
	0, 0, 0,
	' ',
	0, 0, 0, 0, 0, 0
};

unsigned char uppercase_char[64] = {
	0, 0, 
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 
	0, 0, 
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P','{', '}', 
	0, 0,  
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':','\"', '~', 
	0, 
	'|',	
	'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
	0, 0, 0,
	' ',
	0, 0, 0, 0, 0, 0
};

unsigned char caps_char[64] = {
	0, 0, 
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',  
	0, 0, 
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P','[', ']', 
	0, 0,  
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';','\'', '`',  
	0, 
	'\\',	
	'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/',
	0, 0, 0,
	' ',
	0, 0, 0, 0, 0, 0
};

unsigned char caps_shift_char[64] = {
	0, 0, 
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',   
	0, 0, 
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p','{', '}',
	0, 0,  
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':','\"', '~', 
	0, 
	'|',	
	'z', 'x', 'c', 'v', 'b', 'n', 'm','<', '>', '?',
	0, 0, 0,
	' ',
	0, 0, 0, 0, 0, 0
};

unsigned char readChar(){
	return inb(KEY_BUFFER_PORT);
}

void keyboard_handler(){
	uint8_t curr_terminal = get_curr_terminal();
	unsigned char code;
	unsigned char release_code;
	unsigned char release_key;
	
	disable_irq(KEYBOARD_IRQ);
	
	code = readChar();
	release_code = code | KEY_RELEASED_MASK;
	
	position_t current_position = get_cur_position();

	switch (code) {
		case RIGHT_SHIFT:
		case LEFT_SHIFT:
			shift_pressed = 1;
			break;
		case CTRL:
			ctrl_pressed = 1;
			break;
		case CAPS_LOCK:
			if (caps_activated == 0)
				caps_activated = 1;
			else
				caps_activated = 0;
			break;
		case ALT:
			alt_pressed = 1;
			break;
		case BACKSPACE:
			do_backspace(curr_terminal);
			break;
		case ENTER:
			do_enter(curr_terminal);
			break;
	}
	
	if (code == release_code) {
		release_key = code & ~(KEY_RELEASED_MASK);

		if ((release_key == CTRL) && ctrl_pressed)
			ctrl_pressed = 0;
		else if ((release_key == RIGHT_SHIFT) | (release_key == LEFT_SHIFT) && shift_pressed) {
			shift_pressed = 0;
		} else if ((release_key == ALT) && alt_pressed) {
			alt_pressed = 0;
		}
	}

	if (alt_pressed) {
		update_and_switch(code);
		curr_terminal = get_curr_terminal();
	} else if (ctrl_pressed && code == L) {
		clear();
		set_new_position(START_X, START_Y);
		terminal_write(0, "391OS> ", OFFSET_X);

		start_position[curr_terminal].x_coord = OFFSET_X;
		start_position[curr_terminal].y_coord = START_Y;

		print_line_buffer(0, curr_terminal);

		if (buf_offset != 0) {
			start_position[curr_terminal].x_coord = START_X;
			start_position[curr_terminal].y_coord = START_Y + 1;
		}
	} else if (!(cur_buffer_size[curr_terminal] >= LINE_BUFFER - 2)) {
		if (code <= SPACE) {
			if (cur_buffer_size[curr_terminal] == 0)
				start_position[curr_terminal] = current_position;

			if (!shift_pressed && !caps_activated && !ctrl_pressed) {
					keyboard_abstraction(curr_terminal, lowercase_char, code, current_position);
			} else if (shift_pressed && caps_activated) {
					keyboard_abstraction(curr_terminal, caps_shift_char, code, current_position);
			} else if (caps_activated) {
					keyboard_abstraction(curr_terminal, caps_char, code, current_position);
			} else {
					keyboard_abstraction(curr_terminal, uppercase_char, code, current_position);
			}
		}
	}

    send_eoi(KEYBOARD_IRQ);
    enable_irq(KEYBOARD_IRQ);
}

void update_and_switch(uint8_t t_num) {
	switch(t_num) {
		case F1:
			set_new_position(start_position[TERMINAL1].x_coord, start_position[TERMINAL1].y_coord);
			switch_terminal(TERMINAL1);
			break;
		case F2:
			set_new_position(start_position[TERMINAL2].x_coord, start_position[TERMINAL2].y_coord);
			switch_terminal(TERMINAL2);
			break;
		case F3:
			set_new_position(start_position[TERMINAL3].x_coord, start_position[TERMINAL3].y_coord);
			switch_terminal(TERMINAL3);
			break;
	}
}

void keyboard_abstraction(uint8_t curr_terminal, unsigned char charArray[], unsigned scanword, position_t current_position){
	int i;

	if (charArray[scanword] != 0){
		for (i = cur_buffer_size[curr_terminal]; i > cur_buffer_position[curr_terminal]; i--)
			key_line_buffer[curr_terminal][i] = key_line_buffer[curr_terminal][i - 1];

		key_line_buffer[curr_terminal][cur_buffer_position[curr_terminal]] = charArray[scanword];
		cur_buffer_position[curr_terminal]++;
		cur_buffer_size[curr_terminal]++;
		print_line_buffer(buf_offset, curr_terminal);

		if (current_position.x_coord + 1 < NUM_COLS) {
			set_new_position(current_position.x_coord + 1, current_position.y_coord);
		} else {
			start_position[curr_terminal].x_coord = START_X;
			start_position[curr_terminal].y_coord++;
			set_new_position(start_position[curr_terminal].x_coord, start_position[curr_terminal].y_coord);
			buf_offset = NUM_COLS - OFFSET_X;
		}
	}
}

void do_enter(uint8_t curr_terminal) {
	key_line_buffer[curr_terminal][cur_buffer_size[curr_terminal]] = '\n';

	print_line_buffer(buf_offset, curr_terminal);

	position_t cur_position = get_cur_position();
	start_position[get_curr_terminal()] = cur_position;

	strncpy(key_read_buffer[curr_terminal], key_line_buffer[curr_terminal], LINE_BUFFER);
	clear_buffer();

	keyboard_read = 1;
	buf_offset = 0;
}

void do_backspace(uint8_t curr_terminal) {
	if (cur_buffer_position[curr_terminal] > 0) {
		position_t prev_position = get_cur_position();

		cur_buffer_position[curr_terminal]--;
		cur_buffer_size[curr_terminal]--;

		if (cur_buffer_position[curr_terminal] == (NUM_COLS - 1)) {
			start_position[get_curr_terminal()].y_coord--;
			prev_position.y_coord--;
			prev_position.x_coord = NUM_COLS - 1;
			buf_offset = 0;
		}

		int i;
		for (i = cur_buffer_position[curr_terminal]; i <= cur_buffer_size[curr_terminal] + 1; i++) {
			key_line_buffer[curr_terminal][i] = key_line_buffer[curr_terminal][i + 1];
		}

		print_line_buffer(buf_offset, curr_terminal);
		putc(' ');

		if (prev_position.x_coord > 0) {
			set_new_position(prev_position.x_coord - 1, prev_position.y_coord);
		} else {
			set_new_position(NUM_COLS - 1, prev_position.y_coord - 1);
		}
	}
}

void print_line_buffer(uint8_t offset, uint8_t curr_terminal) {
	set_new_position(start_position[get_curr_terminal()].x_coord, start_position[get_curr_terminal()].y_coord);
	puts(key_line_buffer[curr_terminal] + offset);
}

void clear_buffer() {
	int i;
	uint8_t curr_terminal = get_curr_terminal();

	for (i = 0; i < LINE_BUFFER; i++) {
		key_line_buffer[curr_terminal][i] = 0;
	}

	cur_buffer_position[curr_terminal] = 0;
	cur_buffer_size[curr_terminal] = 0;
}

int32_t terminal_read(int32_t fd, char* buf, int32_t nbytes) {
	int32_t bytes_received = 0;
	uint8_t curr_terminal = get_curr_terminal();

	while(keyboard_read == 0) {sti();}

	int32_t i = 0;

	while (key_read_buffer[curr_terminal][i] != 0 && i < nbytes) {
		buf[i] = key_read_buffer[curr_terminal][i];
		bytes_received++;
		i++;
	}
	clear_buffer();

	keyboard_read = 0;
	return bytes_received;
}

int32_t terminal_write(int32_t fd, const char* buf, int32_t nbytes) {
	int i;
	uint8_t curr_terminal = get_curr_terminal();
	int32_t bytes_received = 0;

	if (buf == NULL) {return -1;}

	for (i = 0; i < nbytes; i++) {
		putc(buf[i]);
		start_position[curr_terminal].x_coord += 1;

		if (start_position[curr_terminal].x_coord == NUM_COLS || buf[i] == '\n') {
			start_position[curr_terminal].x_coord = 0;
			start_position[curr_terminal].y_coord += 1;

			if (start_position[curr_terminal].y_coord >= NUM_ROWS) {
				start_position[curr_terminal].y_coord = NUM_ROWS - 1;
			}
		}

		bytes_received++;
	}

	return bytes_received+1;
}

int32_t terminal_open(const uint8_t* filename, void* curr_pcb) {
	return 0;
}

int32_t terminal_close(int32_t fd, void* curr_pcb) {
	return -1;
}
