#include "rtc.h"
/* rtc_init()
    description: init the RTC by setting Periodic Interrupt Enable (PIE) to 1
    reference  from wiki.osdev.org/RTC
*/
static volatile uint32_t rtc_interrupt_counter; // golbal flag for rtc interrept
//static int32_t rtc_freq_flag;//golbal flag for rtc freq

void rtc_init()
{
  
    uint8_t prev,mask;
    mask=0x40; // Periodic Interrupt Enable (PIE) is bit 6

    cli(); //disable interrupt
    outb(RTC_REG_B,RTC_PORT_INDEX);     //select reg B
    prev = inb(RTC_PORT_DATA);   //read prev reg B

    prev = prev | mask;               //enable PIE
    outb(RTC_REG_B,RTC_PORT_INDEX);     //select reg B
    outb(prev,RTC_PORT_DATA);     // write data back to reg B
    enable_irq(RTC_IRQ);
    sti(); //enable interrupt
}

/* rtc_freq()
    description: Changing Interrupt Rate by changing Rate Selector (RS3, RS2, RS1, RS0)
    input: desired freq (choose from 2,4,8,16,....,1024 Hz)
    reference  from wiki.osdev.org/RTC
*/
void rtc_freq(int32_t freq){
    uint8_t rs_data,prev,mask;
    mask=0xF0; // Rate Selector (RS3, RS2, RS1, RS0) 3:0
    switch(freq) {      //setting up RS bits accroding to datasheet
        case 1024: rs_data= 0x06; break;
        case 512:  rs_data= 0x07; break;
        case 256:  rs_data= 0x08; break;
        case 128:  rs_data= 0x09; break;
        case 64:   rs_data= 0x0A; break;
        case 32:   rs_data= 0x0B; break;
        case 16:   rs_data= 0x0C; break;
        case 8:    rs_data= 0x0D; break;
        case 4:    rs_data= 0x0E; break;
        case 2:    rs_data= 0x0F; break;
        default:   rs_data= 0x0F; 
    }
    cli(); //disable interrupt
    outb(RTC_REG_A,RTC_PORT_INDEX);     //select reg A
    prev=inb(RTC_PORT_DATA);    //read prev reg B value  
    prev=prev & mask;               //clear prev freq
    prev=prev | rs_data;               //set new freq
    outb(RTC_REG_A,RTC_PORT_INDEX);     //select reg A
    outb(prev,RTC_PORT_DATA);     // write data back to reg A
    sti(); //enable interrupt
}

/* rtc_interrupt()
    description: interrupt handler function when IRQ8 are called.
    for testing , it print signal when called
*/
void rtc_interrupt() {
    //testing code

    //fuction code
    rtc_interrupt_counter ++;
    
    cli(); //disable interrupt
    outb(RTC_REG_C,RTC_PORT_INDEX);     //select reg C
    inb(RTC_PORT_DATA);    //dump reg C to continue 
    sti(); //enable interrupt
    send_eoi(RTC_IRQ); //done
}


// RTC Driver

/* int32_t  rtc_open(const uint8_t* filename)
    open and set to freq(2)
    input: *inode - postion where freq are stored
    return:0 for success,-1 for fail
*/
int32_t rtc_open(char* filename, int32_t* inode){
    rtc_freq(1024);
    *inode = 2;
    //rtc_freq_flag=2;
    return 0;
}
/* int32_t rtc_read( int32_t fd, void* buf, int32_t nbytes )
    return 0 until the RTC ticks
    input: fd-freq of rtc
    return:0 for success -1 for fail
*/
int32_t rtc_read(int32_t fd, int32_t *offset, uint8_t* buf, int32_t length){
    uint32_t counter = rtc_interrupt_counter; //log counter
    if (fd < 2 || fd > 1024 ) return -1; // fail check for freq too low or high
    while(1){
        //if (rtc_interrupt_counter >= (uint32_t)(1024 / rtc_freq_flag)) {return 0;}// stop when tick is reached
        if (rtc_interrupt_counter < counter || rtc_interrupt_counter>= counter + (uint32_t)(1024/fd)) {return 0;}// stop when counter is reached
    }

}
/* int32_t rtc_write( int32_t *fd, void* buf, int32_t nbytes )
    set RTC freq
    input: buf int32_t freq  ,fd pointer to freq stored
    return:0 for success -1 for fail
*/
int32_t rtc_write( int32_t *fd, void* buf, int32_t nbytes ){
    if (*(int32_t*)buf < 2 || *(int32_t*)buf > 1024 ) return -1; // fail check for freq too low or high
    *fd=*(int32_t*)buf;
    //rtc_freq(1024);//set to highest freq
    //rtc_freq_flag=(*(int32_t*)buf);
    
    return 0;
}
/* int32_t  rtc_close(const uint8_t* filename)
    only return 0;
    input: not used
    return:0 
*/
int32_t rtc_close(int32_t* inode){
    return 0;
}
