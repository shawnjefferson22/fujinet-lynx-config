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
#include "fujidisk.h"


#define FN_RETRIES  3                     // number of retries for commands

unsigned short fd_len;                    // length returned
char dskbuf[BLOCK_SIZE+5];                // buffer to use for disk
char *disk_block_buffer = &dskbuf[5];     // pointer to disk block data



// Set the block we are reading or writing
void _fujidisk_set_block(unsigned long block)
{
  dskbuf[1] = (unsigned char) (block & 0xFF);
  dskbuf[2] = (unsigned char) ((block & 0xFF00UL) >> 8);
  dskbuf[3] = (unsigned char) ((block & 0xFF0000UL) >> 16);
  dskbuf[4] = (unsigned char) ((block & 0xFF000000UL) >> 24);
}


// Receive the disk sector/block
unsigned int fujidisk_read_block(unsigned char dev, unsigned long block)
{
  unsigned char r, i;

  // clear the disk buffer since we may get a partial block
  memset(dskbuf, 0x00, sizeof(dskbuf));

  // set device and command
  dskbuf[0] = FUJICMD_READ;
  _fujidisk_set_block(block);

  // send command
  for(i=0; i<FN_RETRIES; ++i) {
    r = fnio_send_buf(dev, &dskbuf[0], 5);
    if (r) break;
  }
  // failed retries
  if (!r)
    return(0);

  // receive disk block
  //for(i=0; i<FN_RETRIES; ++i) {
  r = fnio_recv_buf(disk_block_buffer, &fd_len);
  //if (r) break;
  //}

  // typically a disk "block" will be 256 bytes, but the last block may be partial as
  // not all files on the Lynx are divisble by 256 bytes.  Return the true number of
  // bytes read.

  // Error or no data returned?
  if ((fd_len == 0) || (!r))
    return(0);
  else
    return(fd_len);
}


// Write the disk sector/block
// Data is expected to be a disk_block_buffer already
unsigned char fujidisk_write_block(unsigned char dev, unsigned long block)
{
  unsigned char r, i;
  
  
  // set device and command
  dskbuf[0] = FUJICMD_WRITE;
  _fujidisk_set_block(block);
  
  // send command
  for(i=0; i<FN_RETRIES; ++i) {
    r = fnio_send_buf(dev, &dskbuf[0], sizeof(dskbuf));
    if (r) break;
  }
  // failed retries
  if (!r)
    return(0);

  // wait for command completion/success
  r = fnio_recv_ack();
  if (r)
    return(1);
  else
    return(0);
}