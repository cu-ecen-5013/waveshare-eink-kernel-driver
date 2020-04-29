#ifndef DRAWFUCNTIONS_H
#define DRAWFUCNTIONS_H

#ifndef __KERNEL__
#include <stdint.h>
#include <stdbool.h>
#else
#include <linux/types.h>
#include <linux/printk.h>

// #define printf PDEBUG

#define AESD_DEBUG 1  //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef AESD_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "aesdchar: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#endif

#include "epd1in54.h"
#include "epdif.h"
#include "Lucida_Console_8pts.h"

// uint8_t imageBuf[5000] = {0};
extern uint8_t imageBuffer[200][25];

#define DISP_WHITE 0
#define DISP_BLACK 1

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

extern void drawPixel(uint8_t x, uint8_t y, bool color);
extern void drawLineX(uint16_t startPosX, uint16_t length, uint16_t startPosY, bool color);
extern void drawLineY(uint16_t startPosX, uint16_t length, uint16_t startPosY, bool color);
extern void writeLetter(uint16_t posX, uint16_t posY, bool color, char inChar);
extern void writeString(uint16_t posX, uint16_t posY, bool color, char *inString);
extern void drawLine(int x0, int y0, int x1, int y1, bool color);
extern void updateDisplay(void);

#endif