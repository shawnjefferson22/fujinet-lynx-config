#include <6502.h>
#include <lynx.h>
#include <tgi.h>
#include <conio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "pathutil.h"


/* Takes the filename off leaving slash
 *
 */
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


/* does this path have a trailing slash (directory) ?
 *
 */
bool path_is_dir(const char *path)
{
  if (path[strlen(path)-1] == '/')
    return(true);
  else
    return(false);
}


/* return string after first occurence of c
 *
 */
char *_extract_at_char(const char *path, char c)
{
  char *cp;

  cp = strrchr(path, c);
  if (cp)
    return cp + 1;   // char after the last slash
  else
    return NULL;        // no slash entire string is filename?
}


/* return just the filename portion of the string
 *
 */
char *extract_filename(const char *path)
{
    if (path == NULL || *path == '\0')
        return NULL;

    if (path_is_dir(path))
        return NULL;

    return(_extract_at_char(path, '/'));
}


/*
 * return just the extension portion of the string
 */
char *extract_ext(const char *path)
{
  if (path == NULL || *path == '\0')
    return NULL;

  if (path_is_dir(path))
    return NULL;

  return(_extract_at_char(path, '.'));
}


/* return the lynx filetype based on the path passed in
 *
 */
unsigned char lynx_filetype(char *path)
{
  char *ext;

  ext = extract_ext(path);
  if (!ext)
    return(FILETYPE_NONE);

  if (((ext[0] == 'O') || (ext[0] == 'o')) && (ext[1] == '\0'))
    return(FILETYPE_OBJ);

  if (((ext[0] == 'L') || (ext[0] == 'l')) && ((ext[1] == 'N') || (ext[1] == 'n')) && ((ext[2] == 'X') || (ext[2] == 'x')))
    return(FILETYPE_LNX);

  if (((ext[0] == 'L') || (ext[0] == 'l')) && ((ext[1] == 'Y') || (ext[1] == 'y')) && ((ext[2] == 'X') || (ext[2] == 'x')))
    return(FILETYPE_LYX);

  if (((ext[0] == 'C') || (ext[0] == 'c')) && ((ext[1] == 'O') || (ext[1] == 'o')) && ((ext[2] == 'M') || (ext[2] == 'm')))
    return(FILETYPE_COM);

  return(FILETYPE_NONE);
}


/* Pad out the filename+ext to 11 characters
 * without the period.  Copy into the dest string.
 * dest string must be at least 12 characters
 * returns 1 on success, 0 on any failure
 */
#ifdef SDCARD_BENNVENN
bool extract_and_pad_filename(const char *filename, char *dest)
{
    size_t n;
    char fn[13];
    const char *ext;
    char s[40];

  	// extract filename.ext
  	strncpy(fn, filename, sizeof(fn) - 1);
    fn[sizeof(fn) - 1] = '\0';

 	  // find dot, and truncate filename portion
    n = strcspn(fn, ".");
    fn[n] = '\0';

	  // extract ext
	  ext = extract_ext(filename);

	  // sanity check
	  if (strlen(fn) > 8 || strlen(ext) > 3)
		  return(false);

    // return padded filename and extension
  	sprintf(dest, "%-8.8s%-3.3s", fn, ext);

    //sprintf(s, "dest:%s", dest);
    //tgi_outtextxy(0, 70, s);    
    //cgetc();

	  return(true);
}
#endif
