/**
 *  for Atari Lynx
 *
 * @brief Disk access routines
 * @author Shawn Jefferson
 * @email sjefferson at shaw dot ca
 */

#ifndef FUJIDISK_H
#define FUJIDISK_H


#define BLOCK_SIZE  256

extern char dskbuf[BLOCK_SIZE];     // expose the buffer


void fujidisk_reset(void);
unsigned char fujidisk_status(void);
unsigned char fujidisk_set_block(unsigned long block);
unsigned int fujidisk_recv_block(void);


#endif