//#include "keyboard.h"
#include "interrupt.h"
#include "lib.h"
#define status ((uint8_t)256)
int errorcode;


/*
Do_IRQ
Description: this function is the c handler of all the exceptions, it switches depending on the int number and runs the interrupt code
Input: INTERRUPT NUMBER 
Output: depending on the interrupt called produces some expected result, currently halts the program
SideEffects
*/
void do_irq(int num){
    //uint8_t value;
    //printf("Reached Do_Irq");
    switch(num){
        case 0:
        clear();
        printf("Divide Error");
        
        halt(status);
        
        case 1:
        clear();
        printf("Exception");
        halt(status);
        
        case 2:
        clear();
        printf("NMI Interrupt");
        halt(status);
        
        case 3:
        clear();
        printf("Breakpoint");
        halt(status);
        
        case 4:
        clear();
        printf("Overflow");
        halt(status);
        
        case 5:
        clear();
        printf("BOUND Range Exceeded");
        halt(status);
        
        case 6:
        clear();
        printf("Invalid Opcode");
        halt(status);
        
        case 7:
        clear();
        printf("Device Not Available");
        halt(status);
        
        case 8:
        clear();
        printf("Double Fault");
        halt(status);
        
        case 9:
        clear();
        printf("Coprocessor Segment Overrun");
        halt(status);
        
        case 10:
        clear();
        printf("Invalid TSS");
        halt(status);
        
        case 11:
        clear();
        printf("Segment Not Present");
        halt(status);
        
        case 12:
        clear();
        printf("Stack-Segment Fault");
        halt(status);
        
        case 13:
        clear();
        printf("General Protection");
        halt(status);
        
        case 14:
        // asm volatile(
        // "popl %eax;"
        // "movl %eax, errorcode;"
        // );   
        clear();
        printf("Page Fault \n");
        printf("ERROR CODE INTEGER %d", pagefault);
        halt(status);
        
        // case 15:
        // clear();
        // printf("Exception");
        // halt(status);
        // 
        case 16:
        clear();
        printf("Math Fault");
        halt(status);
        
        case 17:
        clear();
        printf("Alignment Check");
        halt(status);
        
        case 18:
        clear();
        printf("Machine Check");
        halt(status);
        
        case 19:
        clear();
        printf("Floating-Point Exception");
        halt(status);
        
        case 20:
        clear();
        printf("Exception");
        halt(status);
        
        case 32:
        pit_interrupt();
        return;

        case 33:
        keyboard_intr();
        return;

        case 40:
        rtc_interrupt();
        return;
        

        
        default:
        //printf("NotDivideby0\n");
        return;

   }
}

