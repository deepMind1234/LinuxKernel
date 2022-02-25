#include "paging.h"

/*
Create_Paging
Description: This function enables and sets up the page directory as well as the first paging table, it loads up VideoMem on PT and the kernel on 4MB

Input: nothing but it makes use of predefined arrays for page_directory and page table
Output: on call it loads up and enable paging, so after this function is called we should be paging enabled
SideEffects
*/
void create_paging(void) {
    unsigned int i;
    
    
    for(i = 0; i < NUM_ENTRIES_PD; i++) // assign 0 and 1 later 
    {
        // This sets the following flags to the pages:
        //   Supervisor: Only kernel-mode can access them
        //   Write Enabled: It can be both read from and written to
        //   Not Present: The page table is not present
        // referenced off osdev
        kernel_page_table[i].present = 0;
        kernel_page_table[i].read_write = 1;
        kernel_page_table[i].supervisor = 0;
        kernel_page_table[i].write_through = 0;
        kernel_page_table[i].cache_disable = 0;
        kernel_page_table[i].accessed = 0;
        kernel_page_table[i].dirty = 0;
        kernel_page_table[i].page_size = 0;
        kernel_page_table[i].global = 0;
        kernel_page_table[i].available = 0;
        kernel_page_table[i].address = i;

        user_page_table[i].present = 0;
        user_page_table[i].read_write = 1;
        user_page_table[i].supervisor = 0;
        user_page_table[i].write_through = 0;
        user_page_table[i].cache_disable = 0;
        user_page_table[i].accessed = 0;
        user_page_table[i].dirty = 0;
        user_page_table[i].page_size = 0;
        user_page_table[i].global = 0;
        user_page_table[i].available = 0;
        user_page_table[i].address = i;
    }

    
    //we will fill all 1024 entries in the table, mapping 4 megabytes
    for(i = 0; i < NUM_ENTRIES_PD; i++)
    {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes ;)
        // referenced off osdev
        page_d[i].present = 0;
        page_d[i].read_write = 1;
        page_d[i].supervisor = 0;
        page_d[i].write_through = 0;
        page_d[i].cache_disable = 0;
        page_d[i].accessed = 0;
        page_d[i].dirty = 0;
        page_d[i].page_size = 0;
        page_d[i].global = 0;
        page_d[i].available = 0;
        page_d[i].address = 0;
    }

    // video memory.
    kernel_page_table[VIDEO].present = 1;
    kernel_page_table[VIDEO].address = VIDEO;

    // allocate pages for video buffers
    kernel_page_table[TERM_0_IDX].present = 1;
    kernel_page_table[TERM_0_IDX].address = TERM_0_IDX;

    kernel_page_table[TERM_1_IDX].present = 1;
    kernel_page_table[TERM_1_IDX].address = TERM_1_IDX;

    kernel_page_table[TERM_2_IDX].present = 1;
    kernel_page_table[TERM_2_IDX].address = TERM_2_IDX;


    page_d[0].present = 1;
    page_d[0].address = ((uint32_t)kernel_page_table)>>12;
    
    page_d[1].present = 1;
    page_d[1].global = 1;
    page_d[1].page_size = 1;    // make sure kernel takes full 4MB
    page_d[1].address = MB_4 >> 12;

    loadPageDirectory(page_d);
    enablePaging();
}

/*flush_tlb
 *replaces value in %cr3 with itself. This clears the lookup buffer
 *inputs - none
 *side effects - clears TLB
 *outputs - none
 */
void flush_tlb(){
    asm volatile(
         "movl %cr3, %eax;"
         "movl %eax, %cr3;"
    );
 }

/*new_process_page
 *establish a new process as a 4mb page in the page directory
 *inputs - page_number -> page_d index to place page
 *         pid -> pid of user program
 *outputs - none
 *side effects - maps page to physical address of running program
 */
void new_process_page(int page_number, int pid){
    page_d[page_number].present = 1;
    page_d[page_number].supervisor = 1;
    page_d[page_number].page_size = 1;
    page_d[page_number].address = (pid * MB_4 + MB_8)>>12; // I think this is determined by where the user level program is loaded
    flush_tlb();
}

/*unmap_process_page
 *undo everything that was done in new_process_page
 *inputs - page_number -> page_d index to clear
 *         pid -> unused
 */
void unmap_process_page(int page_number, int pid){
    page_d[page_number].present = 0;
    page_d[page_number].address = 0;
    flush_tlb();
}

/*map_user_page
 *create a mapping between a physical address and a requested virtual
 *address. All new pages are made as a 4kb page in the user page table
 *inputs - physaddr -> physical memory location to map
 *         virtualaddr -> requested virtual memory location
 *outputs - 0 on fail, 1 on success
 */
uint32_t map_user_page(void *physaddr, void *virtualaddr) {
    // verify virtual address and physical address is page aligned
    // virtualaddr = (void*)((uint32_t)virtualaddr & 0xFFFF8000);
    if(((uint32_t)virtualaddr & 0x3FF) != 0) return 0;
    if(((uint32_t)physaddr & 0x3FF) != 0) return 0;

    uint32_t pd_index = ((uint32_t)virtualaddr)>>22;
    uint32_t pt_index = ((uint32_t)virtualaddr)>>12 & 0x3FF;

    // verify page directory/page table entry is unused
    // if(page_d[pd_index].present == 1) return 0;

    page_d[pd_index].present = 1;
    page_d[pd_index].supervisor = 1;
    page_d[pd_index].page_size = 0;
    page_d[pd_index].address = ((uint32_t)user_page_table)>>12;
    user_page_table[pt_index].present = 1;
    user_page_table[pt_index].supervisor = 1;
    user_page_table[pt_index].address = ((uint32_t)physaddr)>>12;
    
    flush_tlb();
    return 1;
}

/*unmap_user_page
 *undo everything done in map_user_page
 *inputs -> virtualaddr - virtual address to unmap
 *side effects -> unmaps user page
 *outputs -> 1 on success, 0 on fail
 */
uint32_t unmap_user_page(void *virtualaddr) {
    if(((uint32_t)virtualaddr & 0x3FF) != 0) return 0;

    // uint32_t pd_index = ((uint32_t)virtualaddr)>>22;
    uint32_t pt_index = ((uint32_t)virtualaddr)>>12 & 0x3FF;

    // page_d[pd_index].present = 0;
    user_page_table[pt_index].present = 0;

    flush_tlb();
    return 1;
}

/* copyVideoMemToBuffer
 * copy all data from visible video memory to
 * a backup buffer in memory. called on terminal
 * change
 * inputs - term -- terminal number to copy
 */
uint32_t copyBufferToVideoMem(uint32_t term) {
    switch(term) {
        case 0:
            memcpy((void*)VIDEO_ADDRESS, (void*)TERM_0_ADDRESS, 4096);
            break;
        case 1:
            memcpy((void*)VIDEO_ADDRESS, (void*)TERM_1_ADDRESS, 4096);
            break;
        case 2:
            memcpy((void*)VIDEO_ADDRESS, (void*)TERM_2_ADDRESS, 4096);
            break;
        default:
            return -1;
    }
    return 0;
}

/* copyVideoMemToBuffer
 * copy all data from visible video memory to
 * a backup buffer in memory. called on terminal
 * change
 * inputs - term -- terminal number to copy
 */

uint32_t copyVideoMemToBuffer(uint32_t term) {
    switch(term) {
        case 0:
            memcpy((void*)TERM_0_ADDRESS, (void*)VIDEO_ADDRESS, 4096);
            break;
        case 1:
            memcpy((void*)TERM_1_ADDRESS, (void*)VIDEO_ADDRESS, 4096);
            break;
        case 2:
            memcpy((void*)TERM_2_ADDRESS, (void*)VIDEO_ADDRESS, 4096);
            break;
        default:
            return -1;
    }
    return 0;
}

/* mapVideoMemory() 
 * map the correct video memory to each terminals
 */
void mapVideoMemory() {
    kernel_page_table[VIDEO].address = VIDEO;
    kernel_page_table[TERM_0_IDX].address = (getTerminalNumber() == 0) ? VIDEO : TERM_0_IDX;
    kernel_page_table[TERM_1_IDX].address = (getTerminalNumber() == 1) ? VIDEO : TERM_1_IDX;
    kernel_page_table[TERM_2_IDX].address = (getTerminalNumber() == 2) ? VIDEO : TERM_2_IDX;
    user_page_table[0].address = (getTerminalNumber() == 0) ? VIDEO : TERM_0_IDX;
    user_page_table[1].address = (getTerminalNumber() == 1) ? VIDEO : TERM_1_IDX;
    user_page_table[2].address = (getTerminalNumber() == 2) ? VIDEO : TERM_2_IDX;

    flush_tlb();
}
/* unmapVideoMemory() 
 * unmap the video memory for each terminals
 */
void unmapVideoMemory() {
    kernel_page_table[VIDEO].address = VIDEO;
    kernel_page_table[TERM_0_IDX].address = TERM_0_IDX;
    kernel_page_table[TERM_1_IDX].address = TERM_1_IDX;
    kernel_page_table[TERM_2_IDX].address = TERM_2_IDX;
    user_page_table[0].address = TERM_0_IDX;
    user_page_table[1].address = TERM_1_IDX;
    user_page_table[2].address = TERM_2_IDX;

    flush_tlb();
}

