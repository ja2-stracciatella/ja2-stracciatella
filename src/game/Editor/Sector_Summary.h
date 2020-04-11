#ifndef __SECTOR_SUMMARY_H
#define __SECTOR_SUMMARY_H

#include "Input.h"

#include <string_theory/string>


void CreateSummaryWindow(void);
void DestroySummaryWindow();
void RenderSummaryWindow(void);
void LoadWorldInfo(void);

void UpdateSectorSummary(const wchar_t* gszFilename, BOOLEAN fUpdate);
inline void UpdateSectorSummary(const ST::string& gszFilename, BOOLEAN fUpdate)
{
        ST::wchar_buffer wstr = gszFilename.to_wchar();
        UpdateSectorSummary(wstr.c_str(), fUpdate);
}

void SaveGlobalSummary(void);

extern BOOLEAN gfGlobalSummaryExists;

extern BOOLEAN gfSummaryWindowActive;

void AutoLoadMap(void);

BOOLEAN HandleSummaryInput(InputAtom*);

#endif
