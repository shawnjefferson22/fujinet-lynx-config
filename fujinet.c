/**
 *  for Atari Lynx
 *
 * @brief I/O routines
 * @author Thom Cherryhomes, Shawn Jefferson
 * @email thom dot cherryhomes at gmail dot com
 */

#include <6502.h>
#include <lynx.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <conio.h>
#include "lynxfnio.h"
#include "fujinet.h"


#define FN_RETRIES   3       // number of retries for commands

char fn_cmd[512];   // Fujinet command to send + aux bytes (might need to be increased to 1024)
unsigned short fn_len;       // length of data returned

char host_slots[MAX_HOSTS][MAX_HOSTNAME_LEN];
FN_DISK_SLOT disk_slots[MAX_DISK_SLOTS];
FN_ADAPTER_CONFIG fncfg;
FN_SSID_DETAIL wifi;
FN_SSID_PASS ssid_pass;



// helper to send a command that sends no data back
unsigned char _send_cmd(unsigned int l)
{
  unsigned char r, i;

  // send the command, retry if needed
  i = 0;
  while (i < FN_RETRIES) {
    r = fnio_send_buf(FUJI_DEVICEID_FUJINET, &fn_cmd[0], l);
    if (r)
      break;
    i++;
  }

  // failed all retries
  if (!r)
    return(0);

  // wait for succesful completion of command
  r = fnio_recv_ack();
  if (r)
    return(1);
  else
    return(0);
}


// helper to send a command that sends data back
// it's the callers responsibility to have a buffer of proper length
// and to cast the data properly
unsigned char _send_cmd_and_recv_buf(unsigned int l, char *buf)
{
  unsigned char r, i;

  i = 0;
  while (i < FN_RETRIES) {
    r = fnio_send_buf(FUJI_DEVICEID_FUJINET, &fn_cmd[0], l);
    if (r)
      break;
    i++;
  }

 // failed all retries
  if (!r)
    return(0);

  // get response
  r = fnio_recv_buf(buf, &fn_len);
  if ((fn_len != 0) && (r))     // did we receive anything?
    return(1);                  // return success
  else
    return(0);                  // return failure
}


unsigned char fujinet_get_wifi_status(void)
{
  unsigned char r;
  char status;

  fn_cmd[0] = FUJICMD_GET_WIFISTATUS;
  r = _send_cmd_and_recv_buf(1, &status);

  if (r) {
    if (status == 3)     // status of 3 is connected, 6 is disconnected
      return(1);

    return(0);
  }

  return(0);    // something went wrong, return not connected
}


unsigned char fujinet_scan_networks(void)
{
  unsigned char r, i;
  char num;

  fn_cmd[0] = FUJICMD_SCAN_NETWORKS;
  r = _send_cmd_and_recv_buf(1, &num);

  if (r)
    return(num);
  else
  	return(0);    // something went wrong no networks
}


unsigned char fujinet_scan_results(unsigned char n)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_GET_SCAN_RESULT;
  fn_cmd[1] = n;

  r = _send_cmd_and_recv_buf(2, (char *) &wifi);
  if (r)
    return 1;

  return(0);    // something went wrong no networks
}


unsigned char fujinet_get_adapter_config(void)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_GET_ADAPTERCONFIG;

  r = _send_cmd_and_recv_buf(1, (char *) &fncfg);
  if (r)
    return 1;

  return(0);    // something went wrong
}


unsigned char fujinet_get_ssid(void)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_GET_SSID;

  r = _send_cmd_and_recv_buf(1, (char *) &ssid_pass);
  if (r)
    return 1;

  return(0);    // something went wrong
}


unsigned char fujinet_set_ssid(char *ssid, char *password)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_SET_SSID;
  fn_cmd[1] = 1;                    // save

  memcpy(((FN_SSID_PASS *) &fn_cmd[1])->ssid, ssid, MAX_SSID_LEN+1);
  memcpy(((FN_SSID_PASS *) &fn_cmd[1])->password, password, MAX_WIFI_PASS_LEN);

  r = _send_cmd(sizeof(FN_SSID_PASS)+2);      // send cmd, save + FN_SSID_PASS
  if (r)
    return(1);

  return(0);    // something went wrong
}


unsigned char fujinet_read_host_slots(void)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_READ_HOST_SLOTS;

  r = _send_cmd_and_recv_buf(1, (char *) &host_slots);
  if (r)
    return 1;

  return(0);    // something went wrong
}


unsigned char fujinet_write_host_slots(unsigned char *host_slots)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_WRITE_HOST_SLOTS;
  memcpy(&fn_cmd[1], host_slots, MAX_HOSTS*MAX_HOSTNAME_LEN);

  r = _send_cmd((MAX_HOSTS*MAX_HOSTNAME_LEN)+1);
  if (r)
    return 1;

  return(0);    // something went wrong
}


unsigned char fujinet_mount_host(unsigned char host)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_MOUNT_HOST;
  fn_cmd[1] = host;

  r = _send_cmd(2);
  if (r)
    return 1;

  return(0);    // something went wrong
}


unsigned char fujinet_unmount_host(unsigned char host)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_UNMOUNT_HOST;
  fn_cmd[1] = host;

  r = _send_cmd(2);
  if (r)
    return 1;

  return(0);    // something went wrong
}


unsigned char fujinet_open_directory(unsigned char host_slot, char *dirpath)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_OPEN_DIRECTORY;
  fn_cmd[1] = host_slot;
  strcpy(&fn_cmd[2], dirpath);

  r = _send_cmd(strlen(dirpath)+3);         // two command bytes + strlen + null
  if (r)
    return 1;

  return(0);    // something went wrong
}


unsigned char fujinet_close_directory(void)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_CLOSE_DIRECTORY;

  r = _send_cmd(1);
  if (r)
    return 1;

  return(0);    // something went wrong
}


unsigned char fujinet_get_directory_position(unsigned int *pos)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_GET_DIRECTORY_POSITION;

  r = _send_cmd_and_recv_buf(1, (char *) pos);
  if (r)
    return(1);

  return(0);    // something went wrong
}


unsigned char fujinet_set_directory_position(unsigned int pos)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_SET_DIRECTORY_POSITION;
  fn_cmd[1] = pos & 0xFF;
  fn_cmd[2] = pos >> 8;

  r = _send_cmd(3);
  if (r)
    return(1);

  return(0);    // something went wrong
}


unsigned char fujinet_read_directory_entry(unsigned char maxlen, unsigned char extra, char *dir_entry)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_READ_DIR_ENTRY;
  fn_cmd[1] = maxlen;
  fn_cmd[2] = extra;			         // additional directory info (0x80 for true)

  memset(dir_entry, 0, maxlen);		 // clear entry buffer

  r = _send_cmd_and_recv_buf(3, (char *) dir_entry);
  if (r)
    return(1);

  return(0);    // something went wrong
}


unsigned char fujinet_set_device_filename(unsigned char ds, char *filename)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_SET_DEVICE_FULLPATH;
  fn_cmd[1] = ds;
  strcpy(&fn_cmd[2], filename);

  r = _send_cmd(strlen(filename)+3);    // two bytes, plus string, plus null
  if (r)
    return(1);

  return(0);    // something went wrong
}


unsigned char fujinet_mount_image(unsigned char ds, unsigned char options)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_MOUNT_IMAGE;
  fn_cmd[1] = ds;
  fn_cmd[2] = options;

  r = _send_cmd(3);
  if (r)
    return(1);

  return(0);    // something went wrong
}


unsigned char fujinet_unmount_image(unsigned char ds)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_UNMOUNT_IMAGE;
  fn_cmd[1] = ds;

  r = _send_cmd(2);
  if (r)
    return(1);

  return(0);    // something went wrong
}


unsigned char fujinet_mount_all(void)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_MOUNT_ALL;

  r = _send_cmd(1);
  if (r)
    return(1);

  return(0);    // something went wrong
}


unsigned char fujinet_read_device_slots(FN_DISK_SLOT *slots)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_READ_DEVICE_SLOTS;

  r = _send_cmd_and_recv_buf(1, (char *) slots);
  if (r)
    return(1);

  return(0);    // something went wrong
}


unsigned char fujinet_write_device_slots(FN_DISK_SLOT *slots)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_WRITE_DEVICE_SLOTS;
  memcpy(&fn_cmd[1], slots, (sizeof(FN_DISK_SLOT)*MAX_DISK_SLOTS));

  r = _send_cmd((sizeof(FN_DISK_SLOT)*MAX_DISK_SLOTS)+1);
  if (r)
    return(1);

  return(0);    // something went wrong
}


unsigned char fujinet_get_time(FN_TIME *datetime)
{
  unsigned char r;

  fn_cmd[0] = FUJICMD_GET_TIME;

  r = _send_cmd_and_recv_buf(sizeof(FN_TIME), (char *) datetime);
  if (r)
    return(1);

  return(0);    // something went wrong
}