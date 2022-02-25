#pragma once
#include "lib.h"
#include "keyboard.h"
#include "paging.h"
#include "systemcalls_handle.h"

#define bufferSize 128
#define numOfTerminal 3
void CTRLL(int terminalid);


typedef struct{
    char terminalBuff[bufferSize];
    char writeBuff[bufferSize];
    char charBuff[1024];
    int numchar;
    int ENTER;

    int pid;
    int screen_x;
    int screen_y;
} term_t;

extern int activeTerminal;
extern int executingTerminal;
extern term_t terminals[numOfTerminal];


void multiterm_init();
void switchToTerminal(uint32_t n);

extern int32_t terminalRead (int32_t fd, void* buf, int32_t nbytes,int terminalid);
extern int32_t terminalWrite (int32_t *fd, const void* buf, int32_t nbytes,int terminalid );
extern int32_t terminalOpen (const uint8_t* filename);
extern int32_t terminalClose (int32_t fd);

int32_t terminal_open(char* filename, int32_t* inode);
int32_t terminal_close(int32_t* inode);

int32_t terminal_write(int32_t *fd, void* buf, int32_t nbytes, int terminalid); 
int32_t terminal_read(int32_t fd, int32_t *offset, uint8_t* buf, int32_t length, int terminalid ); 

int32_t getTerminalNumber();
int32_t setTerminalNumber(uint32_t n);

int32_t getExecutingProcessInTerminal(uint32_t terminalId);
int32_t setExecutingProcessInTerminal(uint32_t terminalId, int pid);

int32_t getExecutingTerminalIndex();
void setExecutingTerminalIndex(int32_t termIdx);
