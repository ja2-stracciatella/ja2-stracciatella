//**************************************************************************
//
// Filename :	WizShare.h
//
//	Purpose :
//
// Modification history :
//
//		25nov96:HJH				- creation
//
//**************************************************************************

#ifndef _WizShare_h
#define _WizShare_h

//**************************************************************************
//
//				Includes
//
//**************************************************************************

#include "Types.h"

//**************************************************************************
//
//				Defines
//
//**************************************************************************

#define MAX_MSG_LENGTH 128
#define NUM_MESSAGES   100

//**************************************************************************
//
//				Typedefs
//
//**************************************************************************

#pragma pack(push, 1)

typedef struct WizSharedtag
{
	BOOLEAN	fMessage;

	INT32		iMessageIndex;		// index to 1st message
	INT32		iNumMessages;		// # messages
	INT32		iLastIndex;

	CHAR		cMessages[NUM_MESSAGES][MAX_MSG_LENGTH];
} WizShared;

#pragma pack(pop)

#endif
