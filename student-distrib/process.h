#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "systemcalls_handle.h"
#include "lib.h"
#include "paging.h"

#define FD_ARR_SIZE 8
#define PROCESS_PAGE 32

//specific fops stuct used in file descriptor
typedef struct fops_t {
    int32_t (*read_spec)(int32_t fd, int32_t *offset, uint8_t* buf, int32_t length);
    int32_t (*write_spec)(int32_t *fd, void* buf, int32_t nbytes);
    int32_t (*open_spec)(char* filename, int32_t* inode);
    int32_t (*close_spec)(int32_t* inode);
} fops_t;

typedef struct{
    fops_t *fops;
    int32_t inode;
    int32_t fpos;
    int32_t flags;
} file_t;

typedef struct pcb_t {
    uint32_t present; // 0 = unused, 1 = in use
    int32_t pid; // PROCESS IDENTIFICATION
    int32_t p_pid;
    uint32_t p_ebp; // ebp of calling execute function
    uint32_t p_esp; // esp of calling execute function
    uint32_t c_ebp; // current ebp stash
    uint32_t c_esp; // current esp stash
    uint32_t p_esp0; // pointer to previous kstack entry
    uint32_t* Kstack; // pointer to own kstack entry

    uint32_t background; // if it should receive execution time in background
                         // 1-> no execution, 0->execution
    int terminalId;
    file_t fd_array[FD_ARR_SIZE]; // PROCESS CONTROL
} pcb_t;

uint32_t activeTerminalProcesses[3];

// loop through ___ to find pcb with matching pid
pcb_t* get_pcb(int32_t pid);
void pcb_init(int32_t pid, uint32_t ebp ,uint32_t esp , int32_t p_pid, uint32_t term);
int32_t pcb_free(int32_t pid);

void switchToProcess(int32_t curr_pid, int32_t new_pid);
void saveProcessState(int32_t pid);
void loadProcessState(int32_t pid);

#endif // PROCESS_H
