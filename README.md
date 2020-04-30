# waveshare-eink-kernel-driver
Repository which holds a port of Waveshare e-ink Arduino source to a Linux Kernel driver for use with AESD Spring 2020 semester

## The wiki
You will find an overview of this repo in the wiki page, some explaination for using GPIO and SPI in kernel space, 
implementing drawing algorithms for a bitmap display, and creating a device tree overlay in order to disable spidev0.0 
so a custom SPI module can be loaded. You will also find a link to a pull request which should show you what exact changes were 
made in the buildroot configuration in order to get this working.

Author: Prayag Milan Desai
