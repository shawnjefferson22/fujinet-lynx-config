#ifndef PATHUTIL_H
#define PATHUTIL_H

enum lynx_filetypes {
    FILETYPE_NONE,
    FILETYPE_OBJ,
    FILETYPE_LNX,
    FILETYPE_LYX,
    FILETYPE_COM
};

void strip_dir_from_path(char *path);
uint8_t path_is_dir(char *path);
char *extract_filename(char *path);
char *extract_ext(char *path);
unsigned char lynx_filetype(char *path);

#endif