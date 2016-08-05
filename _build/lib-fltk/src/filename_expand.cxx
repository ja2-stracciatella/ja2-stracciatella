//
// "$Id: filename_expand.cxx 9325 2012-04-05 05:12:30Z fabien $"
//
// Filename expansion routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

/* expand a file name by substuting environment variables and
   home directories.  Returns true if any changes were made.
   to & from may be the same buffer.
*/

#include <FL/filename.H>
#include <FL/fl_utf8.h>
#include <stdlib.h>
#include "flstring.h"
#if defined(WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#else
# include <unistd.h>
# include <pwd.h>
#endif

#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
static inline int isdirsep(char c) {return c=='/' || c=='\\';}
#else
#define isdirsep(c) ((c)=='/')
#endif

/** Expands a filename containing shell variables and tilde (~).
    Currently handles these variants:
    \code
    "~username"               // if 'username' does not exist, result will be unchanged
    "~/file"
    "$VARNAME"                // does NOT handle ${VARNAME}
    \endcode

    \b Examples:
    \code
    #include <FL/filename.H>
    [..]
    putenv("TMPDIR=/var/tmp");
    fl_filename_expand(out, sizeof(out), "~fred/.cshrc");     // out="/usr/fred/.cshrc"
    fl_filename_expand(out, sizeof(out), "~/.cshrc");         // out="/usr/<yourname>/.cshrc"
    fl_filename_expand(out, sizeof(out), "$TMPDIR/foo.txt");  // out="/var/tmp/foo.txt"
    \endcode
    \param[out] to resulting expanded filename
    \param[in]  tolen size of the expanded filename buffer 
    \param[in]  from filename containing shell variables
    \return 0 if no change, non zero otherwise
 */
int fl_filename_expand(char *to,int tolen, const char *from) {

  char *temp = new char[tolen];
  strlcpy(temp,from, tolen);
  char *start = temp;
  char *end = temp+strlen(temp);

  int ret = 0;

  for (char *a=temp; a<end; ) {	// for each slash component
    char *e; for (e=a; e<end && !isdirsep(*e); e++); // find next slash
    const char *value = 0; // this will point at substitute value
    switch (*a) {
    case '~':	// a home directory name
      if (e <= a+1) {	// current user's directory
        value = fl_getenv("HOME");
#ifndef WIN32
      } else {	// another user's directory
	struct passwd *pwd;
	char t = *e; *(char *)e = 0; 
        pwd = getpwnam(a+1); 
        *(char *)e = t;
	    if (pwd) value = pwd->pw_dir;
#endif
      }
      break;
    case '$':		/* an environment variable */
      {char t = *e; *(char *)e = 0; value = fl_getenv(a+1); *(char *)e = t;}
      break;
    }
    if (value) {
      // substitutions that start with slash delete everything before them:
      if (isdirsep(value[0])) start = a;
#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
      // also if it starts with "A:"
      if (value[0] && value[1]==':') start = a;
#endif
      int t = (int) strlen(value); if (isdirsep(value[t-1])) t--;
      if ((end+1-e+t) >= tolen) end += tolen - (end+1-e+t);
      memmove(a+t, e, end+1-e);
      end = a+t+(end-e);
      *end = '\0';
      memcpy(a, value, t);
      ret++;
    } else {
      a = e+1;
#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
      if (*e == '\\') {*e = '/'; ret++;} // ha ha!
#endif
    }
  }

  strlcpy(to, start, tolen);

  delete[] temp;

  return ret;
}


//
// End of "$Id: filename_expand.cxx 9325 2012-04-05 05:12:30Z fabien $".
//
