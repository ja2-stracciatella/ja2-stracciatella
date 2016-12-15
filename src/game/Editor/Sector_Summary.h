#ifndef __SECTOR_SUMMARY_H
#define __SECTOR_SUMMARY_H

#include "sgp/Input.h"


void CreateSummaryWindow(void);
void DestroySummaryWindow();
void RenderSummaryWindow(void);
void LoadWorldInfo(void);

void UpdateSectorSummary(const wchar_t* gszFilename, BOOLEAN fUpdate);

void SaveGlobalSummary(void);

extern BOOLEAN gfGlobalSummaryExists;

extern BOOLEAN gfSummaryWindowActive;

void AutoLoadMap(void);

BOOLEAN HandleSummaryInput(InputAtom*);

#endif
