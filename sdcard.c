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
  //#include "LynxGD.h"
  #include "LynxSD.h"
#endif


char sd_buf[256];			      // sdcard read buffer
char sd_dir[256];			      // sdcard destination directory
char sd_last_page;	  		  // sdcard directory entries last page flag
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
unsigned char sd_open_rootdir(void)
{
  memset(sd_dir, 0, 256);
  strcpy(sd_dir, "/");
  
  #ifdef SDCARD_BENNVENN
    unsigned char i;
    for(i=0; i<4; ++i) {
      bennvenn_send_command("BACK", 4);
    }

    //SDCARD_BENNVENN_send_command("ROOT", 4);

    bennvenn_send_command("DIR4XXX ", 8);
    bennvenn_file_count();
    sd_num_files = bennvenn_num_folders + bennvenn_num_files;
  #endif

  #ifdef SDCARD_GAMEDRIVE
    LynxSD_Init();
    if (LynxSD_OpenDir(sd_dir) == FR_OK) {
      gd_get_dir_entries();
    }
    else
      return(0);
  #endif

  return(1);
}


unsigned char sd_open_dir(unsigned int entry)
{
  #ifdef SDCARD_BENNVENN
  bennvenn_open(entry);
  bennvenn_send_command("DIR4XXX", 7);
  bennvenn_file_count();
  sd_num_files = bennvenn_num_folders + bennvenn_num_files;
  #endif

  #ifdef SDCARD_GAMEDRIVE
  strcpy(sd_buf, sd_dir);               // make a copy of sd_dir
  sd_buf[strlen(sd_buf)-1] = '\0';      // strip off trailing slash
  if (LynxSD_OpenDir(sd_buf) != FR_OK) {
    return(0);
  }
  gd_get_dir_entries();
  #endif

  return(1);
}


/* sd_get_entries
 *
 * Get the SD card diretory entries, and read 10 into filenames arrary for display
 */
void sd_get_entries(unsigned int dirpos)
{
  unsigned char i;


  sd_last_page = 0;						                      // reset last page flag
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


unsigned char sd_open_file(char *file)
{
	#ifdef SDCARD_GAMEDRIVE
	if (LynxSD_OpenFile(file) == FR_OK)
	  return(1);
	else
	  return(0);
	#endif
}


unsigned char sd_close_file(void)
{
	#ifdef SDCARD_GAMEDRIVE
	if (LynxSD_CloseFile() == FR_OK)
	  return(1);
	else
	  return(0);
	#endif
}


unsigned char sd_write_file_block(uint16_t size, char *buf)
{
	unsigned char n;
  
  n = 3;
  while (n) {
    #ifdef SDCARD_GAMEDRIVE
    if (LynxSD_WriteFile(buf, size) == FR_OK)
	    return(1);
	  #endif
    
    n--;
  }

  return(0);
}


unsigned char select_sdcard_dir(void)
{
  #ifndef SDCARD_NONE

  unsigned char r, st, scrdir;
  unsigned int delay;
  unsigned char joy, sel;
  unsigned int dirpos;
  unsigned char len;


  tgi_clear();
  draw_box_with_text(0, 0, 159, 92, TGI_COLOR_YELLOW, "Destination", "A=Open Dir B=Select");

  // Initialize and start at root directory
  sel = dirpos = 0;
  r = sd_open_rootdir();
  if (!r) {
    display_error_and_wait("Error opening sdcard!");
    return(0);
  }
  sd_get_entries(0);

  // input loop
  while(1) {
    st = scrdir = delay = 0;                       // reset start of entry, dir of scroll, delay
    display_files(sel);

    // Process joystick and keys
    do {
      r = check_joy_and_keys(&joy);
      if (r == '2')       // opt2 exits
        return(0);
      if (r == '1')
        return(1);        // opt1 selects

      // Need to scroll this entry?
      len  = strlen(filenames[sel]);
      if ((delay == SCROLL_DELAY) && (len > 19)) {
        if (scrdir)
          st--;       // backward
        else
          st++;       // forward

        if (st > (len-20))
          scrdir = 1;                                   // reverse scroll dir
        else if (st == 0)
          scrdir = 0;                                   // forward direction

        scroll_file_entry(sel, st);						// scroll it!
        delay = 0;
      }

      delay++;
    } while (!r && !joy);

	// Process the joystick input
    if (JOY_UP(joy)) {
      if (sel != 0)							      // not at top of page
        --sel;
      else {								          // at top of page
	      if (dirpos > 9) {					    // only if we aren't at top of directory already!
	        joy = JOY_LEFT_MASK;		    // set joy left, and let it process down below
	      }
      }
    }
    if (JOY_DOWN(joy)) {
      if (sel < 9) {                        // last entry on page?
        ++sel;
        if (filenames[sel][0] == '\0')      // blank entry? don't increment
          --sel;
      }
      else {
	      joy = JOY_RIGHT_MASK;				        // set joystick right and let it process down below
      }
    }
    if (JOY_LEFT(joy)) {
      // go back previous page?
      sel = 0;
      if (dirpos > 9) {
        dirpos -= 10;                   	      // get previous page of entries
        sd_get_entries(dirpos);				          // get new directory entries
      }
      else {
      	dirpos = 0;							                // back at root
      	#ifdef SDCARD_BENNVENN
          bennvenn_send_command("BACK", 4);   	// back up a directory
          sd_open_dir(dirpos);        		      // open the dir
          sd_get_entries(dirpos);				        // get new directory entries
      	#endif
      	#ifdef SDCARD_GAMEDRIVE
			    if (sd_dir[1] != '\0') {			        // at root already?
            strip_dir_from_path((char *) &sd_dir);       // back up a directory
			    }
          r = sd_open_dir(dirpos);				       // open the previous directory
          if (!r) {
            display_error_and_wait("Error opening sdcard!");
            return(0);
          }
          
          sd_get_entries(dirpos);				        // get new directory entries
      	#endif
  	  }

      continue;							    	              // restart the loop
    }
    if (JOY_RIGHT(joy)) {
      if (!sd_last_page) {						          // if not at last page, get next page
        if ((dirpos + 10) <= sd_num_files) {
          dirpos += 10;
          sd_get_entries(dirpos);
          sel = 0;
          continue;								                // restart the loop
        }
      }
    }

    // B button selects the directory
    if (JOY_BTN_2(joy)) {
      // FIXME: Do something here to record our destination (actually, sd_dir should already be set)

      return(1);                               // exit to host select, directory selected
    }

    // A button selects directory or file
    if (JOY_BTN_1(joy)) {
      // Is this a directory?
      //if (filenames[sel][strlen(filenames[sel])-1] == '/') {
      if (path_is_dir(filenames[sel])) {
        strcat(sd_dir, filenames[sel]);       	// add to directory string

        r = sd_open_dir(dirpos+sel);        		// open the dir  - GameDrive code ignores passed in dir pos
        if (!r) {
          display_error_and_wait("Error opening sdcard!");
          return(0);
        }

        dirpos = sel = 0;
        sd_get_entries(dirpos);
      }
      else {
        // FIXME: on gamedrive, overwrite the file selected?
        return(1);
      }
    }
  }

  #endif		// SDCARD_NONE
}
