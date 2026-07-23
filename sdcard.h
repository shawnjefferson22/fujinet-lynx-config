#ifndef SDCARD_H
#define SDCARD_H

#include <stdbool.h>

extern char sd_buf[256];			// sdcard read buffer
extern char sd_dir[256];			// sdcard destination directory
extern char sd_last_page;	  		// sdcard directory entries last page flag
extern unsigned int sd_num_files;   // total number of files and folders in directory


bool sd_open_rootdir(void);
bool sd_open_dir(unsigned int entry);
void sd_get_dir_entries(unsigned int dirpos);
void sd_get_entries(unsigned int dirpos);
bool sd_open_file(char *file);
bool sd_close_file(void);
bool sd_write_file_block(char *file, uint32_t offset, uint16_t size, char *buf);

bool sd_create_file(char *file, uint32_t size);
unsigned int sd_find_filenum(char *filename);

#endif