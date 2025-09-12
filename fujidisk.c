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


// Set the block we are reading or writing
unsigned char fujidisk_set_block(unsigned long block)
{
  unsigned char r, i;


  dskbuf[0] = (unsigned char) (block & 0xFF);
  dskbuf[1] = (unsigned char) ((block & 0xFF00UL) >> 8);
  dskbuf[2] = (unsigned char) ((block & 0xFF0000UL) >> 16);
  dskbuf[3] = (unsigned char) ((block & 0xFF000000UL) >> 24);
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


// Receive the disk sector/block
unsigned char fujidisk_recv_block(void)
{
  unsigned char r, i;


  for(i=0; i<FN_RETRIES; ++i) {
    r = fnio_recv(DISK_DEV, &dskbuf[0], &fd_len);
    if (r) break;
  }

  // typically a disk "block" will be 256 bytes, but the last block may be partial as
  // not all files on the Lynx are divisble by 256 bytes. We could handle the last
  // block as a special case, or just write 256 bytes to the last block, making
  // the file on sdcard slightly larger than on the server.

  // Error or no data returned?
  if ((fd_len == 0) || (!r))
    return(0);
  else
    return(1);
}


