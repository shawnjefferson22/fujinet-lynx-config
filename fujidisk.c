/**
 *  for Atari Lynx
 *
 * @brief Fuji disk routines
 * @author Shawn Jefferson
 * @email sjefferson at shaw dot ca
 */

#include <6502.h>
#include <lynx.h>
#include <conio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <tgi.h>
#include "display.h"
#include "input.h"
#include "lynxfnio.h"
#include "fujinet.h"


#define FN_RETRIES   3       // number of retries for commands

unsigned short fd_len;       // length returned
char dskbuf[256];            // buffer to use for disk


void fujidisk_reset(void)
{
  fnio_reset(DISK_DEV);
  return;
}

// Currently broken?
unsigned char fujidisk_status(void)
{
  unsigned char r;
  FUJI_IO_STATUS st;


  // ask for status
  r = fnio_status(DISK_DEV, (unsigned char *) &st);
  if (r)     
    return(st.status);                      // return status
  else
    return(STATUS_NO_DRIVE);                // return no drive
}


unsigned char fujidisk_set_block(unsigned long block)
{
  unsigned char r, i;
  
  dskbuf[0] = block & 0xFF;
  dskbuf[1] = block & 0xFF00;
  dskbuf[2] = block & 0xFF0000;
  dskbuf[3] = block & 0xFF000000;
  dskbuf[4] = 0;            // reserved byte

  for(i=0; i<FN_RETRIES; ++i) {
    r = fnio_send(DISK_DEV, &dskbuf[0], 5);
    if (r) break;
  }

  if (!r)
    return(0);
  else
    return(1);
}


unsigned char fujidisk_recv_block(void)
{
  unsigned char r, i;
  
  for(i=0; i<FN_RETRIES; ++i) {
    r = fnio_recv(DISK_DEV, &dskbuf[0], &fd_len);
    if (r == DISK_ACK) break;
  }

  if ((fd_len != 256) || (r == DISK_NACK))
    return(0);
  else
    return(1);
}


unsigned char fujidisk_get_file(unsigned char disk_slot, unsigned char dirpos)
{
  unsigned char r;
  unsigned long size;
  unsigned int i, blocks;


  // get the size of the file
  r = fujinet_set_directory_position(dirpos);		// set directory position to entry
  if (!r) {
    print_error("Error set dir pos!");
    wait_for_button();
    return(0);
  }

  r = fujinet_read_directory_entry(128, 0x80, &dskbuf[0]);
  if (!r) {
    print_error("Error reading entry!");
    wait_for_button();
  }
  size = dskbuf[6];
  size += (unsigned long) dskbuf[7] << 8;
  size += (unsigned long) dskbuf[8] << 16;
  size += (unsigned long) dskbuf[9] << 24;

  // calculate blocks to download
  blocks = size / 256;
  if (size % 256)
    blocks++;

  // mount the disk image in device slot
  r = fujinet_mount_image(disk_slot, DISK_ACCESS_MODE_READ);
  if (!r) {
    print_error("Error mounting image!");
    wait_for_button();
  }

  draw_box_with_text(4, 8, 155, 32, TGI_COLOR_RED, "Downloading", "Option1=Cancel");
  
  // Get all the blocks
  for(i=0; i<blocks; ++i) {
    sprintf(s, "Block %i of %i", i+1, blocks);
    tgi_outtextxy(6, 16, s);
    r = fujidisk_set_block(i);
    r = fujidisk_recv_block();

    if (kbhit()) {
      r = cgetc();
      if (r == '1')
        return(0);
    }
  }

  return(1);
}