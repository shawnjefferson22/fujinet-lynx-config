#include <6502.h>
#include <lynx.h>
#include <conio.h>
#include <joystick.h>
#include <tgi.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "display.h"
#include "input.h"
#include "sdcard.h"
#include "pathutil.h"


#ifdef SDCARD_BENNVENN
  #include "bennvenn.h"
#endif

#ifdef SDCARD_GAMEDRIVE
  #include "LynxSD.h"
#endif


char sd_buf[256];			// sdcard read buffer
char sd_dir[256];			// sdcard destination directory
char sd_last_page;	  		// sdcard directory entries last page flag
unsigned int sd_num_files;  // total number of files and folders in directory


// gamedrive directory entry
#ifdef SDCARD_GAMEDRIVE
  SFileInfo gd_direntry;

  struct {
    char filename[13];
    uint8_t is_dir;
  }  gd_dir_entry[255];
#endif


#ifdef SDCARD_GAMEDRIVE
void gd_get_dir_entries(void)
{
	sd_num_files = 0;
  memset(&gd_dir_entry, 0, sizeof(gd_dir_entry));

	while ((LynxSD_ReadDir(&gd_direntry) == FR_OK) && (sd_num_files < 255)) {
    if ((gd_direntry.fattrib & AM_HID) || (gd_direntry.fattrib & AM_SYS) || (gd_direntry.fattrib & AM_VOL) ||
          (gd_direntry.fattrib & AM_LFN)) {
			continue;
		}

		strcpy(gd_dir_entry[sd_num_files].filename, gd_direntry.fname);
		if (gd_direntry.fattrib & AM_DIR)
			gd_dir_entry[sd_num_files].is_dir = 1;
		else
			gd_dir_entry[sd_num_files].is_dir = 0;

    sd_num_files++;
  }
}
#endif	// SDCARD_GAMEDRIVE


/* sd_open_rootdir
 *
 * Open the SD root directory, if required.
 */
bool sd_open_rootdir(void)
{
  	unsigned char i;

  memset(sd_dir, 0, 256);
  strcpy(sd_dir, "/");

  #ifdef SDCARD_BENNVENN
    for(i=0; i<4; ++i) {						// hack, since ROOT doesn't seem to work
      bennvenn_send_command("BACK", 4);
    }
    //bennvenn_send_command("ROOT", 4);

    bennvenn_send_command("DIR4XXX ", 8);
    bennvenn_file_count();
    sd_num_files = bennvenn_num_folders + bennvenn_num_files;
  #endif

  #ifdef SDCARD_GAMEDRIVE
    (void) i;
    LynxSD_Init();
    if (LynxSD_OpenDir(sd_dir) == FR_OK) {
      gd_get_dir_entries();
    }
    else
      return(false);
  #endif

  return(true);
}


bool sd_open_dir(unsigned int entry)
{
  #ifdef SDCARD_BENNVENN
  bennvenn_open(entry);
  bennvenn_send_command("DIR1LNX ", 8);
  bennvenn_file_count();
  sd_num_files = bennvenn_num_folders + bennvenn_num_files;
  #endif

  #ifdef SDCARD_GAMEDRIVE
  (void) entry;
  strcpy(sd_buf, sd_dir);               // make a copy of sd_dir
  sd_buf[strlen(sd_buf)-1] = '\0';      // strip off trailing slash
  if (LynxSD_OpenDir(sd_buf) != FR_OK) {
    return(false);
  }
  gd_get_dir_entries();
  #endif

  return(true);
}


/* sd_get_entries
 *
 * Get the SD card diretory entries, and read 10 into filenames arrary for display
 */
void sd_get_entries(unsigned int dirpos)
{
  unsigned char i;


  sd_last_page = 0;						                // reset last page flag
  memset(&filenames[0], 0, sizeof(filenames));			// clear filenames array

  if (dirpos >= (sd_num_files)) {
    sd_last_page = 1;
    return;
  }

  #ifdef SDCARD_BENNVENN
    bennvenn_set_dir_pos(dirpos);				// set the dirpos to read from
  #endif

  for(i=0; i<10; ++i) {
    if ((dirpos + i) > sd_num_files) {
      sd_last_page = 1;
      return;
    }

    #ifdef SDCARD_BENNVENN
      bennvenn_read_next_dir_entry((char *) &filenames[i]);

      if (filenames[i][0] == '\0') {				// this must be the last page
        sd_last_page = 1;
        return;
      }
    #endif	// SDCARD_BENNVENN

    #ifdef SDCARD_GAMEDRIVE
	    strcpy(filenames[i], gd_dir_entry[dirpos+i].filename);
	    if (gd_dir_entry[dirpos+i].is_dir)
	  	  strcat(filenames[i], "/");
    #endif	// SDCARD_GAMEDRIVE
  }
}


bool sd_create_file(char *file, uint32_t size)
{
	#ifdef SDCARD_BENNVENN
		bennvenn_new_file(extract_filename(file), size);
	#else
		(void) file;
		(void) size;
	#endif

	return(true);
}


bool sd_open_file(char *file)
{
	#ifdef SDCARD_GAMEDRIVE
		if (LynxSD_OpenFile(file) == FR_OK)
	  	return(true);
		else
	  	return(false);
	#else
		(void) file;
		return(true);
	#endif
}


bool sd_close_file(void)
{
	#ifdef SDCARD_GAMEDRIVE
	if (LynxSD_CloseFile() == FR_OK)
	  return(true);
	else
	  return(false);
	#else
		return(true);
	#endif
}


bool sd_write_file_block(char *file, uint32_t offset, uint16_t size, char *buf)
{
	  unsigned char n, r;
	  unsigned int s, transfer;
    //char fn[12];
    //uint32_t off;
    //char str[40];

  	n = 3;
  	while (n) {
    	#ifdef SDCARD_GAMEDRIVE
    	(void) file;
    	(void) offset;
    	(void) s;
    	(void) transfer;

    	if (LynxSD_WriteFile(buf, size) == FR_OK)
	    	return(true);
	  	#endif

		#ifdef SDCARD_BENNVENN
	    //r = extract_and_pad_filename(extract_filename(file), fn);
      //if (!r)
		  //  return(false);

      //012345678901234567890
  	  //SAVEFILENAMEEXT12341DATA
      //sprintf(sd_buf, "SAVE%8.8s%3.3s", fn, extract_ext(file));

      for (s = 0; s < size; s += transfer) {
			  transfer = (size - s > BV_MAX_WRITE) ? BV_MAX_WRITE : (size - s);
        //off = (uint32_t) (offset + (uint32_t) s);

        //sprintf(str, "o:%-5ld s:%-3d", off, transfer);
        //tgi_outtextxy(0, 50, str);
        //cgetc();

  	    //sd_buf[15] = (unsigned char) ((off >> 24) & 0xFF);
  	    //sd_buf[16] = (unsigned char) ((off >> 16) & 0xFF);
  	    //sd_buf[17] = (unsigned char) ((off >> 8) & 0xFF);
  	    //sd_buf[18] = (unsigned char) (off & 0xFF);

  	    //sd_buf[19] = (uint8_t) transfer;
  	    //memcpy(&sd_buf[20], &buf[s], transfer);

  	    //bennvenn_send_command_noreply(sd_buf, 20 + (uint8_t) transfer);

			  if (!bennvenn_save(extract_filename(file), (uint32_t) (offset + (uint32_t) s), (uint8_t ) transfer, &buf[s]))
			    	continue;   // try again
			}
      
      // must have been successful
      return(true);
		#endif

    	n--;
  	}

  	return(false);
}

#ifdef SDCARD_BENNVENN

/* sd_find_filenum
 *
 * Return the file number of the file matching filename (mainly for BennVenn)
 */
unsigned int sd_find_filenum(char *filename)
{
	char tmp[sizeof(BV_FILE_STRUCT)];
	unsigned int i;


  tgi_outtextxy(0, 40, filename);

  bennvenn_send_command("DIR1LNX ", 8);
	bennvenn_set_dir_pos(0);
  i = 0;

  while (1) {
  	memset(tmp, 0, 64);

   	bennvenn_read_next_dir_entry((char *) &tmp);
    //tgi_outtextxy(0, 50, tmp);
  
    if (tmp[0] == '\0')
		  break;

	  if (strcmp(filename, tmp) == 0)
		  return(i);
  
    //cgetc();  
    i++;
	}

	return(16384);
}
#endif


#define MSTERE0 ((volatile uint8_t *) 0xFD50)
#define MAPCTL ((volatile uint8_t *) 0xFFF9)

#ifndef SDCARD_NONE
void reset_lynx()
{
		uint8_t *ptr;
		uint8_t count;

    #ifdef SDCARD_GAMEDRIVE
		  if (bLaunchLowPower) LynxSD_LowPowerMode();
    #endif

		asm("sei");
		*MSTERE0 = 0; // enable all audio channels
		*MAPCTL = 0; // memory mapping for boot state

		ptr = (uint8_t *) 0xfd00; // timers and audio fd00
		count = 0x40;//40
		while (count--)
		{
			*ptr++ = 0;
		}

		*((uint8_t*) 0xFD80) = 0;
		*((uint8_t*) 0xFD81) = 0;
		*((uint8_t*) 0xFD92) = 0;
		*((uint8_t*) 0xFD9C) = 0;
		*((uint8_t*) 0xFD9D) = 0;
		*((uint8_t*) 0xFD9E) = 0;
		*((uint8_t*) 0xFD9D) = 0;

		ptr = (uint8_t*) 0xfda0; // palette
		count = 0x20;
		while (count--)
		{
			*ptr++ = 0;
		}

		asm("brk");
}
#endif
