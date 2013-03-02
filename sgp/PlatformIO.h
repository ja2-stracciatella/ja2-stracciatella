#pragma once

#include "Platform.h"

/* Platform and compiler-specifics */

/**************************************************************
 * Input/Output
 *************************************************************/

#if defined(_MSC_VER)
  #include <io.h>
  #include <direct.h>
  #define chdir(path)                               _chdir(path)
  #define chmod(path, access_mode)                  _chmod(path, access_mode)
  #define close(file_handle)                        _close(file_handle)
  #define fdopen(file_handle, format)               _fdopen(file_handle, format)
  #define fileno(file)                              _fileno(file)
  #define open(filename, flags)                     _open(filename, flags)
  #define open3(filename, flags, permission)        _open(filename, flags, permission)
  #define unlink(path)                              _unlink(path)
#else
  #include <unistd.h>
  #define open3(filename, flags, permission)        open(filename, flags, permission)

  /* #include <pwd.h> */

/* #  if defined __APPLE__ && defined __MACH__ */
/* #    include <CoreFoundation/CoreFoundation.h> */
/* #    include <sys/param.h> */
/* #  endif */

#endif

#if _WIN32
  #define mkdir(path, mode)                         _mkdir(path)
#endif

/**************************************************************
 *
 *************************************************************/
