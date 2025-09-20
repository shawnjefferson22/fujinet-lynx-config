#ifndef SDCARD_H
#define SDCARD_H


// Type of SD card to build for
//#define SDCARD_BENNVENN
//#define SDCARD_LYNXGD
//#define SDCARD_NONE

extern char sd_buf[256];			// sdcard read buffer
extern char sd_dir[256];			// sdcard destination directory


void open_sd_rootdir(void);
void open_sd_dir(unsigned int entry);
void get_sd_entries(unsigned int dirpos);
unsigned char select_sdcard_dir(void);


#endif