#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
#include "systemcalls_handle.h"
#include "pit.h"

#define IDT_SIZE 256
#define SYSCALL 128

extern uint32_t pagefault;
extern uint32_t syscall_ret;


extern void do_irq(int num);
void InitializeIDT();

// Each of these call do_irq in doIrq.c with a different parameter
// based on which irq was triggered.
// inputs - none
// outputs - none
// side effects - calls do_irq with the irq number.
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq16();
extern void irq17();
extern void irq18();
extern void irq19();
extern void irq20();
extern void irq32();
extern void irq33();
extern void irq40();
extern void irq128();


