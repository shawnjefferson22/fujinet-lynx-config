#include <6502.h>
#include <lynx.h>
#include <tgi.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <joystick.h>
#include "display.h"
#include "input.h"
#include "fujinet.h"
#include "fujidisk.h"


#define KEY_LINE	95
#define ERROR_LINE  95


char s[21];                     // buffer for text display
char filenames[10][64];         // filename display (704 bytes), filenames capped to 64 bytes


// draws a box, with title and prompt from x1,y1, to x2,y2.
// @param color  color of line to draw
// @param title  tile to draw in top left corner
// @param prompt  instructions to draw in bottom left corner
void draw_box_with_text(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color, char *title, char *prompt)
{
  // clear under box
  tgi_setcolor(TGI_COLOR_BLACK);
  tgi_bar(x1, y1, x2, y2);

  // draw box, offsetting slightly for text
  tgi_setcolor(color);
  tgi_line(x1, y1+4, x2, y1+4);     // top line
  tgi_lineto(x2, y2);				// right line
  tgi_lineto(x1, y2);				// bottom line
  tgi_lineto(x1, y1+4);				// left line

  // draw text title and prompt if passed in
  tgi_setcolor(TGI_COLOR_WHITE);
  if (title)
    tgi_outtextxy(x1+8, y1, title);
  if (prompt)
    tgi_outtextxy(x1+8, y2-4, prompt);
}


/* print_key_legend
 *
 * Prints a keystroke legend (or anything passed in) on the last line of the display
 */
void print_key_legend(char *legend)
{
  tgi_setcolor(TGI_COLOR_BLACK);
  tgi_bar(0, KEY_LINE, 0, KEY_LINE+8);
  tgi_setcolor(TGI_COLOR_WHITE);
  tgi_outtextxy(8, KEY_LINE, legend);
}


/* print_error
 *
 * displays an error message on last line, in red
 */
void print_error(char *msg)
{
  tgi_setcolor(TGI_COLOR_BLACK);
  tgi_bar(0, ERROR_LINE, 0, ERROR_LINE+8);
  tgi_setcolor(TGI_COLOR_RED);
  tgi_outtextxy(1, ERROR_LINE, msg);
  tgi_setcolor(TGI_COLOR_WHITE);
}


/* display_error_and_wait
 *
 * Display the error message and wait for button
 * and then clear the error line.
 */
void display_error_and_wait(char *msg)
{
  print_error(msg);
  wait_for_button();
  tgi_bar(0, ERROR_LINE, 0, ERROR_LINE+8);
}


void display_adapter_config(void)
{
  unsigned char r;


  tgi_clear();
  draw_box_with_text(0, 0, 159, 101, TGI_COLOR_BLUE, "Adapter Config", NULL);

  r = fujinet_get_adapter_config();
  if (!r) {
    tgi_setcolor(TGI_COLOR_RED);
    tgi_outtextxy(1, 1, "Error reading config!");
    tgi_setcolor(TGI_COLOR_WHITE);
  }
  else {
    sprintf(s, "SSID: %-13.13s", fncfg.ssid);
    tgi_outtextxy(2, 17, s);
    sprintf(s, "HN: %-16.16s", fncfg.hostname);
    tgi_outtextxy(2, 25, s);
    sprintf(s, "IP: %d.%d.%d.%d", fncfg.localIP[0], fncfg.localIP[1], fncfg.localIP[2], fncfg.localIP[3]);
    tgi_outtextxy(2, 33, s);
    sprintf(s, "GW: %d.%d.%d.%d", fncfg.gateway[0], fncfg.gateway[1], fncfg.gateway[2], fncfg.gateway[3]);
    tgi_outtextxy(2, 41, s);
    sprintf(s, "NM: %d.%d.%d.%d", fncfg.netmask[0], fncfg.netmask[1], fncfg.netmask[2], fncfg.netmask[3]);
    tgi_outtextxy(2, 49, s);
    sprintf(s, "DNS: %d.%d.%d.%d", fncfg.dnsIP[0], fncfg.dnsIP[1], fncfg.dnsIP[2], fncfg.dnsIP[3]);
    tgi_outtextxy(2, 57, s);
    sprintf(s, "MAC: %02X%02X%02X%02X%02X%02X", fncfg.macAddress[0], fncfg.macAddress[1], fncfg.macAddress[2], fncfg.macAddress[3], fncfg.macAddress[4], fncfg.macAddress[5]);
    tgi_outtextxy(2, 66, s);
    sprintf(s, "VER: %s", fncfg.fn_version);
    tgi_outtextxy(2, 75, s);
  }

  check_joy_and_keys(&r);           // something was pressed
}


void display_hosts(unsigned char sel)
{
  unsigned int i;

  for(i=0; i<MAX_HOSTS; ++i)
  {
    if (i == sel)
      tgi_setbgcolor(TGI_COLOR_GREEN);
    else
      tgi_setbgcolor(TGI_COLOR_BLACK);

    sprintf(s, "%-19.19s", host_slots[i]);
    tgi_outtextxy(4, (i*8)+8, s);
  }

  tgi_setbgcolor(TGI_COLOR_BLACK);
}


void display_files(unsigned char sel)
{
  unsigned char i;


  for(i=0; i<10; ++i) {
    sprintf(s, "%-19.19s", filenames[i]);
    if (sel == i)
      tgi_setbgcolor(TGI_COLOR_GREEN);
    else
      tgi_setbgcolor(TGI_COLOR_BLACK);
    tgi_outtextxy(4, (i*8)+8, s);
  }

  tgi_setbgcolor(TGI_COLOR_BLACK);
  return;
}


void scroll_file_entry(unsigned char sel, unsigned char st)
{
  tgi_setbgcolor(TGI_COLOR_GREEN);
  sprintf(s, "%-19.19s", &filenames[sel][st]);
  tgi_outtextxy(4, (sel*8)+8, s);
  tgi_setbgcolor(TGI_COLOR_BLACK);
  return;
}


// display data of dksbuf in hex
// for troubleshooting/debug
void display_file_data(void)
{
  unsigned char i, j;

  for (j=0; j<16; ++j) {
    //sprintf(s, "%02X:", j);
    //tgi_outtextxy(2, j*8, s);
    for(i=0; i<16; ++i) {
      sprintf(s, "%02X", dskbuf[(j*16)+i]);
      tgi_outtextxy(0, j*8, s);
    }
  }

  wait_for_button();
}