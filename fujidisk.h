/**
 *  for Atari Lynx
 *
 * @brief Disk access routines
 * @author Shawn Jefferson
 * @email sjefferson at shaw dot ca
 */

#ifndef FUJIDISK_H
#define FUJIDISK_H

//#define BLOCK_SIZE  256
#define BLOCK_SIZE 1024

typedef enum
{
	BLOCK_RAW = 0,
	BLOCK_LZSA2,
	BLOCK_LZ4
} block_compression_t;

extern char dskbuf[BLOCK_SIZE+6];               // expose the buffer, room for command(1)+block(4)+compression type(1)+block data(1024)
extern char *disk_block_buffer;                 // pointer to disk block data


void _fujidisk_set_block(unsigned long block);

unsigned int fujidisk_read_block(unsigned char dev, unsigned long block);
unsigned char fujidisk_write_block(unsigned char dev, unsigned long block);


#endif