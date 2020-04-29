/*
 * eink_ioctl.h
 */

#ifndef EINK_IOCTL_H
#define EINK_IOCTL_H

#ifdef __KERNEL__
#include <asm-generic/ioctl.h>
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdbool.h>
#endif

struct pixelDataIn {
    // First set of X-Y co-ordinates
    // Used for pixel, line, and section update IOCTLs
    // Start co-ord for line, text and section update
    int x;
    int y;

    // Second set of X-Y co-ordinates
    // Used for pixel, line and section update IOCTLs
    // End co-ord for line
    // X-Y size of the section
    int x1;
    int y1;
    int lineLength;

    // String to be written and length of the string
    char* stringIn;
    unsigned long stringLength;

    // Boolean which specifies if the display update is full or partial
    bool partLUT;

    bool disableUpdate;

    // Should contain an X-Y array of pixel value
    uint8_t *sectionData;
};

// Pick an arbitrary unused value from https://github.com/torvalds/linux/blob/master/Documentation/ioctl/ioctl-number.rst
#define EINK_IOC_MAGIC 0x16

// Define a write command from the user point of view, use command number 1
#define EINKCHAR_IOCWRCHAR _IOW(EINK_IOC_MAGIC, 1, struct pixelDataIn)
#define EINKCHAR_IOCWRXYLINE _IOW(EINK_IOC_MAGIC, 2, struct pixelDataIn)
#define EINKCHAR_IOCWRLUT _IOW(EINK_IOC_MAGIC, 3, bool)
#define EINKCHAR_IOCWRSECTION _IOW(EINK_IOC_MAGIC, 4, uint8_t*)
#define EINKCHAR_IOCWRPIXEL _IOW(EINK_IOC_MAGIC, 5, struct pixelDataIn)
#define EINKCHAR_IOCWRDISUPD _IOW(EINK_IOC_MAGIC, 6, struct pixelDataIn)
#define EINKCHAR_IOCWRPCLEAR _IO(EINK_IOC_MAGIC, 7, NULL)

/**
 * The maximum number of commands supported, used for bounds checking
 */
#define EINKCHAR_IOC_MAXNR 7

#endif /* EINK_IOCTL_H */