/**
 *  for Atari Lynx
 *
 * @brief Disk access routines
 * @author Shawn Jefferson
 * @email sjefferson at shaw dot ca
 */

#ifndef FUJIDISK_H
#define FUJIDISK_H


extern char dskbuf[256];                // expose the buffer


void fujidisk_reset(void);
unsigned char fujidisk_status(void);
unsigned char fujidisk_set_block(unsigned long block);
unsigned char fujidisk_recv_block(void);


#endif