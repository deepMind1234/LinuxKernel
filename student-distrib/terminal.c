#include "terminal.h"

int flag;
term_t terminals[numOfTerminal];
int activeTerminal = 0;
int executingTerminal = 0;

/*multiterm_init()
 *init the struct for 3 terminals
 *inputs - none
 *outputs - none
 */
void multiterm_init() {
    int i;
    for(i = 0; i < numOfTerminal; i++) { //init the struct for 3 terminals
        memset(terminals[i].terminalBuff, 0, bufferSize);
        memset(terminals[i].writeBuff, 0, bufferSize);
        memset(terminals[i].charBuff, 0, 1024);
        terminals[i].ENTER = 1;
        terminals[i].pid = -1;
        terminals[i].screen_x = 0;
        terminals[i].screen_y = 0;
    }
}

//helper interface functions
int32_t getExecutingTerminalIndex() {
    return executingTerminal;
}
//helper interface functions
void setExecutingTerminalIndex(int32_t termIdx) {
    executingTerminal = termIdx;
}

/*switchToTerminal(uint32_t n) 
 *function for terminal switch
 *inputs - switchToTerminal(uint32_t n) 
 *outputs - none
 */
void switchToTerminal(uint32_t n) {
    if(n == getTerminalNumber()) return;
    if(n<0 || n>=3) return;

    int32_t prevTerminalId = getTerminalNumber(); 
    // int32_t curr_pid = getExecutingProcessInTerminal(getTerminalNumber());

    unmapVideoMemory(); 
    
    copyVideoMemToBuffer(prevTerminalId); //remaping video memory
    copyBufferToVideoMem(n);

    setTerminalNumber(n);
    mapVideoMemory();

    update_cursor(terminals[n].screen_x, terminals[n].screen_y); //redraw the cursor
}

/*CTRLL
 *clear the terminal and set the cursor to the beginning
 *inputs - none
 *outputs - none
 *side effects - clears screen
 */
void CTRLL(int terminalid){
    clear();
    update_cursor(0,0);
    int i;

    for(i = 0; i < prevchar; i++){
        putc(terminals[terminalid].terminalBuff[i]);
        terminals[terminalid].numchar++;
    }
}


/* int32_t terminalOpen(const uint8_t* filename)
 * Inputs: filename
 * Return Value: int
 *  Function: opens terminal access for user level program
 */
int32_t terminalOpen(const uint8_t* filename){
    return 0;
}
/* terminalClose (int32_t fd)
 * Inputs: file dir
 * Return Value: int
 *  Function: closes terminal access for user level prog 
 */
int32_t terminalClose (int32_t fd){
    return 0;
}


/* int32_t terminalRead (int32_t fd, void* buf, int32_t nbytes
 * Inputs: file dir, pointer to buffer to copy data too, number of bytes to read
 * Return Value: integer which is the number fo chars read
 *  Function: prints value from terminal buffer to user level*/
int32_t terminalRead (int32_t fd, void* buf, int32_t nbytes, int terminalid){
    uint32_t bytes_to_copy = (nbytes > bufferSize) ? bufferSize : nbytes;
    memset(buf, 0, bytes_to_copy);
    
    while(terminals[terminalid].ENTER);
    terminals[terminalid].ENTER = 1;

    int charactersRead = strlen(terminals[terminalid].writeBuff) - 1;
    if(charactersRead > 127) {
        charactersRead = 127;
        terminals[terminalid].writeBuff[127] = '\n';
    }
    // while(terminals[terminalid].writeBuff[charactersRead] != '\n'){
    //     charactersRead++;
    // } 

    memcpy(buf,terminals[terminalid].writeBuff, charactersRead+1);
    memset(terminals[terminalid].writeBuff, 0, bufferSize);

    return charactersRead+1;
}

/* int32_t terminalWrite (int32_t fd, const void* buf, int32_t nbytes
 * Inputs: file dir, pointer to buffer to copy data from, number of bytes to write
 * Return Value: integer which is the number fo chars read
 *  Function: prints value from terminal buffer to user level*/

int32_t terminalWrite (int32_t *fd, const void* buf, int32_t nbytes,int terminalid){
    //if(nbytes > bufferSize)
        //return -1;
    memcpy(terminals[terminalid].charBuff,buf,nbytes); // we memcpy into out temporary buff this allows me not to cast to (char*)

    int i;
    uint32_t chars_added = 0;
    for(i = 0; i < nbytes; i++){
        if (terminals[terminalid].charBuff[i] == '\0')
            continue;
        putc(terminals[terminalid].charBuff[i]); // for loop and print chars
        chars_added++;
    }
    memset(terminals[terminalid].terminalBuff, 0,128);
    return 0;
}


//all this are driver functions for jumptable only.

int32_t terminal_open(char* filename, int32_t* inode){
    return terminalOpen((uint8_t*)filename);
}
int32_t terminal_close(int32_t* inode){
    return terminalClose(*inode);
}
int32_t terminal_write(int32_t *fd, void* buf, int32_t nbytes,int terminalid){
    return terminalWrite(fd, buf, nbytes,terminalid );
}

int32_t terminal_read(int32_t fd, int32_t *offset, uint8_t* buf, int32_t length, int terminalid){
    return terminalRead(fd,(void*)buf,length,terminalid);
}




// all these are helper interface function for accessing the state variable

int32_t getTerminalNumber() {
    return activeTerminal;
}

int32_t setTerminalNumber(uint32_t n) {
    if(n<0 || n >=3) return -1;
    activeTerminal = n;
    return 0;
}

int32_t getExecutingProcessInTerminal(uint32_t terminalId) {
    if(terminalId < 0 || terminalId >= numOfTerminal) return -1;
    return terminals[terminalId].pid;
}

int32_t setExecutingProcessInTerminal(uint32_t terminalId, int pid) {
    if(terminalId < 0 || terminalId >= numOfTerminal) return -1;
    terminals[terminalId].pid = pid;
    return 0;
}

