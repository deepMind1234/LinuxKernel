#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "lib.h"

#define VIDEO 0xB8
#define TERM_0_IDX 0xB9
#define TERM_1_IDX 0xBA
#define TERM_2_IDX 0xBB
#define VIDEO_ADDRESS 0xB8000
#define TERM_0_ADDRESS 0xB9000
#define TERM_1_ADDRESS 0xBA000
#define TERM_2_ADDRESS 0xBB000
#define MB_4 0x400000
#define MB_8 0x800000
#define KB_4 0x1000
#define NUM_ENTRIES_PD 1024
#define NUM_ENTRIES_PT 1024

typedef union pagedir_entry_t {
    uint32_t val;
    struct {
        uint32_t present        : 1;
        uint32_t read_write     : 1;
        uint32_t supervisor     : 1;
        uint32_t write_through  : 1;    
        uint32_t cache_disable  : 1;
        uint32_t accessed       : 1;
        uint32_t dirty          : 1;
        uint32_t page_size      : 1;
        uint32_t global         : 1;
        uint32_t available      : 3;
        uint32_t address        : 20;
    } __attribute__ ((packed));
} pagedir_entry_t;

pagedir_entry_t page_d[1024] __attribute__((aligned(4096)));
pagedir_entry_t kernel_page_table[1024] __attribute__((aligned(4096)));
pagedir_entry_t user_page_table[1024] __attribute__((aligned(4096)));


void create_paging(void);
extern void loadPageDirectory(pagedir_entry_t *page_d);
extern void enablePaging();
void new_process_page(int page_number, int pid);
void unmap_process_page(int page_number, int pid);

uint32_t map_user_page(void *physaddr, void *virtualaddr);
uint32_t unmap_user_page(void *virtualaddr);

uint32_t copyBufferToVideoMem(uint32_t term);
uint32_t copyVideoMemToBuffer(uint32_t term);

void mapVideoMemory();
void unmapVideoMemory();

#endif
