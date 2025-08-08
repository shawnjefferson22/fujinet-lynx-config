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


/*  Command set
SendCommand("LAST",4); //BOOT the last loaded ROM - make sure config file has the last booted file in there or else.. who knows!
SendCommand("ROOT",4); //Reinitialise the SD card, set root directory
SendCommand("DIR4LNXLYXO  SAV",16); //eg Directory Listing of 4 file types, *.LNX *.LYX, *.O *.SAV - up to 10 file types supported - File and Folder count are returned in the buffer
SendCommand("OPEN\x00\x00\x00\x0F",8); //Boot or open(if a directory) the 15th entry in the SRAM array
SendCommand("INSPECT\x00\x00\x00\x03",8); //Recover the first 126 bytes of the file via the buffer
SendCommand("BACK",4); //Go back one directory.
SendCommand("CONFIGW\x00\x00\x00\x00\x00\x00\x00\x08NEW DATA",23); //Write to the config file CONFIGWaaaabbbbccccccc.... a=offset of file, b=bytes to write, c=the databytes
SendCommand("CONFIGR\x00\x00\x00\x00\x00\x00\x00\x08",15); //Read from the config file CONFIGRaaaabbbb (max length of 126 bytes per transaction due to checksum byte) - data returned in the buffer
Reset(); //Resets the lynx - call this after booting the rom to free up the eeprom bus
File Index Structure: 64 bytes total. LFN=[0:46]; SFN [47:54] File/Folder Marker[55] ('1'=File, '2'=folder); File Size [56:59]; Cluster Address on SD [60:63]
*
*  The SendCommand function expects a  unsigned char ECSDbuffer[128]; for ingoing and outgoing data to the cart
*/


unsigned int bennvenn_num_folders;
unsigned int bennvenn_num_files;


/*
void dump_sdbuf(void)
{
  char s[3];
  unsigned char i, x, y;

  tgi_clear();

  x = y = 0;
  for(i=0; i<64; ++i) {
    sprintf(s, "%02X ", sd_buf[i]);
    tgi_outtextxy(x, y, s);

    x += 16;
    if (x > 128) {
      x = 0;
      y += 8;
    }
  }

  cgetc();
}*/


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
    sd_buf[i+1] = (char) (d & 0xFF);				// 128 byte buffer to read into
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
 *
 * Will return the LFN (up to 46 characters + null) in entry
 */
void bennvenn_read_next_dir_entry(char *entry)
{
  read(0, &sd_buf, sizeof(BV_FILE_STRUCT));

  memcpy(entry, &sd_buf[0], 46);
  entry[47] = '\0';
}


/* Sets the position of directory entry to read in cart
 *
 */
void bennvenn_set_dir_pos(unsigned int pos)
{
  lseek(0, pos*sizeof(BV_FILE_STRUCT), SEEK_SET);
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