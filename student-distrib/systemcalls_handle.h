#ifndef _SYSTEMCALLS_HANDLE_H
#define _SYSTEMCALLS_HANDLE_H

#include "fs.h"
#include "lib.h"
#include "process.h"
#include "paging.h"
#include "terminal.h"
#include "x86_desc.h"
#include "rtc.h"

#define MAX_CMD_LEN 256
#define MAX_PROCESSES 6
#define PROGRAM_COPY_LOC 0x08048000
#define USER_MEM_CAP 0x083FFFFC
#define IN_USE      0
#define NOT_IN_USE  1
#define FDES_START  2
#define FDES_END    7
#define FD_STDIN    0
#define FD_STDOUT   1
#define BAD_CALL    -1
#define FORTY_K_BUFF  40000

extern int8_t storeargs[MAX_PROCESSES][128];

int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd,void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t nullfunc ();
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);

#endif
