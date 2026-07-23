#ifndef PATHUTIL_H
#define PATHUTIL_H

#include <stdbool.h>


enum lynx_filetypes {
    FILETYPE_NONE,
    FILETYPE_OBJ,
    FILETYPE_LNX,
    FILETYPE_LYX,
    FILETYPE_COM
};

unsigned char lynx_filetype(char *path);

void strip_dir_from_path(char *path);
bool path_is_dir(const char *path);
char *extract_filename(const char *path);
char *extract_ext(const char *path);
bool extract_and_pad_filename(const char *filename, char *dest);

char *_extract_at_char(const char *path, char c);

#endif