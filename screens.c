/**
 * Lynx Fujinet Config
 */

#include <6502.h>
#include <lynx.h>
#include <conio.h>
#include <tgi.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <joystick.h>
#include "display.h"
#include "fujinet.h"
#include "fujidisk.h"
#include "input.h"
#include "lynxfnio.h"
#include "pathutil.h"
#include "sdcard.h"
#include "bennvenn.h"
#include "logo.h"
#include "Program.h"
#include "sdcard.h"
#include "screens.h"


FN_SSID_DETAIL networks[10];    // ssid display (340 bytes)
unsigned char sel_host;         // store sel_host for directory display
char dirpath[256];              // directory path to pass to open directory
char filename[256];             // filename buffer
unsigned char dir_last_page;    // last directory page?



/* Wifi Scan and Connect screen
 *
 */
bool select_wifi_network(void)
{
	unsigned char r, i, n;
	unsigned char sel;
	unsigned char joy;
	char password[MAX_WIFI_PASS_LEN];

	#define SCAN_X  22
	#define SCAN_Y  30

RESCAN:
	tgi_clear();

	draw_box_with_text(SCAN_X, SCAN_Y, SCAN_X+116, SCAN_Y+32, TGI_COLOR_RED, NULL, NULL);
	tgi_outtextxy(SCAN_X+3, SCAN_Y+9, "Scanning wifi");
	tgi_outtextxy(SCAN_X+3, SCAN_Y+19, "Please wait...");
	n = fujinet_scan_networks();
	if (n > 12) n = 9;           // cap to 9 networks

	tgi_clear();
	draw_box_with_text(0, 0, 159, 97, TGI_COLOR_BLUE, "Wifi networks", NULL);
	print_key_legend("A=Select B=Rescan");

	// Get the wifi scan data
	for (i=0; i<n; ++i) {
		r = fujinet_scan_results(i);
		if (r) {
			memcpy(networks[i].ssid, wifi.ssid, MAX_SSID_LEN+1);
			networks[i].rssi = wifi.rssi;
		}
		else {
			n = i;                      // make this new last one
			break;
		}
	}

  // Add custom/hidden SSID choice
  strcpy(networks[i].ssid, "Custom SSID");
  networks[i].rssi = 127;
  n++;

  // Display the networks and selected one
  sel = 0;
  while(1) {
    for (i=0; i<n; ++i) {
      if (i == sel)
        tgi_setbgcolor(TGI_COLOR_GREEN);

      sprintf(s, "%-18.18s", networks[i].ssid);
      tgi_outtextxy(3, (i*8)+8, s);
      display_wifi_sprite((i*8)+8, networks[i].rssi);

      tgi_setbgcolor(TGI_COLOR_BLACK);
    }

    r = check_joy_and_keys_loop(&joy);

    if (r == SCANWIFI_KEY) {						// Opt2 exits
      return(0);
    }
    if (JOY_UP(joy)) {
      if (sel != 0)
        --sel;
    }
    if (JOY_DOWN(joy)) {
      if (sel != (n-1))
        ++sel;
    }
    if (JOY_BTN_2(joy)) {				// button B does a rescan
      goto RESCAN;
    }
    if (JOY_BTN_1(joy)) {				// button A selects wifi network
      break;
    }
  }

  // Get hidden SSID name
  tgi_clear();
  if (sel == (n-1)) {
    memset(password, 0, sizeof(password));
    r = get_input_with_title(8, MAX_SSID_LEN, "SSID", password);				// use password buffer
    if (r) {
      memcpy(ssid_pass.ssid, password, MAX_SSID_LEN+1);
    }
    else
      goto RESCAN;
  }
  else
    memcpy(ssid_pass.ssid, networks[sel].ssid, MAX_SSID_LEN+1);

  // Get SSID password
  tgi_clear();
  sprintf(s, "%-19.19s", ssid_pass.ssid);
  tgi_outtextxy(1, 1, s);

  memset(password, 0, sizeof(password));
  r = get_input_with_title(16, MAX_WIFI_PASS_LEN, "Password", password);
  if (r) {
    strcpy(ssid_pass.password, password);

    // clear the password input prompt, display feedback to user
    tgi_setcolor(TGI_COLOR_BLACK);
    tgi_bar(0, 16, 159, 101);
    tgi_setcolor(TGI_COLOR_WHITE);
    tgi_outtextxy(1, 9, "Connecting...");

    r = fujinet_set_ssid(ssid_pass.ssid, ssid_pass.password);
    if (!r) {
	  display_error_and_wait("Error connecting!");
      goto RESCAN;
    }

    for(i=0; i<3; ++i) {
      sleep(2);
      r = fujinet_get_wifi_status();

      if (r) break;
      tgi_outtext(".");
    }

    // Error connecting
    if (!r) {
	  display_error_and_wait("Error connecting!");
      goto RESCAN;      			// should we just exit out? Bad Shawn using gotos!
    }
  }
  else
    goto RESCAN;

  return(1);
}


/* Select and edit hosts screen
 *
 */
bool select_host(void)
{
  unsigned char r, i;
  unsigned char sel, joy;
  char newhost[MAX_HOSTNAME_LEN];

REDRAW:
  i = sel = sel_host = 0;
  tgi_clear();
  draw_box_with_text(0, 0, 159, 77, TGI_COLOR_YELLOW, "Hosts", "A=Select B=Edit");

  // Input loop
  while (1) {
    display_hosts(sel);

    r = check_joy_and_keys_loop(&joy);
    if (r) {
      switch(r) {
        case REFRESH_KEY:
          read_hosts();
          goto REDRAW;
        case ADAPTERCFG_KEY:
          display_adapter_config();
          goto REDRAW;
        case SCANWIFI_KEY:
          r = select_wifi_network();
          goto REDRAW;
      }
    }

    if (JOY_UP(joy)) {
      if (sel != 0)
        --sel;
    }
    if (JOY_DOWN(joy)) {
      if (sel<(MAX_HOSTS-1))
        ++sel;
    }
    if (JOY_BTN_1(joy)) { // A button
      if (host_slots[sel][0] != '\0') {
        sel_host = sel;
	    r = fujinet_mount_host(sel_host);			// mount host
	    if (!r) {							        // error mounting
          display_error_and_wait("Error mounting host!");
	      continue;
        }
        return(true);                          		// exit out to display directories
      }
    }
    if (JOY_BTN_2(joy)) { // B button
      strcpy(newhost, host_slots[sel]);
	  r = get_input(4, (sel*8)+8, MAX_HOSTNAME_LEN, &newhost[0]);
	  if (r) {
	    strcpy(host_slots[sel], newhost);
	    r = fujinet_write_host_slots((unsigned char *) &host_slots);
	    if (!r) {
          display_error_and_wait("Error writing hosts!");
	      return(false);
        }
      }
    }
  }
}


/* Show file/folder info screen
 *
 */
void show_ext_file_info(unsigned char dirpos)
{
	unsigned char r;
	unsigned char i, y;
	unsigned char len;
	FILE_INFO_EXT *entry;
	char str[20];


	// get the size of the file
	r = fujinet_set_directory_position(dirpos);					// set directory position to entry
	if (!r) {
		display_error_and_wait("Error set dir pos!");
		return;
	}

	r = fujinet_read_directory_entry(128, 0x80, &dskbuf[0]);		// get dir entry with extra info
	if (!r) {
		display_error_and_wait("Error reading entry!");
		return;
	}
	entry = (FILE_INFO_EXT *) &dskbuf[0];

 	// Display the title
  tgi_clear();
  if (entry->isdir)
    strcpy(str, "Dir Info");
  else
    strcpy(str, "File Info");
  draw_box_with_text(0, 0, 159, 97, TGI_COLOR_BLUE, str, NULL);

	// 01234567890123456789
	//0FilenameFilenameFile
	//1FilenameFilenameFile
	//2filenameFilenameFile
	//3filenameFilenameFile
	//4filenameFilenameFile
	//5
	//6 Modtime:
	//7  26/01/01 24:24:24
	//8
	//9 Size:
	//0  1,234,567,890
	//1
	//2

	// output long filename
	len = strlen(entry->filename);

  y = 1;                                // y screen coord
  str[19] = '\0';                       // make sure our output string is truncated
	for(i=0; i<len; i=i+19) {
		y += 8;
    strncpy(str, &entry->filename[i], 19);
		tgi_outtextxy(3, y, str);
  }

  // output size if not a directory
	if (!entry->isdir) {
    y += 16;
    sprintf(s, "%ld", entry->size);
	  tgi_outtextxy(3, y, "Size:");
	  tgi_outtextxy(49, y, s);
  }

	// output mod time
  y += 16;
	sprintf(s, "%02d/%02d/%02d %02d:%02d:%02d", entry->month, entry->day, entry->year, entry->hour, entry->min, entry->sec);
	tgi_outtextxy(3, y, "Mod Time:");
	tgi_outtextxy(3, y+8, s);

	wait_for_any_key();
}


/* Download file from Fujinet screen
 *
 */
bool get_file(unsigned char disk_slot, unsigned char dirpos)
{
  unsigned char r;
  unsigned long size;
  unsigned int i, blocks, len;


  // get the size of the file
  r = fujinet_set_directory_position(dirpos);					// set directory position to entry
  if (!r) {
    display_error_and_wait("Error set dir pos!");
    return(false);
  }

  r = fujinet_read_directory_entry(128, 0x80, &dskbuf[0]);		// get dir entry with extra info
  if (!r) {
    display_error_and_wait("Error reading entry!");
    return(false);
  }
  size = dskbuf[6];												// extract the size
  size += (unsigned long) dskbuf[7] << 8;
  size += (unsigned long) dskbuf[8] << 16;
  size += (unsigned long) dskbuf[9] << 24;

  // size sanity check
  if (size > 524352U)      // truncate size to 512k+64 byte header (no lynx carts larger than this?)
    size = 524352U;

  // calculate blocks to download
  blocks = size / BLOCK_SIZE;
  if (size % BLOCK_SIZE)        // last block may be partial
    blocks++;

  // If not a Lynx filetype, ask user if they really want to download this?
  if (lynx_filetype(&filename[0]) == FILETYPE_NONE) {
    tgi_clear();
    r = ask_yesno_download(8);
    if (!r)
      return(false);
  }

  // mount the disk image in device slot
  r = fujinet_mount_image(disk_slot, DISK_ACCESS_MODE_READ);
  if (!r) {
    display_error_and_wait("Error mounting image!");
    return(false);
  }

	// Select directory to write file
	#ifdef SDCARD_GAMEDRIVE
    //switch(blocks) {
    //  case 513:
    //    strcpy(sd_dir, "/FUJINET/GAMEDRV.128");
    //    break;
    //  case 1025:
    //    strcpy(sd_dir, "/FUJINET/GAMEDRV.256");
    //    break;
    //  case 2049:
    //    strcpy(sd_dir, "FUJINET/GAMEDRV.512");
    //    break;
    //  default:
    	strcpy(sd_dir, "/FUJINET/GAMEDRV1");
    //}
	#endif
	#ifdef SDCARD_BENNVENN
  		r = select_sdcard_dir();
  		if (!r)
  			return(false);
	#endif

	// Display downloading dialog
	tgi_clear();
	draw_box_with_text(4, 8, 155, 32, TGI_COLOR_RED, "Downloading", "OPT1=Cancel");
	sprintf(s, "%-19s", sd_dir);        // display dest dir
  	tgi_outtextxy(1, 0, s);

	// Open/Create file on SD card
	display_file_action("o");
	#ifdef SDCARD_GAMEDRIVE
		r = sd_open_file(sd_dir);
	#endif
	#ifdef SDCARD_BENNVENN
		r = sd_create_file(filename, size);
	#endif

	if (!r) {
		display_error_and_wait("Error SD create file!");
		fujinet_unmount_image(disk_slot);
		return(false);
	}

	// Get all the blocks
	for(i=0; i<blocks; ++i) {
		sprintf(s, "Block %i of %i", i+1, blocks);
		tgi_outtextxy(6, 17, s);

		display_file_action("r");
		len = fujidisk_read_block(FUJI_DEVICEID_DISK, i);
		// len is zero, or len != BLOCK_SIZE on any block except last
		if ((!len) || (len != BLOCK_SIZE && (i < (blocks-1)))) {
			display_error_and_wait("Error during receive!");
			fujinet_unmount_image(disk_slot);
			sd_close_file();
			return(false);
		}

		// User cancel?
		if (kbhit()) {
			r = cgetc();
			if (r == '1') {
				fujinet_unmount_image(disk_slot);
				sd_close_file();
				return(false);
			}
		}

		// FIXME: debugging
		//sprintf(s, "o:%-5ld s:%-3d", (uint32_t) ((uint32_t) i * (uint32_t) BLOCK_SIZE), len);
		//tgi_outtextxy(0, 50, s);
    //cgetc();

		// Write the block to SD card file
    display_file_action("w");
  	r = sd_write_file_block(filename, (uint32_t) ((uint32_t) i * (uint32_t) BLOCK_SIZE), len, disk_block_buffer);
  	if (!r) {
	   	display_error_and_wait("Error writing to SD file!");
    	fujinet_unmount_image(disk_slot);
    	sd_close_file();
    	return(false);
 		}
	}

	fujinet_unmount_image(disk_slot);
	display_file_action("c");
	sd_close_file();
	return(true);
}


/* Select Fujinet file screen
 *
 */
void select_files(void)
{
  unsigned char r, st, scrdir;
  unsigned int delay;
  unsigned char joy, sel;
  unsigned int dirpos;
  unsigned char len;
  char entry[129];


 // Initialize and start at root directory
  sel = dirpos = 0;                       // reset selected host and dirpos
  memset(dirpath, 0, 256);                // clear the dirpath
  strcpy(dirpath, "/");                   // start at root

REOPEN_DIR:
  open_dir();
  get_dir_entries();

REDRAW:
  tgi_clear();
  draw_box_with_text(0, 0, 159, 92, TGI_COLOR_YELLOW, "Host Files", "A=Select B=Back");

  // input loop
  while(1) {
    st = scrdir = delay = 0;                       // reset start of entry, dir of scroll, delay
    display_files(sel);

    // Process joystick and keys
    do {
      r = check_joy_and_keys(&joy);

      // Need to scroll this entry?
      len = strlen(filenames[sel]);
      if ((delay == SCROLL_DELAY) && (len > 19)) {
        if (scrdir)
          st--;       // backward
        else
          st++;       // forward

        if (st > (len-20))
          scrdir = 1;                                   // reverse scroll dir
        else if (st == 0)
          scrdir = 0;                                   // forward direction

        scroll_file_entry(sel, st);
        delay = 0;
      }

      delay++;
    } while (!r && !joy);

    if (r) {
      switch(r) {
        case ADAPTERCFG_KEY:
          display_adapter_config();
          goto REDRAW;
        case SCANWIFI_KEY:
		      r = select_wifi_network();
		      return;							// return to hosts selection
	      case FILEINFO_KEY:
			    show_ext_file_info(dirpos+sel);
			    goto REDRAW;
      }
    }

    if (JOY_UP(joy)) {
      if (sel != 0)						    // not at top of page
        --sel;
      else {								      // at top of page
	      if (dirpos > 9) {					// only if we aren't at top of directory already!
	        joy = JOY_LEFT_MASK;		// set joy left, and let it process down below
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
        dirpos -= 10;                       // get previous page of entries
		    r = fujinet_set_directory_position(dirpos);
		    if (!r) {
    	    display_error_and_wait("Error set dir pos!");
          return;
	      }

        get_dir_entries();			      // get more directory entries
        sel = 0;                      // set selected at zero
        continue;                     // restart loop
      }
      // go back to previous directory?
      if (dirpath[1] != '\0') {				// at root already?
        strip_dir_from_path((char *) &dirpath);				// back up a directory
        open_dir();                   // open the dir
        get_dir_entries();					  // get new directory entries
        sel = 0;                      // set selected at zero
        continue;							        // restart the loop
      }
    }
    if (JOY_RIGHT(joy)) {
      if (!dir_last_page) {					// if not at last page, get next page
        dirpos += 10;

        get_dir_entries();
        sel = 0;
        continue;							// restart the loop
      }
    }
    // B button exits file selection, bach to host selection
    if (JOY_BTN_2(joy)) {
	    fujinet_close_directory();		  	      // close the directory before we try another host (ignore return value)
      fujinet_unmount_host(sel_host);       // unmount host (ignore return value)
      return;                               // exit to host select
    }
    // A button selects directory or file
    if (JOY_BTN_1(joy)) {
      r = read_full_dir_entry(dirpos+sel, &entry[0], 0);
      if (!r) {
        display_error_and_wait("Error getting entry!");
        continue;
      }
      // Handle directory
      //if (entry[strlen(entry)-1] == '/') {
      if (path_is_dir(entry)) {
        strcat(dirpath, entry);			  // add this dir to dirpath
        open_dir();                   // open the dir
        get_dir_entries();					  // get new directory entries
        sel = 0;                      // set selected at zero
        continue;                     // skip to top of loop
      }

      // Handle link
      if (entry[0] == '+') {
        strcpy(host_slots[MAX_HOSTS-1], &entry[1]);						        // copy link to last host slot
        r = fujinet_write_host_slots((unsigned char *) host_slots);		// write the host slots
        if (!r) {
          display_error_and_wait("Error writing slots!");
        }
        return;                               // exit to host select
      }

      // Handle file
      // read all the device slots, set the hostslot and then write back
      // this is the only way to do this currently with FN
      r = fujinet_read_device_slots(&disk_slots[0]);
      if (!r) {
		    display_error_and_wait("Error read disk slots!");
		    continue;
      }
      disk_slots[0].hostSlot = sel_host;
      disk_slots[0].mode = DISK_ACCESS_MODE_READ;
      strncpy(disk_slots[0].filename, entry, 35);
      r = fujinet_write_device_slots(&disk_slots[0]);
      if (!r) {
		    display_error_and_wait("Error write disk slots!");
		    continue;
      }

      strcpy(filename, dirpath);          // FIXME: this only allows max 255 long string for filename
      strcat(filename, entry);

      r = fujinet_set_device_filename(0, &filename[0]);
      if (!r) {
        display_error_and_wait("Error setting device!");
        continue;
      }

      // Need to mount all devices, and set hostSlot on device
      r = fujinet_mount_all();
      if (!r) {
	      display_error_and_wait("Error mounting all!");
	      continue;
      }

      // Download the file, select SD destination directory
      r = get_file(0, dirpos+sel);      // filename[0] has the full dirpath+entry
      if (!r)
	      goto REOPEN_DIR;

      #ifdef SDCARD_GAMEDRIVE
      tgi_outtextxy(2,50, "Programming...");
      display_file_action("p");
      if (LynxSD_Program(sd_dir) == FR_OK) {
        tgi_outtextxy(2, 58, "Launching...");
        display_file_action("l");
        LaunchROM();
      }
      #endif

      #ifdef SDCARD_BENNVENN
      	delay = sd_find_filenum(extract_filename(&filename[0]));    // re-using delay local var
      	bennvenn_open(delay);

      	//display_file_data();              // display last block data, testing
		    // goto REOPEN_DIR;
      #endif

      goto REDRAW;
    }
  }
}


/* Show SD Card directories, and allow selecting a destination directory
 * for the file to be written.
 */
bool select_sdcard_dir(void)
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
    return(false);
  }
  sd_get_entries(0);

  // input loop
  while(1) {
    st = scrdir = delay = 0;                       // reset start of entry, dir of scroll, delay
    display_files(sel);

    // Process joystick and keys
    do {
      r = check_joy_and_keys(&joy);
      if (r == '2')       	// opt2 exits
        return(false);
      if (r == '1')
        return(true);      	// opt1 selects

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
            return(false);
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
      return(true);                               // exit to host select, directory selected
    }

    // A button selects directory or file
    if (JOY_BTN_1(joy)) {
      // Is this a directory?
      if (path_is_dir(filenames[sel])) {
        strcat(sd_dir, filenames[sel]);       	// add to directory string

        r = sd_open_dir(dirpos+sel);        		// open the dir  - GameDrive code ignores passed in dir pos
        if (!r) {
          display_error_and_wait("Error opening sdcard!");
          return(false);
        }

        dirpos = sel = 0;
        sd_get_entries(dirpos);
      }
      else {
        return(true);
      }
    }
  }

  #endif		// SDCARD_NONE
}


/* read hosts slots from Fujinet
 *
 */
bool read_hosts(void)
{
  unsigned char r, i;


  // read host slots, try three times
  for(i=0; i<3; ++i) {
    r = fujinet_read_host_slots();
    if (r) break;
  }
  if (!r) {
    display_error_and_wait("Error reading hosts!");
    return(false);
  }

  return(true);
}


/* open directory on fujinet
 *
 */
void open_dir(void)
{
  unsigned char r;

  r = fujinet_close_directory();                    // close the directory just in case it's open
  r = fujinet_open_directory(sel_host, dirpath);    // open new directory
  if (!r) {
    display_error_and_wait("Error opening dir!");
    return;
  }

  // Set directory position to zero
  r = fujinet_set_directory_position(0);
  if (!r) {
    display_error_and_wait("Error set dir pos!");
    return;
  }
}


/* Read full directory entry from Fujinet
 * Pass 0 to get dir entry without extended info, 0x80 for extended info
 *
 */
bool read_full_dir_entry(unsigned int pos, char *entry, unsigned char ext)
{
  unsigned char r;

  r = fujinet_set_directory_position(pos);		// set directory position to entry
  if (!r) {
    display_error_and_wait("Error set dir pos!");
    return(false);
  }

  r = fujinet_read_directory_entry(128, ext, entry);  // read full directory name
  if (!r) {
    display_error_and_wait("Error reading dir!");
    return(false);
  }

  return(true);
}


/* get the directory entries from Fujinet
 *
 */
void get_dir_entries(void)
{
  unsigned char i, r;


  dir_last_page = 0;							            // reset last page flag
  memset(&filenames, 0, sizeof(filenames));		// clear filenames array

  for(i=0; i<10; ++i) {
    r = fujinet_read_directory_entry(60, 0, &filenames[i][0]);
    if (!r) {									               // error reading dir entry
      filenames[i][0] = '\0';					       // clear entry
      return;
    }
    if (filenames[i][0] == 0x7F) {		       // last entry of directory, thanks Thom ;)
      filenames[i][0] = '\0';
      dir_last_page = 1;						         // set last page flag
      return;
    }
  }
}
