#ifndef SDCARD_H
#define SDCARD_H


// Type of SD card to build for
//#define SDCARD_BENNVENN
//#define SDCARD_LYNXGD
//#define SDCARD_NONE

extern char sd_buf[256];			// sdcard read buffer
extern char sd_dir[256];			// sdcard destination directory


unsigned char sd_open_rootdir(void);
unsigned char sd_open_dir(unsigned int entry);
void sd_get_dir_entries(unsigned int dirpos);
unsigned char select_sdcard_dir(void);
unsigned char sd_open_file(char *file);
unsigned char sd_close_file(void);
unsigned char sd_write_file_block(uint16_t size, char *buf);


#endif