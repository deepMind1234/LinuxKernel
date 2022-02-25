#include "interrupt.h"

/*
InitIDT
Description: this functon intitializes the idt, by looping through the 32 bit struct and assiging the values for idt elements, 
set idt entry sets up the offsets for the function pointer

Input: nothing but it makes use of the idt[] array 
Output initialised idt table that can be loaded on boot
SideEffects
*/
void InitializeIDT(){ // Function which initialize idt
    int i;
    // 20 = number of standard exceptions
    for(i = 0;i < 20 ;i++){ // loop through the first 
        // default to kernel level exception
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 1; // is an exception
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;
        idt[i].reserved0 = 0;
        idt[i].dpl = 0; // dpl is set to kernel level priority
        idt[i].present =1; // interrupt is present
    }
    
    // we are ignoring 20- 32 as INTEL RESERVED
    for(i = 31;i < IDT_SIZE ;i++){  // !!!!!!!!!!!!!!! CHANGED HERE TO ACCOMODATE PIT, CHANGE 31 BACK TO 32 IF TEST FAIL !!!!!!!!!!!!!!!!!
        // Rest of Interrupts initialized 
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 0; 
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;
        idt[i].reserved0 = 0;
        idt[i].dpl = 0; // kernel level priority
        idt[i].present =1;
    }
    idt[SYSCALL].dpl = 3; //

    
    SET_IDT_ENTRY(idt[0],irq0);
    SET_IDT_ENTRY(idt[1], irq1);
    SET_IDT_ENTRY(idt[2], irq2);
    SET_IDT_ENTRY(idt[3], irq3);
    SET_IDT_ENTRY(idt[4], irq4);
    SET_IDT_ENTRY(idt[5], irq5);
    SET_IDT_ENTRY(idt[6], irq6);
    SET_IDT_ENTRY(idt[7], irq7);
    SET_IDT_ENTRY(idt[8], irq8);
    SET_IDT_ENTRY(idt[9], irq9);
    SET_IDT_ENTRY(idt[10], irq10);
    SET_IDT_ENTRY(idt[11], irq11);
    SET_IDT_ENTRY(idt[12], irq12);
    SET_IDT_ENTRY(idt[13], irq13);
    SET_IDT_ENTRY(idt[14], irq14);
    //SET_IDT_ENTRY(idt[14], irq15);
    SET_IDT_ENTRY(idt[16], irq16);
    SET_IDT_ENTRY(idt[17], irq17);
    SET_IDT_ENTRY(idt[18], irq18);
    SET_IDT_ENTRY(idt[19], irq19);
    SET_IDT_ENTRY(idt[20], irq20);
    SET_IDT_ENTRY(idt[32], irq32);
    SET_IDT_ENTRY(idt[33], irq33);
    SET_IDT_ENTRY(idt[40], irq40);
    SET_IDT_ENTRY(idt[SYSCALL], irq128);
}


