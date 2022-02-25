#include "process.h"

#define KB_8 0x2000
#define MB_8 0x800000
#define pcbSize 28


/*pcb_init
 *initialize a pcb
 *inputs - pid -> pid of new process to create
 *         ebp -> stored ebp of execute function
 *         esp -> stored esp of parent_pid
 *         parent_pid -> pid of parent process to jump back to
 *outputs - none
 *side effects - initializes a pcb on the kernel stack.
 */
void pcb_init(int32_t pid, uint32_t ebp ,uint32_t esp , int32_t parent_pid, uint32_t term){
    int i;
    uint32_t curr_KstackBase = (MB_8 - KB_8 * pid)-4;
    uint32_t prev_KstackBase = (MB_8 - KB_8 * parent_pid)-4;
    pcb_t* pcb = get_pcb(pid);
    if(!pcb) return;

    pcb->present = 1;
    pcb->pid = pid;
    pcb->p_pid = parent_pid;
    pcb->p_ebp = ebp;
    pcb->p_esp = esp;
    pcb->p_esp0 = prev_KstackBase;
    pcb->Kstack = (uint32_t*)curr_KstackBase;

    pcb->terminalId = term;

    for (i = 0; i < FD_ARR_SIZE; i ++) {
        pcb->fd_array[i].flags = NOT_IN_USE;
    }
}

/*get_pcb
 *return address to pcb block of a pid
 *inputs - pid -> pid of pcb to fetch
 *outputs - pointer to pcb
 *side effects - none
 */
pcb_t* get_pcb(int32_t pid) {
    if(pid < 0 || pid > MAX_PROCESSES) return 0;

    return (pcb_t*)(MB_8 - KB_8 * (pid+1));
}

/*pcb_free
 *mark the pcb of a given pid as not present
 *inputs - pid -> pid of pcb to free
 *outputs - 1 on success, -1 on fail
 *side effects -> marks pcb as not present.
 */
int32_t pcb_free(int32_t pid){
    pcb_t* pcb = get_pcb(pid);
    if(!pcb) return -1;

    pcb->present = 0;
    return 1;
}

/*switchToProcess
 * switch to next process
 *inputs - int32_t curr_pid, int32_t new_pid
 *side effects - switch to next process
 */
void switchToProcess(int32_t curr_pid, int32_t new_pid) {
    saveProcessState(curr_pid);

    // load program memory to correct location
    unmap_process_page(PROCESS_PAGE, curr_pid);
    new_process_page(PROCESS_PAGE, new_pid);

    loadProcessState(new_pid);
}

/*saveProcessState
 * save the current state of current process
 *inputs - int32_t pid
 *outputs - none
 *side effects - none
 */
void saveProcessState(int32_t pid) {
    pcb_t* pcb = get_pcb(pid);

    asm volatile( //save the current state of current process
        "movl %%ebp, %0;"
        "movl %%esp, %1;"
        : "=r"(pcb->c_ebp), "=r"(pcb->c_esp)
    );
}

/*loadProcessState
 *load saved ProcessState and perform a context switch
 *inputs - int32_t pid
 *outputs - none
 *side effects - context switch
 */
void loadProcessState(int32_t pid) {
    pcb_t* pcb = get_pcb(pid);

    uint32_t store_ebp = pcb->c_ebp; //load saved ProcessState
    uint32_t store_esp = pcb->c_esp;

    tss.ss0 = KERNEL_DS;
    tss.esp0 = (uint32_t)get_pcb(pid)->Kstack;
    
    asm volatile( //context switch
        "movl %0, %%ebp;"
        "movl %1, %%esp;"
        "leave;"
        "ret;"
        : 
        : "r"(store_ebp), "r"(store_esp)
        : "memory"
    );
    
}

