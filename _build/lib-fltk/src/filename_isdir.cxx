//
// "$Id: filename_isdir.cxx 9325 2012-04-05 05:12:30Z fabien $"
//
// Directory detection routines for the Fast Light Tool Kit (FLTK).
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

// Used by fl_file_chooser

#include "flstring.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <FL/filename.H>
#include <FL/fl_utf8.h>


#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
static inline int isdirsep(char c) {return c=='/' || c=='\\';}
#else
#define isdirsep(c) ((c)=='/')
#endif

int _fl_filename_isdir_quick(const char* n) {
  // Do a quick optimization for filenames with a trailing slash...
  if (*n && isdirsep(n[strlen(n) - 1])) return 1;
  return fl_filename_isdir(n);
}

/**
   Determines if a file exists and is a directory from its filename.
   \code
   #include <FL/filename.H>
   [..]
   fl_filename_isdir("/etc");		// returns non-zero
   fl_filename_isdir("/etc/hosts");	// returns 0
   \endcode
   \param[in] n the filename to parse
   \return non zero if file exists and is a directory, zero otherwise
*/
int fl_filename_isdir(const char* n) {
  struct stat	s;
  char		fn[FL_PATH_MAX];
  int		length;

  length = (int) strlen(n);

#ifdef WIN32
  // This workaround brought to you by the fine folks at Microsoft!
  // (read lots of sarcasm in that...)
  if (length < (int)(sizeof(fn) - 1)) {
    if (length < 4 && isalpha(n[0]) && n[1] == ':' &&
        (isdirsep(n[2]) || !n[2])) {
      // Always use D:/ for drive letters
      fn[0] = n[0];
      strcpy(fn + 1, ":/");
      n = fn;
    } else if (length > 0 && isdirsep(n[length - 1])) {
      // Strip trailing slash from name...
      length --;
      memcpy(fn, n, length);
      fn[length] = '\0';
      n = fn;
    }
  }
#else
  // Matt: Just in case, we strip the slash for other operating
  // systems as well, avoid bugs by sloppy implementations
  // of "stat".
  if (length > 1 && isdirsep(n[length - 1])) {
    length --;
    memcpy(fn, n, length);
    fn[length] = '\0';
    n = fn;
  }
#endif

  return !fl_stat(n, &s) && (s.st_mode&0170000)==0040000;
}

//
// End of "$Id: filename_isdir.cxx 9325 2012-04-05 05:12:30Z fabien $".
//
