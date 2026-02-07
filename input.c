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
#define INPUT_CHAR_LEN (sizeof(input_chars) - 1)


// Finds the passed in character in the input_char array and returns the index, or 0
unsigned char _find_char(unsigned char c)
{
  unsigned char i;
  unsigned char len;

  len = strlen(input_chars);
  for(i=0; i<len; ++i) {
     if (input_chars[i] == c)
       return(i);
  }

  return(0);
}


// removes a character from the string at position i
void _remove_char(char *s, unsigned char i)
{
  unsigned char n;
  unsigned char max;


  max = strlen(s);
  if (i >= max)
    return;

  for(n=i; n<max; ++n) {
    s[n] = s[n+1];
  }
}


void _insert_char(char *s, unsigned char i)
{
    unsigned char len;
  

  len = strlen(s);
  if (i > len)        // invalid index
    return;

    // shift right, including '\0'
    memmove(&s[i + 1], &s[i], len - i + 1);

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
 * input must be an array large enough to hold max + 1 (null byte)
 */
uint8_t get_input(uint8_t x, uint8_t y, uint8_t max, char *input)
{
  uint8_t i;    // current location in string
  uint8_t j;	  // location in input_chars
  uint8_t st;   // start of string to display
  uint8_t len;
  uint8_t joy, c;


  // init input
  i = st = 0;

  // set initial char
  if (input[0] != '\0')
    j = _find_char(input[0]);                // find first char in input char array
  else {
    j = 0;
    input[0] = input_chars[0];
    input[1] = '\0';
  }

  // debounce the button/joystick
  while (joy_read(0));

  while(1) {
    // display the text
    tgi_setbgcolor(TGI_COLOR_DARKGREY);
    sprintf(s, "%-19.19s", &input[0]);
    tgi_outtextxy(x, y, s);
    tgi_setbgcolor(TGI_COLOR_BLACK);

    // erase the cursor below the whole area
    tgi_setcolor(TGI_COLOR_BLACK);
    tgi_bar(x, y+7, x + (max * 8), y+8);

    // display cursor below character
    tgi_setcolor(TGI_COLOR_GREEN);
    tgi_bar((i*8)+x, y+7, (i*8)+8+x, y+8);	// draw underline cursor
    tgi_setcolor(TGI_COLOR_WHITE);

    do {
      c = check_joy_and_keys(&joy);
    } while (!c && !joy);

    // Get the current string length
    len = strlen(input);

    // Process keys
    switch(c) {
      // delete char
      case '1':
        _remove_char(&input[0], i);   // remove character at i
          
        len = strlen(input);          // new length
        if (i > 0 && i >= len)
          i--;                        // move us back one character

        if (input[i] == '\0')          // nothing here, set to first letter
          input[i] = input_chars[0];
        continue;
      // insert char
      case '2':
        if (len < max) {                // already maxed out?
          _insert_char(&input[0], i);   // insert a character at i
        }
        continue;
      // clear string
      case '3':
        memset(input, 0, max+1);
        i = 0;
        continue;
    }

    // Process joystick input
    // select next character 
    if (JOY_UP(joy)) {                  
      j++;
      if (j == INPUT_CHAR_LEN)			// wrap around?
        j = 0;
      
      input[i] = input_chars[j];        // set the character in string
    }

    // select previous character
    if (JOY_DOWN(joy)) {
      if (j == 0)							          // wrap around?
        j = INPUT_CHAR_LEN-1;
      else
        j--;

      input[i] = input_chars[j];        // set the character in string
    }

    // move left in string 
    if (JOY_LEFT(joy)) {
      if (i > 0) {
        if (st > 0) {
          if (i == st)
            --st;
        }
        --i;
      
        j = _find_char(input[i]);
      }
      continue;
    }

    // move right in string
    if (JOY_RIGHT(joy)) {
      if (i < max-1) {
        ++i;
        
        if (i == len) {
          input[i] = input_chars[j];
          input[i+1] = '\0';
        }
        else 
          j = _find_char(input[i]);
        
        //if (i == 18)
        //  st++;
      }
      continue;
    }

    // A button accepts
    if (JOY_BTN_1(joy)) {
      input[max] = '\0';				// just in case
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
