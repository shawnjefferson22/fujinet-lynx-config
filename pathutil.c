#include <6502.h>
#include <lynx.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "pathutil.h"


void strip_dir_from_path(char *path)
{
  unsigned char i;


  for (i=strlen(path)-2; i>0; --i)		// start before the trailing '/'
    if (path[i] == '/') {
      path[i+1] = '\0';
      return;
    }

  strcpy(path, "/");       				// either no directory or at root
}


uint8_t path_is_dir(char *path)
{
  if (path[strlen(path)-1] == '/')
    return(1);
  else
    return(0);
}


char *_extract_at_char(char *path, char c)
{
  char *cp;

  cp = strrchr(path, c);
  if (cp)
    return cp + 1;   // char after the last slash
  else
    return NULL;        // no slash entire string is filename?
}


char *extract_filename(char *path)
{
    if (path == NULL || *path == '\0')
        return NULL;

    if (path_is_dir(path))
        return NULL;

    return(_extract_at_char(path, '/'));

 /*   slash = strrchr(path, '/');
    if (slash)
        return slash + 1;   // char after the last slash
    else
        return NULL;        // no slash entire string is filename?
*/
}


char *extract_ext(char *path)
{
  if (path == NULL || *path == '\0')
    return NULL;

  if (path_is_dir(path))
    return NULL;

  return(_extract_at_char(path, '.'));
}


unsigned char lynx_filetype(char *path)
{
  char *ext;

  ext = extract_ext(path);
  if (!ext)
    return(FILETYPE_NONE);

  if (((ext[0] == 'O') || (ext[0] == 'o')) && (ext[1] == '\0'))
    return(FILETYPE_OBJ);

  if (((ext[0] == 'L') || (ext[0] == 'l')) && ((ext[1] == 'N') || (ext[1] == 'n')) && ((ext[0] == 'X') || (ext[0] == 'x')))
    return(FILETYPE_LNX);

  if (((ext[0] == 'L') || (ext[0] == 'l')) && ((ext[1] == 'Y') || (ext[1] == 'y')) && ((ext[0] == 'X') || (ext[0] == 'x')))
    return(FILETYPE_LYX);

  if (((ext[0] == 'C') || (ext[0] == 'c')) && ((ext[1] == 'O') || (ext[1] == 'o')) && ((ext[0] == 'M') || (ext[0] == 'm')))
    return(FILETYPE_COM);

  return(FILETYPE_NONE);
} 