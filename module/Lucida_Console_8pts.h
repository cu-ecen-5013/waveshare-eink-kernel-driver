/*
 * Lucida_Console_8pts.h
 *
 *  Created on: 03-Dec-2019
 *      Author: prayag
 */

#ifndef LUCIDA_CONSOLE_8PTS_H_
#define LUCIDA_CONSOLE_8PTS_H_

#ifndef __KERNEL__
#include <stdint.h>
#include "bitmap_db.h"
#else
#include "bitmap_db.h"
#include <linux/types.h>
#endif

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

/* Font data for Lucida Console 8pt */
extern const uint8_t lucidaConsole_8ptBitmaps[];
extern const FONT_INFO lucidaConsole_8ptFontInfo;
extern const FONT_CHAR_INFO lucidaConsole_8ptDescriptors[];

extern const uint8_t lucidaConsole_8ptBitmaps_narrow[];
extern const FONT_INFO lucidaConsole_8ptFontInfo_narrow;
extern const FONT_CHAR_INFO lucidaConsole_8ptDescriptors_narrow[];

#endif /* LUCIDA_CONSOLE_8PTS_H_ */