/**
 *  for Atari Lynx
 *
 * @brief I/O routines
 * @author Thom Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 */

#include <6502.h>
#include <lynx.h>
#include <serial.h>
#include <tgi.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "lynxfnio.h"


#define RECV_TIMEOUT  3   // timeout for receive loop, in seconds

// Global variables
unsigned char _ck;			  // checksum byte
char _r;                  // response/data from FN



unsigned char fnio_init(void)
{
  struct ser_params params = {
    SER_BAUD_62500,
    SER_BITS_8,
    SER_STOP_1,
    SER_PAR_ODD,
    SER_HS_NONE
  };

  ser_install(lynx_comlynx_ser); // This will activate the ComLynx
  CLI();

  return ser_open(&params);
}


unsigned char fnio_done(void)
{
  return ser_close();
}


/**
 * @brief calculates the checksum on the packet and stores in global var
 * @param *b pointer to buffer
 * @param len length of buffer
 */
void _checksum(char *b, unsigned short len)
{
  register unsigned short i;


  // initialize checksum to zero
  _ck = 0;

  for (i = 0; i < len; ++i)
    _ck ^= b[i];

  return;
}


/**
 * @brief loop to get data that is used many times
 * @param b The buffer to receive into
 */
void _serial_get_loop(void)
{
  clock_t start, now;

  start = clock();
  while (ser_get(&_r) == SER_ERR_NO_DATA) {
    now = clock();
    if (((now - start) / CLOCKS_PER_SEC) > RECV_TIMEOUT) {
      _r = FUJICMD_NAK;
      break;
    }
  }
}


/**
 * @brief send packet to FujiNet device
 * @param dev The device # 0x00 - 0x0F
 * @param buf The buffer to send
 * @param len Length of buffer to send
 * @return 1 on ACK, 0 on NAK
 */
unsigned char fnio_send_buf(unsigned char dev, char *buf, unsigned short len)
{
  register unsigned short i;


  // Calculate checksum on buffer
  _checksum(buf, len);

  // Send the Device and Length
  ser_put(dev);
  ser_put(len >> 8);
  ser_put(len & 0xFF);
  ser_get(&_r);           // get rid of reflected data
  ser_get(&_r);
  ser_get(&_r);

  // send the payload
  for(i=0; i<len; ++i) {
    //while (ser_put(buf[i]) != SER_ERR_OVERFLOW);        // handle if we overflowed the TX buffer
    ser_put(buf[i]);
    ser_get(&_r);         // get rid of reflected data we just sent
  }

  // send the checksum
  ser_put(_ck);
  ser_get(&_r);           // get rid of reflected data

  // Get response
  _serial_get_loop();

  // r contains our response, ACK or NACK
  if (_r == FUJICMD_ACK)
    return(1);
  else
    return(0);
}


unsigned char fnio_recv_buf(char *buf, unsigned short *len)
{
  register unsigned short i;


  // Get first length byte
  _serial_get_loop();
  *len = _r << 8;
  // Get second length byte
  _serial_get_loop();
  *len |= _r & 0xFF;

  if (*len > LEN_MAX)       // no more than LEN_MAX bytes
	  *len = LEN_MAX;

  // Now get the payload
  for (i=0; i<*len; ++i) {
	  _serial_get_loop();
   	buf[i] = _r;
  }

  // Get the checksum
  _serial_get_loop();

  // checksum matches?
  _checksum(buf, *len);
  if (_r == _ck) {
	  ser_put(FUJICMD_ACK);								  // ACK
    ser_get((char *) &_r);                // get reflected data
    return(1);                            // succes, checksum matches
  }
  else {
	  ser_put(FUJICMD_NAK); 								// NACK, checksum bad
    ser_get((char *) &_r);                // get reflected data
	  *len = 0;											        // return zero length
    return(0);                            // checksum bad
  }
}


// receive an ACK or NAK
// returns 1 on ACK received or 0 on NAK
// Call after commands that don't send back any data
unsigned char fnio_recv_ack(void)
{
  
  _serial_get_loop();
  if (_r == FUJICMD_ACK)
    return(1);
  else
    return(0); 
}