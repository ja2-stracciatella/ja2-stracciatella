#ifndef __FILES_H
#define __FILES_H

#define FILES_DAT_FILE "Temp/files.dat"

void GameInitFiles(void);
void EnterFiles(void);
void ExitFiles(void);
void HandleFiles(void);
void RenderFiles(void);

extern BOOLEAN fEnteredFileViewerFromNewFileIcon;
extern BOOLEAN fNewFilesInFileViewer;

// add a file about this terrorist
BOOLEAN AddFileAboutTerrorist(INT32 iProfileId);

#endif
