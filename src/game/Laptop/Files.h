#ifndef __FILES_H
#define __FILES_H

#include "Types.h"

#define FILES_DATA_FILE "files.dat"

void GameInitFiles(void);
void EnterFiles(void);
void ExitFiles(void);
void HandleFiles(void);
void RenderFiles(void);

extern BOOLEAN fEnteredFileViewerFromNewFileIcon;
extern BOOLEAN fNewFilesInFileViewer;

// add all files about terrorists
void AddFilesAboutTerrorists();

#endif
