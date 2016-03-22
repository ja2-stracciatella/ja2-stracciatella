/****************************************************************************
 *
 * This file is part of SLOG (Simple Logger for C and C++).
 *
 * Copyright (c) 2013 SLOG authors and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#include "slog.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#if defined(_MSC_VER)
  /* Visual Studio */
  #include <io.h>
  #define write _write
  #define snprintf(buf, size, format, ...) _snprintf_s(buf, size, _TRUNCATE, format, ##__VA_ARGS__)
  #define vsnprintf(buf, size, format, args) vsnprintf_s(buf, size, _TRUNCATE, format, args)
  #pragma warning( disable : 4996 )  /* disable some VS warnings, e.g. 'fopen': This function or variable may be unsafe. */
#else
  #include <unistd.h>
#endif


/************************************************************
 * Static data
 ************************************************************/

static int s_initialized = 0;
static int s_consoleFD;         /**< Console file descriptor. 0 - no, 1 - stdout, 2 - stderr. */
static FILE *s_logFile = NULL;  /**< File for logging. */

static SLOGLevel s_consoleLevel = SLOG_INFO;
static SLOGLevel s_fileLevel = SLOG_DEBUG;

/* gDebugFlags contains one flag per topic to enable (1) or disable (0) logging
 * default is all on for now. Watch the variable width when adding more topics! */
static long long gDebugFlags = 0xFFFFFFFFFFFFFFFF;

static char SLOGTags[NUMBER_OF_TOPICS][TAG_LENGTH + 1] =
{
  "Game Loop",
  "Strategic Map",
  "AI",
  "Scheduler",
  "Path AI",
  "Animation Ctl",
  "Gap",
  "Weapons",
  "Soldier List",
  "Animation Data",
  "Overhead",
  "Soldier Tile",
  "Keys",
  "Animation Cache",
  "Points",
  "Morale",
  "Merc Hire",
  "Opp List",
  "Lighting",
  "Render World",
  "Tile Def",
  "Explosion",
  "Physics",
  "Ambient",
  "Save/Load",
  "Music Control",
  "Smacker",
  "Event Pump",
  "Quests",
  "Editor",
  "Resources",
  "JA2 Screens",
  "Init",
  "Font",
  "SGP",
  "STCI",
  "Container",
  "VSurface",
  "Sound",
  "Memory Manager",
  "Game Screen",
  "Bobby Ray",
  "Strategic AI",
  "Air Raid",
  "Bullets",
  "Handle Items",
  "Interface",
  "Line of Sight",
  "Tactical Save",
  "Soldier Anim",
  "Soldier Ctrl",
  "Team Turns",
  "World Def",
  "Tile Animation",
  "Himage",
  "Library DB",
  "File Man",
  "Mod Pack",
  "Default CM",
  "Soldier",
  "Asserts"
};

/************************************************************
 * Function implementation
 ************************************************************/

void SLOG_Init(SLOGConsole console, const char *logFile)
{
  if(!s_initialized)
  {
    switch(console)
    {
    case SLOG_STDOUT:
      s_consoleFD = 1;
      break;
    case SLOG_STDERR:
      s_consoleFD = 2;
      break;
    default:
      s_consoleFD = 0;
    }

    if(logFile)
    {
      s_logFile = fopen(logFile, "wt");
      /* ignoring any errors */
    }

    s_initialized = 1;
  }
}

/** @brief Library deinitialization.
 *
 * This function should be called before program exit only once. */
void SLOG_Deinit()
{
  if(s_logFile)
  {
    fclose(s_logFile);
    s_logFile = NULL;
  }
}


void SLOG_SetLevel(SLOGLevel console, SLOGLevel file)
{
  s_consoleLevel = console;
  s_fileLevel = file;
}


/** Get level name for including into the log. */
static const char* getLevelName(SLOGLevel level)
{
  switch(level)
  {
  case SLOG_DEBUG:      return "DEBUG";
  case SLOG_INFO:       return "INFO";
  case SLOG_WARNING:    return "WARNING";
  case SLOG_ERROR:      return "ERROR";
  }
  return "";
}

void SLOG_LogMessage(SLOGLevel level, SLOGTopics tag, const char *format, ...)
{
  if ( !(gDebugFlags & (1 << tag))) return;
  int logToConsole = (s_consoleFD != 0) && (level >= s_consoleLevel);
  int logToFile = (s_logFile != NULL) && (level >= s_fileLevel);

  if(logToConsole || logToFile)
  {
    struct tm *localTime;
    time_t currentTime;
    va_list args;
    char buf[256];
    int size = 0;

    currentTime = time(NULL);
    localTime = localtime(&currentTime);

    /* print time, level and tag */
    size += snprintf(buf + size, sizeof(buf) - size, "%4d/%02d/%02d %02d:%02d:%02d %-7s [%-16s] ",
                     localTime->tm_year + 1900, localTime->tm_mon, localTime->tm_mday,
                     localTime->tm_hour, localTime->tm_min, localTime->tm_sec,
                     getLevelName(level), SLOGTags[tag]);

    /* print message */
    va_start(args, format);
    size += vsnprintf(buf + size, sizeof(buf) - size, format, args);
    va_end(args);

    if(size >= (int)sizeof(buf))
    {
      /* there were not enough space in the buf */
      size = sizeof(buf) - 1;
    }

    buf[size] = 0;

    if(logToConsole)
    {
      write(s_consoleFD, buf, size);
    }

    if(logToFile)
    {
      fputs(buf, s_logFile);
      fflush(s_logFile);
    }
  }
}
