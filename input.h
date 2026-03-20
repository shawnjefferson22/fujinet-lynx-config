/**
 *  for Atari Lynx Config
 *
 * @brief input routines
 * @author Shawn Jefferson
 * @email sjefferson at shaw dot ca
 */

#ifndef INPUT_H
#define INPUT_H


//void wait_for_button(void);
void wait_for_any_key(void);
uint8_t check_keypress(void);
uint8_t check_joy_and_keys(uint8_t *joy);
uint8_t check_joy_and_keys_loop(uint8_t *joy);
void debounce_joy_and_keys(void);

uint8_t get_input(uint8_t x, uint8_t y, uint8_t max, char *input);
uint8_t get_input_with_title(uint8_t y, uint8_t max, char *title, char *input);

uint8_t ask_yesno_download(uint8_t y);


#endif