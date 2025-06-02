/**
 *  for Atari Lynx Config
 *
 * @brief input routines
 * @author Shawn Jefferson
 * @email sjefferson at shaw dot ca
 */

#ifndef INPUT_H
#define INPUT_H


void wait_for_button(void);
unsigned char check_keypress(void);
unsigned char check_joy_and_keys(unsigned char *joy);

uint8_t get_input(uint8_t x, uint8_t y, uint8_t max, char *input);
uint8_t get_input_with_title(uint8_t y, uint8_t max, char *title, char *input);


#endif