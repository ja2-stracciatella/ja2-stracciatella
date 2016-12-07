#include "slog/slog.h"

#define TAG "test"

int main(void)
{
  /* Library initialization.  Must be called only once. */
  SLOG_Init(SLOG_STDERR, "example02-log.txt");

  /* Setting log levels: all messages to console and all to the file.
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
