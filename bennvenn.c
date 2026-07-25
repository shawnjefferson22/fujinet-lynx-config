#include <6502.h>
#include <lynx.h>
#include <tgi.h>
#include <conio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "bennvenn.h"
#include "sdcard.h"
#include "pathutil.h"

#ifdef SDCARD_BENNVENN


/*  Command set
// Directory commands and booting
SendCommand("LAST",4); //BOOT the last loaded ROM - make sure config file has the last booted file in there or else.. who knows!
SendCommand("ROOT",4); //Reinitialise the SD card, set root directory
SendCommand("DIR4LNXLYXO  SAV",16); //eg Directory Listing of 4 file types, *.LNX *.LYX, *.O *.SAV - up to 10 file types supported - File and Folder count are returned in the buffer
SendCommand("OPEN\x00\x00\x00\x0F",8); //Boot or open(if a directory) the 15th entry in the SRAM array
SendCommand("INSPECT\x00\x00\x00\x03",8); //Recover the first 126 bytes of the file via the buffer
SendCommand("BACK",4); //Go back one directory.

// config reading (??)
SendCommand("CONFIGW\x00\x00\x00\x00\x00\x00\x00\x08NEW DATA",23); //Write to the config file CONFIGWaaaabbbbccccccc.... a=offset of file, b=bytes to write, c=the databytes
SendCommand("CONFIGR\x00\x00\x00\x00\x00\x00\x00\x08",15); //Read from the config file CONFIGRaaaabbbb (max length of 126 bytes per transaction due to checksum byte) - data returned in the buffer

// file loading/saving
sendCommand("LOAD<filename 11 bytes><4 bytes dest in SRAM><4 bytes offset from file><4 bytes length>", size of string);
SendCommand("SAVE<filename 11 bytes><4 bytes offet><1 byte length><data>", size of string);
SendCommand("NEW<filename 11 bytes><4 bytes size>", size of string);

Reset(); //Resets the lynx - call this after booting the rom to free up the eeprom bus
File Index Structure: 64 bytes total. LFN=[0:46]; SFN [47:54] File/Folder Marker[55] ('1'=File, '2'=folder); File Size [56:59]; Cluster Address on SD [60:63]
*
*  The SendCommand function expects a  unsigned char ECSDbuffer[128]; for ingoing and outgoing data to the cart
*/

unsigned int bennvenn_num_folders;
unsigned int bennvenn_num_files;


/* trim spaces off entry
 */
void rtrim(char *str)
{
    size_t len = strlen(str);

    while (len > 0 && str[len - 1] == ' ')
    {
        str[len - 1] = '\0';
        len--;
    }
}


/* bennvenn_get_response
 *
 * Get the response after sending the command to the SD card
 */
void bennvenn_get_response(void) {
  unsigned char i, cell;
  unsigned int d;

  i = 0;
  for(cell=0; cell<64; ++cell) {				// 64 2-byte EEPROM cells to read
    d = lynx_eeread_BV(cell);
    sd_buf[i+1] = (char) (d & 0xFF);			// 128 byte buffer to read into
    sd_buf[i] = (char) (d >> 8);

    i += 2;
  }
}


/* bennvenn_send_command
 *
 * Send command to BennVenn SD card via eeprom API, and get response into buffer
 * length should be an even number
 */
unsigned char bennvenn_send_command(char data[64], unsigned char length)
{
  unsigned char n;
  unsigned int wait4reply = 2000;
  unsigned int d;


  // send the command by writing to eeprom
  for (n=0; n<length; n=n+2) {
    d = (unsigned int) data[n] << 8;
    d = d + (unsigned int) data[n+1];

    lynx_eewrite_BV(n/2, d);
  }
  lynx_eewrite_BV(0x3F, 0x55);			// 0x55 = process instruction.

  // wait for processing or timeout
  do {
    d = lynx_eeread_BV(0x3F);

    if (d == 0xAA55) {
      bennvenn_get_response();
      return(1);
    }
    else
      wait4reply--;
  } while (wait4reply != 0);

  return(0);
}


/* bennvenn_open
 *
 * helper to send open command
 *
 * Each nibble of int must be in a separate byte (for some reason)
 */
void bennvenn_open(unsigned int num)
{
  strcpy((char *) sd_buf, "OPEN");

  sd_buf[4] = (unsigned char) ((num >> 12) &0x0F);
  sd_buf[5] = (unsigned char) ((num >> 8) &0x0F);
  sd_buf[6] = (unsigned char) ((num >> 4) &0x0F);
  sd_buf[7] = (unsigned char) ((num) &0x0F);

  bennvenn_send_command(sd_buf, 8);
}


/* bennvenn_file_count
 *
 * Calculate the file and folder count from response to DIR command
 * 01234567890123456789  ??
 * FILES9999FOLDERS9999  ??
 */
void bennvenn_file_count(void)
{
  //num_files = (unsigned int) ((unsigned int) (sd_buf[6]-'0')*1000) + ((sd_buf[7]-'0')*100) + ((sd_buf[8]-'0')*10) + ((sd_buf[9]-'0'));
  bennvenn_num_folders = (unsigned int) ((unsigned int) (sd_buf[19]-'0')*1000) + ((sd_buf[20]-'0')*100) + ((sd_buf[21]-'0')*10) + ((sd_buf[22]-'0'));

  bennvenn_num_files = (unsigned int) ((sd_buf[6]-'0')*1000);
  bennvenn_num_files += (unsigned int) ((sd_buf[7]-'0')*100);
  bennvenn_num_files += (unsigned int) ((sd_buf[8]-'0')*10);
  bennvenn_num_files += (unsigned int) ((sd_buf[9]-'0'));
}


/* Reads the next directory entry from the bennvenn cartridge
 * Can be called after the directory command is issued.
 * File Index Structure: 64 bytes total. LFN=[0:46]; SFN [47:54] File/Folder Marker[55] ('1'=File, '2'=folder); File Size [56:59]; Cluster Address on SD [60:63]
 *
 * Will return the LFN (up to 46 characters + null) in entry
 */
void bennvenn_read_next_dir_entry(char *entry)
{
  	BV_FILE_STRUCT *bv_entry;

	// read the directory entry
  	bv_entry = (BV_FILE_STRUCT *) &sd_buf;
  	read(0, bv_entry, sizeof(BV_FILE_STRUCT));

	// copy the filename
  	memcpy(entry, bv_entry->long_name, sizeof(bv_entry->long_name));
  	rtrim(entry);		// trim whitespace off right side

  	// Is this a directory?
  	//if (bv_entry->type != 1) {					// FIXME: what are the actual type values?
    //	entry[strlen(entry)] = '/';
    //	entry[strlen(entry)+1] = '\0';
  	//}
  	//else
    	entry[47] = '\0';						// ensure LFN is null terminated
}


/* Sets the position of directory entry to read in cart
 *
 */
void bennvenn_set_dir_pos(unsigned int pos)
{
  lseek(0, pos*sizeof(BV_FILE_STRUCT), SEEK_SET);
}


/* Creates a new file with filename, and size
 *
 */
bool bennvenn_new_file(char *filename, unsigned long size)
{
  	unsigned char r;
    char fn[12];
    //char s[40];

    //sprintf(s, "f:%s", filename);
    //tgi_outtextxy(0, 70, s);

	  r = extract_and_pad_filename(filename, fn);
	  if (!r)
		  return(false);

    //sprintf(s, "f:%s", fn);
    //tgi_outtextxy(0, 78, s);
    //sprintf(s, "s:%-5ld", size);
    //tgi_outtextxy(0, 86, s);
    //cgetc();

    //012345678901234567
  	//NEWFILENAMEEXT1234
    sprintf(sd_buf, "NEW%-8.8s%3.3s", fn, extract_ext(filename));
  	sd_buf[14] = (unsigned char) (size >> 24);
  	sd_buf[15] = (unsigned char) (size >> 16);
  	sd_buf[16] = (unsigned char) (size >> 8);
  	sd_buf[17] = (unsigned char) (size);

  	r = bennvenn_send_command(sd_buf, 18);
	  if (r)
		  return(true);
	  else
		  return(false);
}


/* Saves a file to the bennvenn cartridge
 *
 */
bool bennvenn_save(char *filename, unsigned long offset, unsigned char size, char *buf)
{
  	unsigned char r;
    char fn[12];
    //char s[40];


	  // Cannot save more than 108 bytes at a time
	  if (size > BV_MAX_WRITE)
		  return(false);

	  r = extract_and_pad_filename(filename, fn);
	  if (!r)
		  return(false);

    //sprintf(s, "o:%-5ld s:%-3d", offset, size);
    //tgi_outtextxy(0, 56, s);
    //cgetc();

    //012345678901234567890
  	//SAVEFILENAMEEXT12341DATA
    sprintf(sd_buf, "SAVE%8.8s%3.3s", fn, extract_ext(filename));
  	sd_buf[15] = (unsigned char) (offset >> 24);
  	sd_buf[16] = (unsigned char) (offset >> 16);
  	sd_buf[17] = (unsigned char) (offset >> 8);
  	sd_buf[18] = (unsigned char) (offset & 0xFF);

  	sd_buf[19] = size;
  	memcpy(&sd_buf[20], buf, size);

  	r = bennvenn_send_command(sd_buf, 20+size);
    return(true);
}


/*
void test_bennvenn_dir(void)
{
  char s[21];
  char entry[50];


  bennvenn_send_command("BACK", 4);
  bennvenn_send_command("DIR4LNXLYXO  SAV ", 16);
  bennvenn_file_count();

  //dump_sdbuf();
  //tgi_clear();

  sprintf(s, "files: %d", bennvenn_num_files);
  tgi_outtextxy(0, 8, s);

  sprintf(s, "folders: %d", bennvenn_num_folders);
  tgi_outtextxy(0, 16, s);

  bennvenn_set_dir_pos(1);
  //bennvenn_get_dir_entries();

  //bennvenn_get_dir_entry(0, &entry[0]);
  bennvenn_read_next_dir_entry(&entry[0]);
  sprintf(s, "%-8s", entry);
  tgi_outtextxy(0, 24, s);

  //bennvenn_get_dir_entry(1, &entry[0]);
  bennvenn_read_next_dir_entry(&entry[0]);
  sprintf(s, "%-8s", entry);
  tgi_outtextxy(0, 32, s);

  //bennvenn_get_dir_entry(2, &entry[0]);
  bennvenn_read_next_dir_entry(&entry[0]);
  sprintf(s, "%-8s", entry);
  tgi_outtextxy(0, 40, s);

  tgi_outtextxy(0, 95, "press any key");
}
*/

#endif