#include "pit.h"
#include "i8259.h"

/* pit_init()
    description: init the pit to 1khz at irq0
    reference  from wiki.osdev.org/RTC
*/
void pit_init() {
    cli();
    outb(PIT_MODE, PIT_CMD); //set mode of pit
    outb(PIT_INTV_L, PIT_DATA); //set 1khz freq
    outb(PIT_INTV_H, PIT_DATA);

    enable_irq(PIT_IRQ);//enable irq0
    sti();
}
/* pit_interrupt()
    description: interrupt handler function for irq0
    do context switch every 10ms
*/
void pit_interrupt() {
    int32_t curr_pid = getExecutingProcessInTerminal(getExecutingTerminalIndex());
    int32_t new_pid;

    if(curr_pid == -1) {    //make sure there is at least one shell per terminal
        send_eoi(PIT_IRQ);
        execute((uint8_t*)"shell"); 
    }

    setExecutingTerminalIndex((getExecutingTerminalIndex() + 1) % 3);
    setVideoMemFromTermId(getExecutingTerminalIndex());
    new_pid = getExecutingProcessInTerminal(getExecutingTerminalIndex()); //select next process to switch

    if(new_pid == -1) { //make sure there is at least one shell per terminal
        send_eoi(PIT_IRQ);
        saveProcessState(curr_pid);
        execute((uint8_t*)"shell");
    } else {
        // perform context switch
        switchToProcess(curr_pid, new_pid);
    }

    send_eoi(PIT_IRQ); //END OF INTR
}
