#include "keyboard.h"

#define SCANCODE_SIZE   0x59
#define SCANCODE_LIMT   0x80


// special key flags
int CTRL = 0;
int SHIFT = 0;
int CAPSLOCK = 0;
int ALT = 0;

// for color changing
int hex = 0x01;

// scancode set 1
char scancode[SCANCODE_SIZE][2] = {
    { 0 , 0 },{ 0 , 0 },{'1','!'},{'2','@'},
    {'3','#'},{'4','$'},{'5','%'},{'6','^'},
    {'7','&'},{'8','*'},{'9','('},{'0',')'},
    {'-','_'},{'=','+'},{'\b','\b'},{' ',' '},
    {'q','Q'},{'w','W'},{'e','E'},{'r','R'},
    {'t','T'},{'y','Y'},{'u','U'},{'i','I'},
    {'o','O'},{'p','P'},{'[','{'},{']','}'},
    {'\n','\n'},{ 0 , 0 },{'a','A'},{'s','S'},
    {'d','D'},{'f','F'},{'g','G'},{'h','H'},
    {'j','J'},{'k','K'},{'l','L'},{';',':'},
    {'\'','\"'},{'`','~'},{ 0, 0 },{'\\','|'},
    {'z','Z'},{'x','X'},{'c','C'},{'v','V'},
    {'b','B'},{'n','N'},{'m','M'},{',','<'},
    {'.','>'},{'/','?'},{ 0 , 0 },{'*', 0 },
    { 0 , 0 },{' ',' '},{ 0 , 0 },{ 0 , 0 },
    { 0 , 0 },{ 0 , 0 },{ 0 , 0 },{ 0 , 0 },
    { 0 , 0 },{ 0 , 0 },{ 0 , 0 },{ 0 , 0 },
    { 0 , 0 },{ 0 , 0 },{ 0 , 0 },{'7', 0 },
    {'8', 0 },{'9', 0 },{'-', 0 },{'4', 0 },
    {'5', 0 },{'6', 0 },{'+', 0 },{'1', 0 },
    {'2', 0 },{'3', 0 },{'0', 0 },{'.', 0 },
    { 0 , 0 },{ 0 , 0 },{ 0 , 0 },{ 0 , 0 },
    { 0 , 0 }
};

/* keyboard_init(void)
 * Initialize keyboard
 * input: none
 * output: none
 * side effect: start up keyboard irq
 */
void keyboard_init(void) {
    enable_irq(KEY_IRQ_PORT);
}

/* keyboard_intr(void)
 * deal with intr
 * input: none
 * output: none
 * side effect: translation from scancode to ascii also handles flags caused by some scancodes
 */
void keyboard_intr(void) {
    uint8_t keyidx;
    char key;
    int32_t prev_executing_term_idx = getExecutingTerminalIndex();
    setExecutingTerminalIndex(activeTerminal);
    setVideoMemFromTermId(getExecutingTerminalIndex());

    keyidx = inb(KEY_DATA_PORT);
    term_t *showing_term = &(terminals[activeTerminal]);

    //printf("keyboard get intr ack\n");
    switch(keyidx){
    case 0x0F : // TAB
        key = scancode[keyidx][SHIFT];
        int i;
        for(i=0;i<SPACE_PER_TAB;i++){ // number of spaces in tab
            showing_term->terminalBuff[showing_term->numchar] = key;
            showing_term->numchar += 1;
            keyboard_print(key);
        }
        break;
    case 0x01: // ESCAPE CHAR DOES COLOUR CHANGE
        hex++;
        if(hex > 8)// value of atttrib
            hex = 1;
        changecolour(hex);  
        break;  
    case 0x0E: // BACKSPACE
        if(showing_term->numchar == 0) break;
        showing_term->terminalBuff[showing_term->numchar] = '\0';
        showing_term->numchar--;
        backspace();
        break;    
    case 0x1D: // CTRL
        CTRL = 1;
        break;
    case 0x9D: // CTRL RELEASE
        CTRL = 0;
        break;
    case 0x2A: case 0x36: // LSHIFT AND RSHIFT
        SHIFT = (CAPSLOCK) ? 0 : 1;
        break;
    case 0xAA: case 0xB6: //LSHIFT AND RSHIFT RELEASE
        SHIFT = (CAPSLOCK) ? 1 : 0;
        break;
    case 0x3A: // CAPSLOCK
        CAPSLOCK = CAPSLOCK^1;
        SHIFT = SHIFT^1;
        break;
    case 0x38: // ALT
        ALT = 1;
        break;
    case 0xB8: // ALT RELEASE
        ALT = 0;
        break;
    case 0x3B: // FN1
        if(ALT) switchToTerminal(0);
        break;
    case 0x3C: //FN 2
        if(ALT) switchToTerminal(1);
        break;
    case 0x3D: // FN3
        if(ALT) switchToTerminal(2);
        break;
    case 0x26: // L
        if(CTRL){
            CTRLL(activeTerminal);
            break;
        }	
    default:
        key = scancode[keyidx][SHIFT]; // SCAN CODE to ASCII
        if(showing_term->numchar == bufferSize || key == 0) // values we want to ignore
            break;
        
        // handle enter key press
        // dump terminalBuff into writeBuff and clear terminalBuff
        if(key == '\n'){
            showing_term->ENTER = 0;
            showing_term->terminalBuff[showing_term->numchar] = key;
            memcpy(showing_term->writeBuff,showing_term->terminalBuff,bufferSize);
            memset(showing_term->terminalBuff, 0, bufferSize);
            keyboard_print(key);
            break;
        }

        if(showing_term->numchar == bufferSize -1)
            break;
            
        if (keyidx < SCANCODE_LIMT) {
            if (keyidx == RTC_START) {
                rtc_init();
            } 
            key = scancode[keyidx][SHIFT]; // print lowcase
            showing_term->terminalBuff[showing_term->numchar] = key;
            showing_term->numchar++;
            keyboard_print(key);
            break;  
        }
    }

    setExecutingTerminalIndex(prev_executing_term_idx);
    setVideoMemFromTermId(getExecutingTerminalIndex());
    send_eoi(1); //key borad intr stop
}

/* keyboard_print(char key)
 * print key
 * input: key
 * output: none
 * side effect: print key
 */
void keyboard_print(char key) {
    printf("%c", key);
}
