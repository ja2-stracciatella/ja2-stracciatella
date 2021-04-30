#ifndef __SECTOR_SUMMARY_H
#define __SECTOR_SUMMARY_H

#include "Input.h"

#include <string_theory/string>


void CreateSummaryWindow(void);
void DestroySummaryWindow();
void RenderSummaryWindow(void);
void LoadWorldInfo(void);

void UpdateSectorSummary(const ST::string& gszFilename, BOOLEAN fUpdate);

void SaveGlobalSummary(void);

extern BOOLEAN gfGlobalSummaryExists;

extern BOOLEAN gfSummaryWindowActive;

extern BOOLEAN gfSummaryWindowSaveRequested;

void AutoLoadMap(void);

BOOLEAN HandleSummaryInput(InputAtom*);

#endif
