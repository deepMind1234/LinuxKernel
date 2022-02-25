#include "systemcalls_handle.h"
#define maxProcess 6
#define terminalBuffSize 128

int8_t storeargs[MAX_PROCESSES][terminalBuffSize];

#define argSize 127
int realArgSize;

// only valid function for terminal is terminal_read
// others should be bad_read, bad_write, bad_close return BAD_CALL or sum
// make sure it's the right arguments

//fops table used in file descritor, specified for four type
// fops_t stdin_table = {
//     terminal_read,
//     terminal_write,
//     terminal_open,
//     terminal_close
// };
// fops_t stdout_table = {
//     terminal_read,
//     terminal_write,
//     terminal_open,
//     terminal_close
// };
fops_t dir_table = {
    dir_read,
    dir_write,
    dir_open,
    dir_close
};
fops_t file_table = {
    file_read,
    file_write,
    file_open,
    file_close
};
fops_t rtc_table = {
    rtc_read,
    rtc_write,
    rtc_open,
    rtc_close
};


// init_pcb
    // starts at 0x800000
    // each new frame is 8kb grows downward
    // calculate frame base location with pid

int32_t halt (uint8_t status) {
    int32_t status_ext = status; 
    int32_t curr_pid = getExecutingProcessInTerminal(getExecutingTerminalIndex());
    
    pcb_t* pcb = get_pcb(curr_pid);
    if(pcb->p_pid == -1) {  // when there is no shell on terminal , open a new one
        pcb_free(curr_pid);
        setExecutingProcessInTerminal(getExecutingTerminalIndex(), -1);
        clear();
        execute((uint8_t*)"shell");
    }
    memset(&storeargs[curr_pid],'\0',128); //init argument

    cli();
    
    uint32_t p_pid =pcb->p_pid;
    uint32_t p_ebp = pcb->p_ebp;
    uint32_t p_esp =pcb->p_esp;
    uint32_t p_esp0 = pcb->p_esp0;

    ///////////////////
    // RESETUP PARENT PAGING
    //////////////////
    unmap_process_page(PROCESS_PAGE, curr_pid); //sanity check
    new_process_page(PROCESS_PAGE, p_pid); //sanity check
    
    // check if vidmap is still being used
    
    unmap_user_page((void*)(MB_4*35 + KB_4*getExecutingTerminalIndex()));

    ///////////////////
    // CLEAN UP THE PCB STRUCT
    //////////////////
    pcb_free(curr_pid);
    setExecutingProcessInTerminal(getExecutingTerminalIndex(), p_pid);
    tss.esp0 = p_esp0;

    sti();
    asm volatile (
        "movl %0, %%eax;"
        "movl %2, %%esp;"
        "movl %1, %%ebp;"
        "jmp HALT_POINT;"
        :
        : "r"(status_ext), "r"(p_ebp), "r"(p_esp)
        :"eax"

    );

    return BAD_CALL;
}

int32_t execute (const uint8_t* command) {
    ///////////////////////////////////////
    // step 1. parse command
    ///////////////////////////////////////  
    if (!command) return BAD_CALL;

    int32_t storeEBP, storeESP;
    int i;
    int j;
    int found_file = 0;
    int8_t fname[FILENAME_LEN + 1];
    int found_arg = 0;
    int8_t args[argSize+1];
    int k = 0;
    memset(args,'\0',terminalBuffSize);

    for(i = 0; i < FILENAME_LEN + 1; i++) //name matching
        fname[i] = '\0';

    while(command[k] == ' '){
        k++;
    }


    for(i = k; i < k + FILENAME_LEN; i++) {//name matching
        if (command[i] == ' ' || command[i] == '\0') {
            found_file = 1;
            break;
        }
    }

    if(command[i] == ' '){ //name matching
        for(j = 0; j < argSize+1 ; j++){
            if (command[i + j + 1] == ' ' || command[i + j + 1] == '\0') {
                found_arg = 1;
                break;
            }   
        }     
    }   
    realArgSize = j + 1;


    if(!found_file) return FS_FAIL;
    strncpy(fname, (int8_t*)(&command[k]), i-k);

    
    if(found_arg){
        for(j = 0; j <= realArgSize ; j++){
            args[j] = command[i + j + 1];
            if(j == realArgSize)
                args[j] = '\0';
        }
    }
    
   

    ///////////////////////////////////////
    // step 2. validate if executable (ELF/Filetype)
    ///////////////////////////////////////
    dentry_t dentry;
    int32_t inode_idx;
    int32_t offset = 0; // offset init
    uint8_t buf[64];
    int32_t curr_pid = getExecutingProcessInTerminal(getExecutingTerminalIndex());

    // confirm filetype
    if (read_dentry_by_name(fname, &dentry) == FS_FAIL) return FS_FAIL;
    if(dentry.filetype != RGL_FILE_TYPE) return FS_FAIL;

    // confirm starting bytes
    if(file_open(fname, &inode_idx) == FS_FAIL) return FS_FAIL;
    if(file_read(inode_idx, &offset, buf, 4) == FS_FAIL) return FS_FAIL;
    
    if(!(buf[0] == 0x7F && buf[1] == 0x45 && buf[2] == 0x4c && buf[3] == 0x46)) // check binary num
        return FS_FAIL;

    ////////////////////////////////////////
    // step 3. map user program
    ////////////////////////////////////////
   
    uint32_t next_pid = -1;
    for(i = 0; i < MAX_PROCESSES; i++) {
        if(get_pcb(i)->present == 0) {
            next_pid = i;
            break;
        }
    }
    
    if(next_pid < 0) return -1;

    ////////////////////////////////////////////
    //Store Arguements for get Args
    ////////////////////////////////////////////
    if (curr_pid != -1 && found_arg == 1){ 
        strncpy(storeargs[next_pid], args,j+1);
    }
    ///////////////////////////////////////////

    if(next_pid < 0 || next_pid >= MAX_PROCESSES) return -1;

    new_process_page(PROCESS_PAGE, next_pid);

    ////////////////////////////////////////
    // step 4. load user program (copying, look for starting)
    ////////////////////////////////////////
    uint32_t eip_addr = 0x00000000; // eip_addr init val
    offset = 24; // offset init value
    if(file_read(inode_idx, &offset, buf, 4) == FS_FAIL) return FS_FAIL; 
    eip_addr = buf[0] + (buf[1]<<8) + (buf[2]<<16) + (buf[3]<<24); // converting to hex

    offset = 0; // offset init value
    if(file_read(inode_idx, &offset, (uint8_t*)PROGRAM_COPY_LOC, MB_4) == FS_FAIL) return FS_FAIL;
    // memcpy((char*)PROGRAM_COPY_LOC, (char*)buf, offset+1);
    

    ////////////////////////////////////////
    // step 5. setup pcb (8kb pages. define a function called pcb_init)
    ////////////////////////////////////////
    asm volatile(
        "movl %%ebp, %0;"
        "movl %%esp, %1;"
        : "=r"(storeEBP), "=r"(storeESP)
    );

    pcb_init(next_pid,storeEBP,storeESP, getExecutingProcessInTerminal(getExecutingTerminalIndex()) , getExecutingTerminalIndex());
    tss.ss0 = KERNEL_DS;
    tss.esp0 = (uint32_t)get_pcb(next_pid)->Kstack;
    setExecutingProcessInTerminal(getExecutingTerminalIndex(), next_pid);


    ////////////////////////////////////////
    // step 6. fake iret
    ////////////////////////////////////////
    sti();
    asm volatile(
        "andl $0x00FF, %%eax;"
        "movw %%ax, %%ds;"
        "pushl %%eax;"    
        "pushl %%ebx;"
        "pushfl ;"
        "pushl %%ecx;"
        "pushl %%edx;"
        "iret;"
        "HALT_POINT:;"
        "leave;"
        "ret;"
        :
        :"a"(USER_DS),"b"(USER_MEM_CAP),"c"(USER_CS),"d"(eip_addr)
        :"cc"
        
    );

    return 0;
}

/* read syscall
 * input: fd -- fd array index
 *        buf -- buffer for input
 *        nbytes -- num of bytes read
 * output: FAIL/PASS information
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes) {
    // arguments should be different when accessing different type
    // pseudo code example:
    // if (get_pcb(curr_pid)->fd_array[fd].fops == &dir_table) 
    //      return pcb->fd_array[fd].fops->read_spec(inode, offset, buf, length);
    if(fd<0 || fd>FDES_END) return FS_FAIL; //check invaild FD
    if(fd==FD_STDOUT) return FS_FAIL; //check invaild FD

    int32_t curr_pid = getExecutingProcessInTerminal(getExecutingTerminalIndex());//get process id
    if(fd>=FDES_START && get_pcb(curr_pid)->fd_array[fd].flags == NOT_IN_USE) return FS_FAIL;
    if(fd==FD_STDIN) return terminal_read(fd,NULL,buf,nbytes,get_pcb(curr_pid)->terminalId);//go to terminal
    if(fd>=FDES_START && fd<=FDES_END) {
        return get_pcb(curr_pid)->fd_array[fd].fops->read_spec(get_pcb(curr_pid)->fd_array[fd].inode,&get_pcb(curr_pid)->fd_array[fd].fpos,(uint8_t*)buf,nbytes);//go to file system
    }
        
    
    return BAD_CALL;
}

/* write syscall
 * input: fd -- fd array index
 *        buf -- buffer for output
 *        nbytes -- num of bytes write
 * output: FAIL/PASS information
 */
int32_t write (int32_t fd,void* buf, int32_t nbytes) {
    if(fd<0 || fd>FDES_END) return FS_FAIL; //check invaild FD
    
    int32_t curr_pid = getExecutingProcessInTerminal(getExecutingTerminalIndex()); //get process id
    if(fd>=FDES_START && get_pcb(curr_pid)->fd_array[fd].flags == NOT_IN_USE) return FS_FAIL;

    if(fd>=FDES_START && fd<=FDES_END) 
        return get_pcb(curr_pid)->fd_array[fd].fops->write_spec(&get_pcb(curr_pid)->fd_array[fd].inode,buf,nbytes);//only rtc use this, reuse inode for storing freq;
    
    if(fd==FD_STDIN) return FS_FAIL; //check invaild FD
    if(fd==FD_STDOUT) return terminal_write(&get_pcb(curr_pid)->fd_array[fd].inode,buf,nbytes,get_pcb(curr_pid)->terminalId);//go to terminal

    return BAD_CALL;
}

/* int32_t open (const uint8_t* filename)
 *   Find the file in the file system and assign an unused file descriptor
 *   File descriptors are be set up according to the file type
 */
int32_t open (const uint8_t* filename) {
    dentry_t fdentry;
    int32_t i;
    int32_t inode_idx;
    int32_t curr_pid = getExecutingProcessInTerminal(getExecutingTerminalIndex());
    
    // traverse the file descriptor to find empty lot
    for (i = FDES_START; i <= FDES_END; i++) {
        if (get_pcb(curr_pid)->fd_array[i].flags == NOT_IN_USE) {
            get_pcb(curr_pid)->fd_array[i].flags = IN_USE;
            get_pcb(curr_pid)->fd_array[i].fpos = NULL;
            
            if (strncmp((char*)filename,"rtc",4)==0) { //check for rtc open
                
                get_pcb(curr_pid)->fd_array[i].fops = &rtc_table;
                get_pcb(curr_pid)->fd_array[i].inode = 2; // inode for storing freq.
                rtc_open(NULL,&get_pcb(curr_pid)->fd_array[i].inode);
                return i;
            }

            if (read_dentry_by_name ((char*)filename, &fdentry) == FS_FAIL) { //read dentry
                get_pcb(curr_pid)->fd_array[i].flags = NOT_IN_USE;
                return FS_FAIL;
            }

            switch (fdentry.filetype) {
                case RGL_FILE_TYPE://check for file open
                    if (file_open((char*)filename, &inode_idx) == FS_FAIL) {
                        get_pcb(curr_pid)->fd_array[i].flags = NOT_IN_USE;
                        return FS_FAIL;
                    }
                    get_pcb(curr_pid)->fd_array[i].fops = &file_table;
                    get_pcb(curr_pid)->fd_array[i].inode = inode_idx;
                    break;
                case DIRECTORY_TYPE://check for dir open
                    if (dir_open((char*)filename, &inode_idx) == FS_FAIL){
                        get_pcb(curr_pid)->fd_array[i].flags = NOT_IN_USE;   
                        return FS_FAIL; 
                    } 
                    get_pcb(curr_pid)->fd_array[i].fops = &dir_table;
                    get_pcb(curr_pid)->fd_array[i].inode = NULL;
                    break;
            }
            break;
        } else if (i == FDES_END) {
            return BAD_CALL;
        }
    }
    return i;
}

/*int32_t close (int32_t fd)
 *Close the file descriptor passed in (set it to be available)
 *Check for invalid descriptors
 *side effects - none
 */

int32_t close (int32_t fd) {
    int32_t inode_idx;
    int32_t curr_pid = getExecutingProcessInTerminal(getExecutingTerminalIndex());
    if (fd<FDES_START || fd>FDES_END) return BAD_CALL;//invaild
    if (fd>=FDES_START && get_pcb(curr_pid)->fd_array[fd].flags == NOT_IN_USE) return BAD_CALL; //invaild

    inode_idx = get_pcb(curr_pid)->fd_array[fd].inode;
    if (get_pcb(curr_pid)->fd_array[fd].fops->close_spec(&inode_idx) == FS_FAIL) return FS_FAIL;//invaild
    get_pcb(curr_pid)->fd_array[fd].flags = NOT_IN_USE;//clean
    return NULL;
}

/*nullfunc
 *placeholder function for file operations not permitted
 *inputs - none
 *outputs - BAD_CALL
 *side effects - none
 */
int32_t nullfunc () {
    return BAD_CALL;
}


/*getargs
 *return arguments from execute call to user program
 *inputs - buf -> buffer to fill with arguments
 *         nbytes -> number of bytes to copy to buf
 *outputs - none
 *side effects - fills buffer with stored arguments
 */
int32_t getargs (uint8_t* buf, int32_t nbytes){
    int i;
    int32_t curr_pid = getExecutingProcessInTerminal(getExecutingTerminalIndex());
    if(realArgSize > nbytes) return -1;
    if(curr_pid == -1) return -1;
    
    int8_t* args = storeargs[curr_pid] ;
    if((*(args) == NULL)) return -1;
    memset(buf,'\0', nbytes);
    for(i = 0; i < realArgSize; i++) {  //matching the argument
        if((i == 0) && (args[i] == '\0')) return -1;
        buf[i] = args[i];
    }

    //memcpy(buf,storeargs[curr_pid],nbytes);
    return 0;
}

/*vidmap
 *map video memory to a predetermined address so it is accessible
 *to user programs
 *inputs - screen_start -> reference to pointer that will store new virtual address
 *outputs - 1 on success, -1 on fail
 *side effects - map virtual address to video address
 */
int32_t vidmap (uint8_t** screen_start) {
    if(!screen_start) return -1;
    if(!((uint32_t)screen_start >= MB_4*32 && (uint32_t)screen_start <= MB_4*33)) return -1; //sanity check

    uint32_t vidmap_virtual_addr = MB_4*35;
    uint32_t phys_addr = VIDEO_ADDRESS;
    switch(getExecutingTerminalIndex()) { //selecting correct vedio mem to wirte
        case 0:
            // phys_addr = TERM_0_ADDRESS;
            vidmap_virtual_addr = MB_4*35;
            break;
        case 1:
            // phys_addr = TERM_1_ADDRESS;
            vidmap_virtual_addr = MB_4*35 + KB_4*1;
            break;
        case 2:
            // phys_addr = TERM_2_ADDRESS;
            vidmap_virtual_addr = MB_4*35 + KB_4*2;
            break;
    }

    if(!map_user_page((void*)phys_addr, (void*)vidmap_virtual_addr)) return -1;

    *screen_start = (uint8_t*)vidmap_virtual_addr;
    return 1;
}

/*set_handler
 *syscall defined, but not implemented
 *inputs - ignored
 *outputs - 0
 *side effects - none
 */
int32_t set_handler (int32_t signum, void* handler_address) {
    return 0;
}

/*sigreturn
 *syscall defined, but not implemented
 *inputs - ignored
 *outputs - 0
 *side effects - none
 */
int32_t sigreturn () {
    return 0;
}



