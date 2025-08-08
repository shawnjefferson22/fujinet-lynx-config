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

#ifdef BENNVENN
  #include "bennvenn.h"
#endif

char sd_buf[256];			// sdcard read buffer
char sd_dir[256];			// sdcard destination directory
char sd_last_page;			// sdcard directory entries last page flag



/* open_sd_rootdir
 *
 * Open the SD root directory, if required.
 */
void open_sd_rootdir(void)
{
  #ifdef BENNVENN
    unsigned char i;
    for(i=0; i<4; ++i) {
      bennvenn_send_command("BACK", 4);
    }

    //bennvenn_send_command("ROOT", 4);

    bennvenn_send_command("DIR4XXX ", 8);
    bennvenn_file_count();
  #endif
}


void open_sd_dir(unsigned int entry)
{
   #ifdef BENNVENN
     bennvenn_open(entry);
     bennvenn_send_command("DIR4XXX", 7);
     bennvenn_file_count();
  #endif
}


/* get_sd_entries
 *
 * Get the SD card diretory entries, and read 10 into filenames arrary for display
 */
void get_sd_entries(unsigned int dirpos)
{
  unsigned char i;


  sd_last_page = 0;						// reset last page flag
  memset(&filenames[0], 0, sizeof(filenames));			// clear filenames array

  #ifdef BENNVENN
    if (dirpos >= bennvenn_num_folders) {
      sd_last_page = 1;
      return;
    }

    bennvenn_set_dir_pos(dirpos);				// set the dirpos to read from
  #endif

  for(i=0; i<10; ++i) {
    #ifdef BENNVENN
      if ((dirpos + i) > bennvenn_num_folders) {
        sd_last_page = 1;
        return;
      }

      bennvenn_read_next_dir_entry((char *) &filenames[i]);

      if (filenames[i][0] == '\0') {				// this must be the last page
        sd_last_page = 1;
        return;
      }
    #endif
  }
}



unsigned char select_sdcard_dir(void)
{
  unsigned char r, st, scrdir;
  unsigned int delay;
  unsigned char joy, sel;
  unsigned int dirpos;


//REDRAW:
  tgi_clear();
  draw_box_with_text(0, 0, 159, 92, TGI_COLOR_YELLOW, "Destination", "A=Open Dir B=Select");

  // Initialize and start at root directory
  sel = dirpos = 0;
  memset(sd_dir, 0, 256);
  strcpy(sd_dir, "/");
  open_sd_rootdir();
  get_sd_entries(0);

  // input loop
  while(1) {
    st = scrdir = delay = 0;                       // reset start of entry, dir of scroll, delay
    display_files(sel);

    // Process joystick and keys
    do {
      r = check_joy_and_keys(&joy);

      // Need to scroll this entry?
      if ((delay == SCROLL_DELAY) && (strlen(filenames[sel]) > 19)) {
        if (scrdir)
          st--;       // backward
        else
          st++;       // forward

        if (st > (strlen(filenames[sel])-20))
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
      if (sel != 0)							  // not at top of page
        --sel;
      else {								      // at top of page
	    if (dirpos > 9) {					  // only if we aren't at top of directory already!
	      joy = JOY_LEFT_MASK;			// set joy left, and let it process down below
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
	    joy = JOY_RIGHT_MASK;				// set joystick right and let it process down below
      }
    }
    if (JOY_LEFT(joy)) {
      // go back previous page?
      if (dirpos > 9) {
        dirpos -= 10;                   // get previous page of entries
        get_sd_entries(dirpos);		  	  // get more directory entries
        sel = 0;                      	// set selected at zero
        continue;                     	// restart loop
      }
      // go back to previous directory?
      //if (sd_dir[1] != '\0') {				  	  // at root already?
        //strip_dir_from_path();				    // back up a directory
        bennvenn_send_command("BACK", 4);   // back up a directory
        dirpos = sel = 0;
        open_sd_dir(dirpos+sel);        		// open the dir
        get_sd_entries(dirpos);					    // get new directory entries
        sel = 0;                      			// set selected at zero
        continue;						                // restart the loop
      //}
    }
    if (JOY_RIGHT(joy)) {
      if (!sd_last_page) {						// if not at last page, get next page
        dirpos += 10;

        get_sd_entries(dirpos);
        sel = 0;
        continue;								// restart the loop
      }
    }
    
    // B button selects the directory
    if (JOY_BTN_2(joy)) {
      // FIXME: Do something here to record our destination

      return(1);                               // exit to host select, no directory selected
    }

    // A button selects directory or file
    if (JOY_BTN_1(joy)) {
      strcat(sd_dir, "/");                     // add slash
      strcat(sd_dir, filenames[sel]);          // add to directory string
      
      open_sd_dir(dirpos+sel);                
      dirpos = sel = 0;
      get_sd_entries(dirpos);
    }
  }

  return(1);
}
