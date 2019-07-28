#include "Logger.h"

#include <stdio.h>
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

static char SLOGTags[NUMBER_OF_TOPICS][TAG_LENGTH + 1] =
{
  "Game Loop",
  "Strategic Map",
  "AI",
  "Scheduler",
  "Path AI",
  "Animations",
  "Gap",
  "Weapons",
  "Overhead",
  "Keys",
  "Points",
  "Morale",
  "Merc Hire",
  "Opp List",
  "Lighting",
  "Render World",
  "Tiles",
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
  "Video",
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
  "Team Turns",
  "World Def",
  "Himage",
  "Library DB",
  "File Man",
  "Mod Pack",
  "Default CM",
  "Soldier",
  "Asserts",
  "Queen Command",
  "NPC",
  "Campaign",
  "Loyalty",
  "Auto Resolve",
  "Corpses",
  "Fixme",
  "Launcher"
};

void LogMessage(LogLevel level, SLOGTopics tag, const char *format, ...) {
  char buf[256];
  va_list args;

  va_start(args, format);
  vsnprintf(buf, 256, format, args);
  va_end(args);

  Logger_LogWithCustomMetadata(level, buf);

  #ifdef ENABLE_ASSERTS
    if (tag == DEBUG_TAG_ASSERTS)
    {
      abort();
    }
  #endif
}