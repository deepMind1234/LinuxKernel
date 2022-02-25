/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* i8259_init
 * Initialize the 8259 PIC
 * input: none
 * output: none
 * side effect: start up pic
 */
void i8259_init(void) {
    master_mask = 0xff; // mask all intr for master
    slave_mask = 0xff; //mask all intr for slave
    // init pic master
    cli();
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW4, MASTER_8259_DATA);
    // init pic slave
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);
    // mask out all port
    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);
    enable_irq(SLAVE_ON_MASTER); // unmask master port that connect to slave
    sti();
}

/* enable_irq
 * Enable (unmask) the specified IRQ
 * input: irq num
 * output: none
 * side effect: enable specific irq
 */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
    // determine which port should be write
    if (irq_num < PIC_LIMT) {
        port = MASTER_8259_DATA;
    } else {
        port = SLAVE_8259_DATA;
        irq_num -= PIC_LIMT;
    }
    value = inb(port) & ~(1 << irq_num); // if corresponding port is 1, set to 0
    outb(value, port);
}

/* disable_irq
 * Disable (mask) the specified IRQ
 * input: irq num
 * output: none
 * side effect: disable specific irq
 */
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
    // determine which port should be write
    if (irq_num < PIC_LIMT) {
        port = MASTER_8259_DATA;
    } else {
        port = SLAVE_8259_DATA;
        irq_num -= PIC_LIMT;
    }
    value = inb(port) | (1 << irq_num); // if corresponding port is 0, set to 1
    outb(value, port); // write new irq num
}

/* send_eoi(uint32_t irq_num
 * Send end-of-interrupt signal for the specified IRQ
 * input: irq num
 * output: none
 * side effect: send end of intr
 */
void send_eoi(uint32_t irq_num) {

    if (irq_num >= PIC_LIMT && irq_num <= PIC_END) {
        irq_num -= PIC_LIMT;
        outb(EOI | irq_num, SLAVE_8259_PORT); // send or'd eoi to slave
        outb(EOI | SLAVE_ON_MASTER, MASTER_8259_PORT); // send 2 or'd eoi to master
    } else if (irq_num < PIC_LIMT && irq_num >= PIC_START) {
        outb(EOI | irq_num, MASTER_8259_PORT); // send or'd eoi to master
    }
}
