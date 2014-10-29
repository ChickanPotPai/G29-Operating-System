//http://pdos.csail.mit.edu/6.828/2012/xv6.html


/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask = 0xFB; /* IRQs 0-7 */
uint8_t slave_mask = 0xFF; /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void 
i8259_init(void)
{
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW1, SLAVE_8259_PORT);

	outb(ICW2_MASTER, MASTER_8259_PORT + 1);
	outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);

	outb(ICW3_MASTER, MASTER_8259_PORT + 1);
	outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);

	outb(ICW4, MASTER_8259_PORT + 1);
	outb(ICW4, SLAVE_8259_PORT + 1);

	outb(master_mask, MASTER_8259_PORT + 1);
	outb(slave_mask, SLAVE_8259_PORT + 1);
}

/* Enable (unmask) the specified IRQ */
void
enable_irq(uint32_t irq_num)
{
	uint8_t data;
	uint16_t PICport;

	if (irq_num < 8) {
		PICport = MASTER_8259_PORT + 1;
	} else {
		irq_num -= 8;
		PICport = SLAVE_8259_PORT + 1;
	}

	data = inb(PICport) & ~(1 << irq_num);
	outb(data, PICport);
}

/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num)
{
	uint8_t data;
	uint16_t PICport;

	if (irq_num < 8) {
		PICport = MASTER_8259_PORT + 1;
	} else {
		PICport = SLAVE_8259_PORT + 1;
		irq_num -= 8;
	}

	data = inb(PICport) | (1 << irq_num);
	outb(data, PICport);
}

/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num)
{
	unsigned char out_eoi;
	uint32_t new_irq_num;

	if (irq_num >= 8) {
		new_irq_num = irq_num - 8;
		out_eoi = EOI | new_irq_num;
		outb(out_eoi, SLAVE_8259_PORT);
		outb(EOI | 2, MASTER_8259_PORT);
	} else {
		out_eoi = EOI | irq_num;
		outb(out_eoi, MASTER_8259_PORT);
	}
}
