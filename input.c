#include <6502.h>
#include <conio.h>
#include <tgi.h>
#include <lynx.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <joystick.h>
#include "display.h"
#include "input.h"
#include "sdcard.h"


char input_chars[] = {"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890.,!@#$%^&*()[]<>/?:; _"};


// Finds the passed in character in the input_char array and returns the index, or 0
unsigned char _find_char(unsigned char c)
{
  unsigned char i;

  for(i=0; i<strlen(input_chars); ++i) {
     if (input_chars[i] == c)
       return(i);
  }

  return(0);
}


// removes a character from the string at position i
void _remove_char(char *s, unsigned char i)
{
  unsigned char n;

  for(n=i; n<strlen(s); ++n) {
    s[n] = s[n+1];
  }
}


// string passed in must have max size-2 to insert a char
void _insert_char(char *s, unsigned char i)
{
  unsigned char n;

  s[strlen(s)+1] = '\0';            // make sure new string is null terminated!
  for(n=strlen(s)+1; n>i; --n) {
    s[n] = s[n-1];
  }

  s[i] = input_chars[0];
}


// waits for either button A or B to be pressed
void wait_for_button(void)
{
  unsigned int joy;


  while (1) {
    joy = joy_read(0);
    if (JOY_BTN_1(joy) || JOY_BTN_2(joy))
      break;
  }
  while (joy_read(0) == joy);     // debounce
}


/* check_joy_and_keys
 *
 * Joystick and key check, doing debounce for joystick and keys
 *
 * Returns the keypressed, or 0 if no key pressed, and status of joystick in joy.
 * Check if a key is pressed on the lynx and if so, return the key char.
 * The keyboard returns '1', '2', '3', 'P', 'R', 'F' or '?'.
 * Pause and Opt1 at the same time means Restart and pressing Pause and Opt2 means Flip screen.
 *
 * Opt1 = 1
 * Opt2 = 2
 * Opt1+Opt2 = 3
 * Pause = P
 * Opt1+Pause = Restart (R)
 * Opt2+Pause = Flip screen (F)
 */
unsigned char check_joy_and_keys(unsigned char *joy)
{
  unsigned char c;


  // keys input
  if (kbhit()) {
    c = cgetc();
    while (kbhit());					// debouce key
    return(c);
  }

  // joystick input
  *joy = joy_read(0);
  if (*joy) {
    while (joy_read(0) == *joy);     // debounce joystick
    return(0);
  }
}


/* get_input
 *
 * get text input from the user, at X, Y on screen, max chars and store in input
 * returns 1 if enter, 0 if esc.
 *
 * max must be an array large enough to hold input + null
 */
uint8_t get_input(uint8_t x, uint8_t y, uint8_t max, char *input)
{
  uint8_t i;    // current location in string
  uint8_t j;	// location in input_chars
  uint8_t st;   // start of string to display
  uint8_t joy, c;


  // init input
  i = st = 0;

  // set initial char
  j = _find_char(input[0]);                // find first char in input char array

  // debounce the button/joystick
  while (joy_read(0));

  while(1) {
    tgi_setbgcolor(TGI_COLOR_DARKGREY);
    sprintf(s, "%-19.19s", &input[st]);
    tgi_outtextxy(x, y, s);
    tgi_setbgcolor(TGI_COLOR_BLACK);

    tgi_setcolor(TGI_COLOR_BLACK);          // erase green cursor
    tgi_bar(x, y+7, (x*8)*max, y+8);

    tgi_setcolor(TGI_COLOR_GREEN);
    tgi_bar((i*8)+x, y+7, (i*8)+8+x, y+8);	// draw underline cursor
    tgi_setcolor(TGI_COLOR_WHITE);

    do {
      c = check_joy_and_keys(&joy);
    } while (!c && !joy);

    switch(c) {
      case '1':                         // delete char
        _remove_char(&input[0], i);
        i--;
        continue;
      case '2':                         // insert char
        _insert_char(&input[0], i);
        continue;
      case '3':                         // clear string
        memset(input, 0, max);
        continue;
    }

    // select next character
    if (JOY_UP(joy)) {
      j++;
      if (j > strlen(input_chars))			// wrap around?
        j = 0;
      input[i] = input_chars[j];
    }

    // select previous character
    if (JOY_DOWN(joy)) {
      input[i]++;
      if (j == 0)							// wrap around?
        j = strlen(input_chars)-1;
      else
        j--;

      input[i] = input_chars[j];
    }

    if (JOY_LEFT(joy)) {
      if (i > 0) {
        if (st > 0) {
          if (i == st)
            --st;
        }
        --i;
      }
    }
    if (JOY_RIGHT(joy)) {
      if (i < (max-2)) {					// need room for trailing null character
        ++i;
        if (input[i] == 0)
          input[i] = input_chars[j];
        if (i == 18)
          st++;
      }
    }

    // A button accepts
    if (JOY_BTN_1(joy)) {
      input[max-1] = '\0';				// just in case, string was cleared with nulls though
      break;
    }

    // B button cancels
    if (JOY_BTN_2(joy))
      return(0);
  }

  return(1);
}


/* get_input_with_title
 *
 * Draw a dialog box at y, with title, and get input
 *
 */
uint8_t get_input_with_title(uint8_t y, uint8_t max, char *title, char *input)
{
  uint8_t r;

                                                          //012345678901234567890
  draw_box_with_text(0, y, 159, y+24, TGI_COLOR_RED, title, "A=Enter B=Esc");
  r = get_input(2, y+8, max, input);

  return(r);
}
