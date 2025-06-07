/**
 *  for Atari Lynx Config
 *
 * @brief input routines
 * @author Shawn Jefferson
 * @email sjefferson at shaw dot ca
 */

#ifndef DISPLAY_H
#define DISPLAY_H


extern char s[21];                     // buffer for text display
extern char filenames[10][64];         // filename display (704 bytes), filenames capped to 64 bytes


void draw_box_with_text(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color, char *title, char *prompt);
void print_key_legend(char *legend);
void print_error(char *msg);
void display_error_and_wait(char *msg);

void display_adapter_config(void);
void display_hosts(unsigned char sel);
void display_files(unsigned char sel);
void scroll_file_entry(unsigned char sel, unsigned char st);

void display_file_data(void);

#endif