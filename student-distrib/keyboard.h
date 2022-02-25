/* keyboard.h - Defines keyboard related function used in intr
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"
#include "rtc.h"
#include "i8259.h"
#include "lib.h"
#include "terminal.h"

#define KEY_IRQ_PORT    0X01
#define KEY_DATA_PORT   0X60
#define RTC_START       0X1A
#define RTC_STOP        0x1b
#define SPACE_PER_TAB   4

/* Initialize keyboard */
void keyboard_init(void);
/* deal with intr */
void keyboard_intr(void);
/* print key */
void keyboard_print(char key);

extern int ENTER;
extern int numchar[];

#endif /* _KEYBOARD_H */
