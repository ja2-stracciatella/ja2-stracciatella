# SLOG - Simple Logger for C and C++

This is a simple logging library for C and C++.

## Usage

- add files slog.h and slog.c into your project;
- initialize the library on program start, for example in main function;
- use macroses SLOG\* for logging.

### Example

```
#include "slog/slog.h"

#define TAG "test"

int main(void)
{
  /* Library initialization.  Must be called only once. */
  SLOG_Init(SLOG_STDERR, "example01-log.txt");

  /* Settings log level.
   * Log all messages to console and all to the file.
   * Default settings are: all messages to file and >= INFO to console. */
  SLOG_SetLevel(SLOG_DEBUG, SLOG_DEBUG);

  SLOGD(TAG, "debug message");
  SLOGI(TAG, "info");
  SLOGW(TAG, "warning");
  SLOGE(TAG, "error");

  SLOGI(TAG, "formatted message (%d, %s)", 123, "bar");

  /* Library deinitialization. */
  SLOG_Deinit();
  return 0;
}
```

Output of the program:

```
2013/03/14 00:42:56 DEBUG   [test            ] debug message
2013/03/14 00:42:56 INFO    [test            ] info
2013/03/14 00:42:56 WARNING [test            ] warning
2013/03/14 00:42:56 ERROR   [test            ] error
2013/03/14 00:42:56 INFO    [test            ] formatted message (123, bar)
```

## Design goals

- simple in usage and implementation;
- fast compilation;
- logging to console and/or into a file;
- suitable for C and C++;
- printf-like formatting;
- no overhead when logging is disabled;
- cross-platform (Unix, Windows);
- suitable for open-source and commercial products;

## License

_The MIT License (MIT)_

You can use this library in commercial and open-source projects. No attribution is required. Please see the full text of the license in LICENSE.txt

## Resources

Home page: [https://bitbucket.org/gennady/slog](https://bitbucket.org/gennady/slog)
