#include "Debug.h"
#include "LoadSaveData.h"
#include "LoadSaveEMail.h"
#include "SGPFile.h"


static void LoadEMailFromFile(HWFILE const File)
{
	UINT32 uiSizeOfSubject;
	File->read(&uiSizeOfSubject, sizeof(UINT32)); // XXX HACK000B
	File->seek(uiSizeOfSubject, FILE_SEEK_FROM_CURRENT); // XXX HACK000B

	UINT16	usOffset;
	UINT16	usLength;
	UINT8		ubSender;
	UINT32	iDate;
	INT32		iFirstData;
	UINT32	uiSecondData;
	BOOLEAN fRead;

	BYTE Data[44];
	File->read(Data, sizeof(Data));

	DataReader S{Data};
	EXTR_U16(S, usOffset)
	EXTR_U16(S, usLength)
	EXTR_U8(S, ubSender)
	EXTR_SKIP(S, 3)
	EXTR_U32(S, iDate)
	EXTR_SKIP(S, 4)
	EXTR_I32(S, iFirstData)
	EXTR_U32(S, uiSecondData)
	EXTR_SKIP(S, 16)
	EXTR_BOOL(S, fRead)
	EXTR_SKIP(S, 3)
	Assert(S.getConsumed() == lengthof(Data));

	AddEmailMessage(usOffset, usLength, iDate, ubSender, fRead, iFirstData, uiSecondData);
}


void LoadEmailFromSavedGame(HWFILE const File)
{
	ShutDownEmailList();

	GameInitEmail();

	UINT32 uiNumOfEmails;
	File->read(&uiNumOfEmails, sizeof(UINT32));

	for (UINT32 cnt = 0; cnt < uiNumOfEmails; cnt++)
	{
		LoadEMailFromFile(File);
	}
}


static void SaveEMailIntoFile(HWFILE const File, Email const* const Mail)
{
	BYTE Data[48];

	DataWriter D{Data};
	INJ_U32(D, 0) // was size of subject
	INJ_U16(D, Mail->usOffset)
	INJ_U16(D, Mail->usLength)
	INJ_U8(D, Mail->ubSender)
	INJ_SKIP(D, 3)
	INJ_U32(D, Mail->iDate)
	INJ_SKIP(D, 4)
	INJ_I32(D, Mail->iFirstData)
	INJ_U32(D, Mail->uiSecondData)
	INJ_SKIP(D, 16)
	INJ_BOOL(D, Mail->fRead)
	INJ_SKIP(D, 3)
	Assert(D.getConsumed() == lengthof(Data));

	File->write(Data, sizeof(Data));
}


void SaveEmailToSavedGame(HWFILE const File)
{
	const Email* pEmail;

	// Count the emails
	UINT32 uiNumOfEmails = 0;
	for (pEmail = pEmailList; pEmail != NULL; pEmail = pEmail->Next)
	{
		uiNumOfEmails++;
	}
	File->write(&uiNumOfEmails, sizeof(UINT32));

	for (pEmail = pEmailList; pEmail != NULL; pEmail = pEmail->Next)
	{
		SaveEMailIntoFile(File, pEmail);
	}
}
