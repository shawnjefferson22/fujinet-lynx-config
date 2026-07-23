/**
 * Lynx Fujinet Config
 */

#include <6502.h>
#include <lynx.h>
#include <conio.h>
#include <tgi.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <joystick.h>
#include "display.h"
#include "fujinet.h"
#include "fujidisk.h"
#include "input.h"
#include "lynxfnio.h"
#include "pathutil.h"
#include "screens.h"
#include "sdcard.h"
#include "bennvenn.h"
#include "logo.h"
#include "Program.h"


void main(void)
{
  unsigned char r, j;

  // Setup TGI
  tgi_install(tgi_static_stddrv);
  tgi_init();
  tgi_setdrawpage(0);
  tgi_setviewpage(0);

  // setup joystick
  joy_install(joy_static_stddrv);

  // Start Comlynx for Fujinet
  fnio_init();

  // display splash screen
  tgi_setcolor(TGI_COLOR_WHITE);
  tgi_setbgcolor(TGI_COLOR_BLACK);
  tgi_clear();
  display_splash_screen();

  // Check wifi status, if not connected do select ssid
  r = fujinet_get_wifi_status();
  if (!r) {
    r = select_wifi_network();
    if (!r) {
      tgi_setcolor(TGI_COLOR_RED);
      tgi_outtextxy(1, 8, "No wifi connection!");

      while(1) {
        r = check_joy_and_keys(&j);
        if (r == ADAPTERCFG_KEY)
          display_adapter_config();
      }
    }
  }

  // read host slots
  r = read_hosts();
  if (!r)
    while(1);

  // Hosts and files loop
  while(1) {
    r = select_host();
    if (!r)                     // bad return, don't view files
      continue;
    select_files();
  }
}