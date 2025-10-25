#include <6502.h>
#include <lynx.h>
#include <tgi.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <joystick.h>
#include "display.h"
#include "input.h"
#include "fujinet.h"
#include "fujidisk.h"
#include "logo.h"
#include "sprites.h"


#define KEY_LINE	95
#define ERROR_LINE  95


char s[21];                     // buffer for text display
char filenames[10][64];         // filename display (704 bytes), filenames capped to 64 bytes

// wifi strength sprite
SCB_REHV_PAL full_wifi_sprite = {
    BPP_4 | TYPE_NORMAL,
    REHV,
    0x01,
    0,
    (unsigned char *) &full_wifi_spr,
    149, 0,													// always at x = 149
    0x0100, 0x0100,
    { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }		// only two pens used, background, white
};

SCB_REHV_PAL medlow_wifi_sprite = {
    BPP_4 | TYPE_NORMAL,
    REHV,
    0x01,
    0,
    NULL,
    149, 0,													// always at x = 149
    0x0100, 0x0100,
    { 0xF7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }		// only three pens used, black, white and grey
};


//unsigned char wifi_full_pens[8] = { 0x0F, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00 };
//unsigned char wifi_medlow_pens[8] = { 0xF7, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00 };
// in the default palette, pens to colors:
// 0 = Black / Transparent
// 7 - Grey
// F - White
//
// 1 = Red
// 3 = Green
// 14 = yellow


// display wifi strength sprite
void display_wifi_sprite(uint8_t y, int8_t rssi)
{


  // don't display anything for custom SSID entry
  if (rssi == 127)
    return;

	if (rssi > -50) {
    full_wifi_sprite.vpos = y;
	  tgi_sprite(&full_wifi_sprite);
    return;
	}
	else if (rssi > -70) {
		medlow_wifi_sprite.data = (unsigned char *) &med_wifi_spr;
	}
	else {
		medlow_wifi_sprite.data = (unsigned char *) &low_wifi_spr;
	}

  medlow_wifi_sprite.vpos = y;
	tgi_sprite(&medlow_wifi_sprite);
}


// display splash screen sprite, waiting 5 seconds or until key pressed
void display_splash_screen(void)
{
  unsigned long t;
  unsigned char r, joy;


  SCB_REHV_PAL logo_sprite = {
    BPP_4 | TYPE_NORMAL,
    REHV,
    0x01,
    0,
    (unsigned char *) &fujinet_logo,
    0, 0,
    0x0100, 0x0100,
    { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF }
  };

  tgi_setpalette(&fujinet_logo_pal);
  tgi_sprite(&logo_sprite);
                       //01234567890
  tgi_outtextxy(72, 92, "Config v1.0");

  t = clock();
  while (((clock() - t) / CLOCKS_PER_SEC) < 5) {
  	r = check_joy_and_keys(&joy);
  	if (r || joy)
  		break;
  }

  tgi_setpalette(tgi_getdefpalette());
}


// draws a box, with title and prompt from x1,y1, to x2,y2.
// @param color  color of line to draw
// @param title  tile to draw in top left corner
// @param prompt  instructions to draw in bottom left corner
void draw_box_with_text(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color, char *title, char *prompt)
{
  // clear under box
  tgi_setcolor(TGI_COLOR_BLACK);
  tgi_setbgcolor(TGI_COLOR_BLACK);
  tgi_bar(x1, y1, x2, y2);

  // draw box, offsetting slightly for text
  tgi_setcolor(color);
  tgi_line(x1, y1+4, x2, y1+4);     // top line
  tgi_lineto(x2, y2);				        // right line
  tgi_lineto(x1, y2);				        // bottom line
  tgi_lineto(x1, y1+4);				      // left line

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


/* clear_error_line
 *
 */
void clear_error_line(void)
{
  tgi_setcolor(TGI_COLOR_BLACK);
  tgi_bar(0, ERROR_LINE, 159, ERROR_LINE+8);
  tgi_setcolor(TGI_COLOR_WHITE);
}


/*  print_error
 *
 * displays an error message on last line, in red
 */
void print_error(char *msg)
{
  clear_error_line();
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
  clear_error_line();
}


void display_adapter_config(void)
{
  unsigned char r, j;


  tgi_clear();
  draw_box_with_text(0, 0, 159, 101, TGI_COLOR_BLUE, "Adapter Config", NULL);

  r = fujinet_get_adapter_config();
  if (!r) {
    print_error("Error reading config!");
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

  // wait for some keypress
  do {
    r = check_joy_and_keys(&j);
  } while (!r && !j);
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
     for(i=0; i<16; ++i) {
      sprintf(s, "%02X", dskbuf[(j*16)+i]);
      tgi_outtextxy(i*16, j*8, s);
    }
  }

  wait_for_button();
}