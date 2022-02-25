#include "lib.h"
#include "i8259.h"

#define RTC_PORT_INDEX 0x70
#define RTC_PORT_DATA 0x71
#define RTC_IRQ 0x08

//index with NMI bit
#define RTC_REG_A 0x8A
#define RTC_REG_B 0x8B
#define RTC_REG_C 0x8C

void rtc_init();
void rtc_freq(int32_t freq);
void rtc_interrupt();

int32_t rtc_open(char* filename, int32_t* inode);
int32_t rtc_read(int32_t fd, int32_t *offset, uint8_t* buf, int32_t length);
int32_t rtc_write( int32_t *fd, void* buf, int32_t nbytes );
int32_t rtc_close(int32_t* inode);
