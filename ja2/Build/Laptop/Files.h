#ifndef __FILES_H
#define __FILES_H


#define	FILES_DAT_FILE			"Temp/files.dat"

void GameInitFiles();
void EnterFiles();
void ExitFiles();
void HandleFiles();
void RenderFiles();


struct files{
 UINT8 ubCode; // the code index in the files code table
 UINT8 ubFormat; // layout format
 UINT32 uiIdNumber; // unique id number
 UINT32 uiDate; // time in the world in global time (resolution, minutes)
 BOOLEAN fRead;
	char* pPicFileNameList[2];

 struct files *Next; // next unit in the list

};

struct filestring{
	STR16 pString;
	struct filestring *Next;
};

typedef struct filestring FileString;
typedef struct filestring *FileStringPtr;

// files codes
enum{
	FILES_WELCOME_NOTICE,
};


// special codes for special files
enum{
	ENRICO_BACKGROUND = 0,
	SLAY_BACKGROUND,
	MATRON_BACKGROUND,
	IMPOSTER_BACKGROUND,
	TIFFANY_BACKGROUND,
	REXALL_BACKGROUND,
	ELGIN_BACKGROUND,

};
extern UINT8 ubFileRecordsLength[];
extern BOOLEAN fEnteredFileViewerFromNewFileIcon;
extern BOOLEAN fNewFilesInFileViewer;



typedef struct files FilesUnit;
typedef struct files *FilesUnitPtr;

extern FilesUnitPtr pFilesListHead;

struct filerecordwidth{
	INT32 iRecordNumber;
	INT32 iRecordWidth;
	INT32 iRecordHeightAdjustment;
	UINT8 ubFlags;
	struct filerecordwidth *Next;
};

typedef struct filerecordwidth FileRecordWidth;
typedef struct filerecordwidth *FileRecordWidthPtr;

// add a file about this terrorist
BOOLEAN AddFileAboutTerrorist( INT32 iProfileId );

#endif
