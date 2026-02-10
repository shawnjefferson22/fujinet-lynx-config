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

extern char dskbuf[BLOCK_SIZE+5];               // expose the buffer (room for device(1)+command(1)+block(4))
extern char *disk_block_buffer;                 // pointer to disk block data


void _fujidisk_set_block(unsigned long block);

unsigned int fujidisk_read_block(unsigned char dev, unsigned long block);
unsigned char fujidisk_write_block(unsigned char dev, unsigned long block);


#endif