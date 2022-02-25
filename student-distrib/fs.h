#ifndef _FS_H
#define _FS_H

#include "types.h"
#include "lib.h"
//#include "process.h"

#define DENTRY_MAX      63
#define FILENAME_LEN    32    
#define DENTRY_RESERVE  24
#define BB_RESERVE      52
#define INODE_RESERVE   1023
#define DATA_RESERVE    1024
#define USR_RTC_TYPE    0
#define DIRECTORY_TYPE  1
#define RGL_FILE_TYPE   2
#define INODE_OFFSET    1
#define BLOCK_SIZE      4096

#define FS_FAIL -1
#define FS_PASS 0

typedef struct {
    char filename[FILENAME_LEN];
    uint32_t filetype;
    uint32_t inode_num;
    uint8_t reserved[DENTRY_RESERVE];
} dentry_t;

typedef struct {
    uint32_t dir_count;
    uint32_t inode_count;
    uint32_t data_count;
    uint8_t reserved[BB_RESERVE];
    dentry_t direntries[DENTRY_MAX];
} boot_block_t;

typedef struct {
    uint32_t length;
    uint32_t data_block_num[INODE_RESERVE];
} inode_t;

typedef struct {
    uint32_t data[DATA_RESERVE];
} data_block_t;



int32_t fs_init(uint32_t fs_start, uint32_t fs_end);
int32_t file_open(char* filename, int32_t* inode);
int32_t file_close(int32_t* inode);
int32_t file_write(int32_t *fd, void* buf, int32_t nbytes); // modification needed
int32_t file_read(int32_t fd, int32_t *offset, uint8_t* buf, int32_t length);  // modification needed

int32_t dir_open(char* filename, int32_t* inode);
int32_t dir_close(int32_t* inode);
int32_t dir_write(int32_t *fd, void* buf, int32_t nbytes); // modification needed
int32_t dir_read(int32_t fd, int32_t *offset, uint8_t* buf, int32_t nbytes);  // modification needed

// int32_t file_open_driver(char* filename);
// int32_t file_close_driver(int32_t fd);
// int32_t file_write_driver(int32_t fd, void* buf, int32_t nbytes); // modification needed
// int32_t file_read_driver(int32_t fd, void* buf, int32_t nbytes);  // modification needed

// int32_t dir_open_driver(char* filename);
// int32_t dir_close_driver(int32_t fd);
// int32_t dir_write_driver(int32_t fd, void* buf, int32_t nbytes); // modification needed
// int32_t dir_read_driver(int32_t fd, void* buf, int32_t nbytes);  // modification needed

int32_t read_dentry_by_name (const char* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t fd, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t print_dentry_details (uint8_t* fname);

uint32_t dir_num ();
/*
data_block_t* blk_loc(uint32_t block_idx);
inode_t* inode_loc(uint32_t inode);
*/
#endif //_FS_H
