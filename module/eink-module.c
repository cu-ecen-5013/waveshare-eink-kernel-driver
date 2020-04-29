

#ifdef __KERNEL__
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/gpio.h>      // Required for the GPIO functions
#include <linux/interrupt.h> // Required for the IRQ code
#include <linux/spi/spi.h>
#include <linux/printk.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
// #define printf PDEBUG

#define AESD_DEBUG 1 // Remove comment on this line to enable debug

#undef PDEBUG /* undef it, just in case */
#ifdef AESD_DEBUG
#ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) printk(KERN_DEBUG "aesdchar: " fmt, ##args)
#else
/* This one for user space */
#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ##args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#endif

#include "epd1in54.h"
#include "epdif.h"
#include "Lucida_Console_8pts.h"
#include "drawFunctions.h"
#include "eink_ioctl.h"

struct aesd_dev {
  struct mutex lock; /* mutual exclusion semaphore */
  struct cdev cdev;  /* Char device structure		*/
};

int einkReset = RST_PIN;
int einkDC = DC_PIN;
int einkBusy = BUSY_PIN;

bool einkResetState = false;
bool einkDCState = false;
bool einkBusyState = false;

struct spi_device *spi_device;

int aesd_major = 0; // use dynamic major
int aesd_minor = 0;

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Prayag Desai");
MODULE_DESCRIPTION("A module to work with the e-ink display");
MODULE_VERSION("0.1");

#define BUF_LEN 80

struct aesd_dev aesd_device;

ssize_t eink_write(struct file *filp, const char __user *buf, size_t count,
                   loff_t *f_pos) {
  PDEBUG("Write\n");
  return count;
}

long eink_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  int i = 0, temp = 0;
  int tempX, tempX1, tempY, tempY1;
  bool partialUpdate;
  PDEBUG("EINK: IOCTL\n");
  /*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != EINK_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > EINKCHAR_IOC_MAXNR) return -ENOTTY;

  char bufIn[BUF_LEN];

  struct pixelDataIn *tempIn = kmalloc(sizeof(struct pixelDataIn), GFP_KERNEL);
  copy_from_user((void*)tempIn, (struct pixelDataIn __user *)arg, sizeof(struct pixelDataIn));



  tempX = tempIn->x;
  tempY = tempIn->y;
  tempX1 = tempIn->x1;
  tempY1 = tempIn->y1;

  bool disUpdate = tempIn->disableUpdate;

  partialUpdate = tempIn->partLUT;

  // PDEBUG("Line from IOCTL: X1: %d Y1: %d X2: %d Y2: %d\n", tempX, tempY, tempX1, tempY1);
  // drawLine(tempX, tempY, tempX1, tempY1, DISP_BLACK);

  switch (cmd)
  {
    case EINKCHAR_IOCWRCHAR:
      PDEBUG("String from IOCTL\n");
      char *in = kmalloc(tempIn->stringLength + 1, GFP_KERNEL);
      copy_from_user(in, tempIn->stringIn, tempIn->stringLength);
      in[tempIn->stringLength] = '\0';
      PDEBUG("String from IOCTL: %s @ X: %d Y: %d Length: %ld\n", in, tempX, tempY, tempIn->stringLength);
      mutex_lock_interruptible(&aesd_device.lock);
      writeString(tempX, tempY, DISP_BLACK, in);
      kfree(in);
      if(!disUpdate)
        updateDisplay();
      mutex_unlock(&aesd_device.lock);
      break;

    case EINKCHAR_IOCWRXYLINE:
      PDEBUG("Line from IOCTL: X1: %d Y1: %d X2: %d Y2: %d\n", tempX, tempY, tempX1, tempY1);
      mutex_lock_interruptible(&aesd_device.lock);
      drawLine(tempX, tempY, tempX1, tempY1, DISP_BLACK);
      if(!disUpdate)
        updateDisplay();
      mutex_unlock(&aesd_device.lock);
      break;

    case EINKCHAR_IOCWRLUT:
      if(partialUpdate) {
        PDEBUG("Partial LUT selected\n");
        mutex_lock_interruptible(&aesd_device.lock);
        Init(lut_partial_update);
        mutex_unlock(&aesd_device.lock);
      }
      else {
        PDEBUG("Full LUT selected\n");
        mutex_lock_interruptible(&aesd_device.lock);
        Init(lut_full_update);
        mutex_unlock(&aesd_device.lock);
      }
      break;
    
    case EINKCHAR_IOCWRSECTION:
      PDEBUG("Section write from IOCTL: *START* X1: %d Y1: %d  *SIZE* X2: %d Y2: %d\n", tempX, tempY, tempX1, tempY1);
      size_t sectionSize = tempX1 * tempY1;
      uint8_t *sectionPointer = kmalloc(sectionSize, GFP_KERNEL);
      copy_from_user(sectionPointer, tempIn->sectionData, sectionSize);
      
      mutex_lock_interruptible(&aesd_device.lock);
      int i, j;
      for(i = 0; i < tempY1; i ++)
      {
        for(j = 0; j < tempX1; j++)
        {
            int index = j + tempX1 * i;
            drawPixel(tempX + j, tempY + i, sectionPointer[index]);
        }
      }
      if(!disUpdate)
        updateDisplay(); 
      mutex_unlock(&aesd_device.lock);
      kfree(sectionPointer);
      break;

    case EINKCHAR_IOCWRPIXEL:
      mutex_lock_interruptible(&aesd_device.lock);
      drawPixel(tempX + tempX1, tempY + tempY1, DISP_BLACK);
      if(!disUpdate)
        updateDisplay(); 
      mutex_unlock(&aesd_device.lock);
      break;

    case EINKCHAR_IOCWRDISUPD:
      mutex_lock_interruptible(&aesd_device.lock);
      updateDisplay(); 
      mutex_unlock(&aesd_device.lock);
      break;

    default:
      PDEBUG("Invalid IOCTL command: %d\n", cmd);
      break;
  }

  kfree(tempIn);
  return 0;
}

struct file_operations aesd_fops = {
    .owner = THIS_MODULE,
    .write = eink_write,
    .unlocked_ioctl = eink_ioctl,
};

static int aesd_setup_cdev(struct aesd_dev *dev) {
  int err, devno = MKDEV(aesd_major, aesd_minor);

  cdev_init(&dev->cdev, &aesd_fops);
  dev->cdev.owner = THIS_MODULE;
  dev->cdev.ops = &aesd_fops;
  err = cdev_add(&dev->cdev, devno, 1);
  if (err) {
    printk(KERN_ERR "Error %d adding aesd cdev", err);
  }
  return err;
}

int __init eink_init(void) {
  dev_t dev = 0;
  int result;
  result = alloc_chrdev_region(&dev, aesd_minor, 1, "einkChar");
  aesd_major = MAJOR(dev);
  if (result < 0) {
    printk(KERN_WARNING "Can't get major %d\n", aesd_major);
    return result;
  }
  memset(&aesd_device, 0, sizeof(struct aesd_dev));


  mutex_init(&aesd_device.lock);

  // NO PULL UPS
  printk(KERN_INFO "GPIO INIT: Initializing the GPIO\n");

  if (!gpio_is_valid(einkReset)) {
    PDEBUG(KERN_INFO "GPIO_TEST: invalid LED GPIO\n");
    return -ENODEV;
  }
  if (!gpio_is_valid(einkDC)) {
    PDEBUG(KERN_INFO "GPIO_TEST: invalid LED GPIO\n");
    return -ENODEV;
  }
  if (!gpio_is_valid(einkBusy)) {
    PDEBUG(KERN_INFO "GPIO_TEST: invalid LED GPIO\n");
    return -ENODEV;
  }

  gpio_request(einkReset, "sysfs");
  gpio_request(einkDC, "sysfs");

  gpio_direction_output(einkReset, einkResetState);
  gpio_direction_output(einkDC, einkDCState);

  gpio_export(einkReset, false);
  gpio_export(einkDC, false);

  gpio_request(einkBusy, "sysfs");
  gpio_direction_input(einkBusy);
  gpio_set_debounce(einkBusy, 50);
  gpio_export(einkBusy, false);

  int ret;

  printk("beddsvfs to setup slave.\n");

  struct spi_master *master;

  // Register information about your slave device:
  struct spi_board_info spi_device_info = {
      .modalias = "eink-spi-driver",
      .max_speed_hz = 10000000, // speed your device (slave) can handle
      .bus_num = 0,
      .chip_select = 0,
      .mode = 0
  };

  /*To send data we have to know what spi port/pins should be used. This
  information can be found in the device-tree. */
  master = spi_busnum_to_master(spi_device_info.bus_num);
  if (!master) {
    printk("MASTER not found.\n");
    return -ENODEV;
  }
  printk("PADD to setup slave.\n");

  //  // create a new slave device, given the master and device info
  spi_device = spi_new_device(master, &spi_device_info);

  if (!spi_device) {
    printk("FAILED to create slave.\n");
    return -ENODEV;
  }

  spi_device->bits_per_word = 8;

  ret = spi_setup(spi_device);

  printk("MAGGG to setup slave.\n");

  if (ret) {
    printk("FAILED to setup slave.\n");
    spi_unregister_device(spi_device);
    return -ENODEV;
  }
  printk("jrfskm to setup slave.\n");

  Init(lut_full_update);
  ClearFrameMemory(0xFF); // bit set = white, bit reset = black
  DisplayFrame();

  int i, j;

  for (i = 0; i < 200; i++) {
    for (j = 0; j < 200 / 8; j++) {
      imageBuffer[i][j] = 0xFF;
    }
  }


  result = aesd_setup_cdev(&aesd_device);
  printk(KERN_ERR "-------------------------------------\n");
  if (result) {
    unregister_chrdev_region(dev, 1);
  }

  return 0;
}

void __exit eink_exit(void) {
  dev_t devno = MKDEV(aesd_major, aesd_minor);
  cdev_del(&aesd_device.cdev);

  gpio_unexport(einkReset);
  gpio_unexport(einkDC);
  gpio_unexport(einkBusy);

  gpio_free(einkReset);
  gpio_free(einkDC);
  gpio_free(einkBusy);

  if (spi_device)
    spi_unregister_device(spi_device);

  unregister_chrdev_region(devno, 1);
}

module_init(eink_init);
module_exit(eink_exit);

