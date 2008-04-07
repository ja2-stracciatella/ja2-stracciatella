#include "Debug.h"
#include "HImage.h"
#include "MemMan.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "WCheck.h"


// ******************************************************************************
//
// Video Object SGP Module
//
// Video Objects are used to contain any imagery which requires blitting. The data
// is contained within a Direct Draw surface. Palette information is in both
// a Direct Draw Palette and a 16BPP palette structure for 8->16 BPP Blits.
// Blitting is done via Direct Draw as well as custum blitters. Regions are
// used to define local coordinates within the surface
//
// Second Revision: Dec 10, 1996, Andrew Emmons
//
// *******************************************************************************


#define COMPRESS_TRANSPARENT				0x80
#define COMPRESS_RUN_MASK						0x7F


typedef struct VOBJECT_NODE
{
	HVOBJECT hVObject;
  struct VOBJECT_NODE* next;
#ifdef SGP_VIDEO_DEBUGGING
	char* pName;
	char* pCode;
#endif
} VOBJECT_NODE;

static VOBJECT_NODE* gpVObjectHead = NULL;
static VOBJECT_NODE* gpVObjectTail = NULL;
UINT32 guiVObjectSize = 0;


BOOLEAN InitializeVideoObjectManager(void)
{
	//Shouldn't be calling this if the video object manager already exists.
	//Call shutdown first...
	Assert(gpVObjectHead == NULL);
	Assert(gpVObjectTail == NULL);
	gpVObjectHead = NULL;
	gpVObjectTail = NULL;
	return TRUE;
}


static BOOLEAN InternalDeleteVideoObject(SGPVObject*);


BOOLEAN ShutdownVideoObjectManager(void)
{
	while (gpVObjectHead != NULL)
	{
		VOBJECT_NODE* curr = gpVObjectHead;
		gpVObjectHead = gpVObjectHead->next;
		InternalDeleteVideoObject(curr->hVObject);
#ifdef SGP_VIDEO_DEBUGGING
		if (curr->pName != NULL) MemFree(curr->pName);
		if (curr->pCode != NULL) MemFree(curr->pCode);
#endif
		MemFree(curr);
	}
	gpVObjectHead = NULL;
	gpVObjectTail = NULL;
	guiVObjectSize = 0;
	return TRUE;
}


#ifdef JA2TESTVERSION
static UINT32 CountVideoObjectNodes(void)
{
	UINT32 i = 0;
	for (const VOBJECT_NODE* curr = gpVObjectHead; curr != NULL; curr = curr->next)
	{
		i++;
	}
	return i;
}
#endif


static void AddStandardVideoObject(HVOBJECT hVObject)
{
	if (hVObject == NULL) return;

	VOBJECT_NODE* const Node = MALLOC(VOBJECT_NODE);
	Assert(Node != NULL); // out of memory?
	Node->hVObject = hVObject;

	Node->next = NULL;
#ifdef SGP_VIDEO_DEBUGGING
	Node->pName = NULL;
	Node->pCode = NULL;
#endif

	if (gpVObjectTail == NULL)
	{
		gpVObjectHead = Node;
	}
	else
	{
		gpVObjectTail->next = Node;
	}
	gpVObjectTail = Node;

	guiVObjectSize++;
#ifdef JA2TESTVERSION
	if (CountVideoObjectNodes() != guiVObjectSize)
	{
		guiVObjectSize = guiVObjectSize;
	}
#endif
}


SGPVObject* AddStandardVideoObjectFromHImage(HIMAGE hImage)
{
	if (hImage == NULL)
	{
		DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Invalid hImage pointer given");
		return NULL;
	}

	if (!(hImage->fFlags & IMAGE_TRLECOMPRESSED))
	{
		DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Invalid Image format given.");
		return NULL;
	}

	SGPVObject* const vo = MALLOCZ(SGPVObject);
	CHECKF(vo != NULL);

	ETRLEData TempETRLEData;
	CHECKF(GetETRLEImageData(hImage, &TempETRLEData));

	vo->usNumberOfObjects = TempETRLEData.usNumberOfObjects;
	vo->pETRLEObject      = TempETRLEData.pETRLEObject;
	vo->pPixData          = TempETRLEData.pPixData;
	vo->uiSizePixData     = TempETRLEData.uiSizePixData;
	vo->ubBitDepth        = hImage->ubBitDepth;

	if (hImage->ubBitDepth == 8)
	{
		// create palette
		const SGPPaletteEntry* const src_pal = hImage->pPalette;
		Assert(src_pal != NULL);

		SGPPaletteEntry* const pal = MALLOCN(SGPPaletteEntry, 256);
		memcpy(pal, src_pal, sizeof(*pal) * 256);

		vo->pPaletteEntry = pal;
		vo->p16BPPPalette = Create16BPPPalette(pal);
		vo->pShadeCurrent = vo->p16BPPPalette;
	}

	AddStandardVideoObject(vo);
	return vo;
}


SGPVObject* AddStandardVideoObjectFromFile(const char* const ImageFile)
{
	const HIMAGE hImage = CreateImage(ImageFile, IMAGE_ALLIMAGEDATA);
	if (hImage == NULL)
	{
		DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, String("Invalid Image Filename '%s' given", ImageFile));
		return NULL;
	}

	SGPVObject* const vo = AddStandardVideoObjectFromHImage(hImage);
	DestroyImage(hImage);
	return vo;
}


BOOLEAN DeleteVideoObject(SGPVObject* const vo)
{
	VOBJECT_NODE* prev = NULL;
	VOBJECT_NODE* curr = gpVObjectHead;
	while (curr != NULL)
	{
		if (curr->hVObject == vo)
		{ //Found the node, so detach it and delete it.
			InternalDeleteVideoObject(vo);

			if (curr == gpVObjectHead) gpVObjectHead = gpVObjectHead->next;
			if (curr == gpVObjectTail) gpVObjectTail = prev;
			if (prev != NULL) prev->next = curr->next;

#ifdef SGP_VIDEO_DEBUGGING
			if (curr->pName != NULL) MemFree(curr->pName);
			if (curr->pCode != NULL) MemFree(curr->pCode);
#endif
			MemFree(curr);
			guiVObjectSize--;
#ifdef JA2TESTVERSION
			if (CountVideoObjectNodes() != guiVObjectSize)
			{
				guiVObjectSize = guiVObjectSize;
			}
#endif
			return TRUE;
		}
		prev = curr;
		curr = curr->next;
	}
	return FALSE;
}


BOOLEAN BltVideoObject(SGPVSurface* const dst, const SGPVObject* const src, const UINT16 usRegionIndex, const INT32 iDestX, const INT32 iDestY)
{
	Assert(src->ubBitDepth     ==  8);
	Assert(GetVSurfaceBPP(dst) == 16);

	UINT32 uiPitch;
	UINT16* pBuffer = (UINT16*)LockVideoSurface(dst, &uiPitch);
	if (pBuffer == NULL) return FALSE;

	if (BltIsClipped(src, iDestX, iDestY, usRegionIndex, &ClippingRect))
	{
		Blt8BPPDataTo16BPPBufferTransparentClip(pBuffer, uiPitch, src, iDestX, iDestY, usRegionIndex, &ClippingRect);
	}
	else
	{
		Blt8BPPDataTo16BPPBufferTransparent(pBuffer, uiPitch, src, iDestX, iDestY, usRegionIndex);
	}

	UnLockVideoSurface(dst);
	return TRUE;
}


// Deletes all palettes, surfaces and region data
static BOOLEAN InternalDeleteVideoObject(SGPVObject* const hVObject)
{
	CHECKF(hVObject != NULL);

	DestroyObjectPaletteTables(hVObject);

	if (hVObject->pPaletteEntry != NULL) MemFree(hVObject->pPaletteEntry);
	if (hVObject->pPixData      != NULL) MemFree(hVObject->pPixData);
	if (hVObject->pETRLEObject  != NULL) MemFree(hVObject->pETRLEObject);

	if (hVObject->ppZStripInfo != NULL)
	{
		for (UINT32 usLoop = 0; usLoop < hVObject->usNumberOfObjects; usLoop++)
		{
			if (hVObject->ppZStripInfo[usLoop] != NULL)
			{
				MemFree(hVObject->ppZStripInfo[usLoop]->pbZChange);
				MemFree(hVObject->ppZStripInfo[usLoop]);
			}
		}
		MemFree(hVObject->ppZStripInfo);
	}

	MemFree(hVObject);

	return TRUE;
}


/* Destroys the palette tables of a video object. All memory is deallocated, and
 * the pointers set to NULL. Be careful not to try and blit this object until
 * new tables are calculated, or things WILL go boom. */
BOOLEAN DestroyObjectPaletteTables(HVOBJECT hVObject)
{
	for (UINT32 x = 0; x < HVOBJECT_SHADE_TABLES; x++)
	{
		if (hVObject->fFlags & VOBJECT_FLAG_SHADETABLE_SHARED) continue;

		if (hVObject->pShades[x] != NULL)
		{
			if (hVObject->pShades[x] == hVObject->p16BPPPalette)
			{
				hVObject->p16BPPPalette = NULL;
			}

			MemFree(hVObject->pShades[x]);
			hVObject->pShades[x] = NULL;
		}
	}

	if (hVObject->p16BPPPalette != NULL)
	{
		MemFree(hVObject->p16BPPPalette);
		hVObject->p16BPPPalette = NULL;
	}

	hVObject->pShadeCurrent = NULL;

	return TRUE;
}


UINT16 SetObjectShade(HVOBJECT pObj, UINT32 uiShade)
{
	Assert(pObj != NULL);
	Assert(uiShade >= 0);
	Assert(uiShade < HVOBJECT_SHADE_TABLES);

	if (pObj->pShades[uiShade] == NULL)
	{
		DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_2, "Attempt to set shade level to NULL table");
		return FALSE;
	}

	pObj->pShadeCurrent = pObj->pShades[uiShade];
	return TRUE;
}


/* Given a VOBJECT and ETRLE image index, retrieves the value of the pixel
 * located at the given image coordinates. The value returned is an 8-bit
 * palette index */
BOOLEAN GetETRLEPixelValue(UINT8* pDest, HVOBJECT hVObject, UINT16 usETRLEIndex, UINT16 usX, UINT16 usY)
{
	CHECKF(hVObject != NULL);
	CHECKF(usETRLEIndex < hVObject->usNumberOfObjects);

	const ETRLEObject* pETRLEObject = &hVObject->pETRLEObject[usETRLEIndex];

	CHECKF(usX < pETRLEObject->usWidth);
	CHECKF(usY < pETRLEObject->usHeight);

	// Assuming everything's okay, go ahead and look...
	UINT8* pCurrent = &((UINT8*)hVObject->pPixData)[pETRLEObject->uiDataOffset];

	// Skip past all uninteresting scanlines
	for (UINT16 usLoopY = 0; usLoopY < usY; usLoopY++)
	{
		while (*pCurrent != 0)
		{
			if (*pCurrent & COMPRESS_TRANSPARENT)
			{
				pCurrent++;
			}
			else
			{
				pCurrent += *pCurrent & COMPRESS_RUN_MASK;
			}
		}
	}

	// Now look in this scanline for the appropriate byte
	UINT16 usLoopX = 0;
	do
	{
		UINT16 ubRunLength = *pCurrent & COMPRESS_RUN_MASK;

		if (*pCurrent & COMPRESS_TRANSPARENT)
		{
			if (usLoopX + ubRunLength >= usX)
			{
				*pDest = 0;
				return TRUE;
			}
			else
			{
				pCurrent++;
			}
		}
		else
		{
			if (usLoopX + ubRunLength >= usX)
			{
				// skip to the correct byte; skip at least 1 to get past the byte defining the run
				pCurrent += (usX - usLoopX) + 1;
				*pDest = *pCurrent;
				return TRUE;
			}
			else
			{
				pCurrent += ubRunLength + 1;
			}
		}
		usLoopX += ubRunLength;
	}
	while (usLoopX < usX);
	// huh???
	return FALSE;
}


const ETRLEObject* GetVideoObjectETRLESubregionProperties(const SGPVObject* const vo, const UINT16 usIndex)
{
	CHECKN(usIndex < vo->usNumberOfObjects);
	return &vo->pETRLEObject[usIndex];
}


BOOLEAN BltVideoObjectOutline(SGPVSurface* const dst, const SGPVObject* const hSrcVObject, const UINT16 usIndex, const INT32 iDestX, const INT32 iDestY, const INT16 s16BPPColor)
{
	UINT32 uiPitch;
	UINT16* pBuffer = (UINT16*)LockVideoSurface(dst, &uiPitch);
	if (pBuffer == NULL) return FALSE;

	if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect))
	{
		Blt8BPPDataTo16BPPBufferOutlineClip(pBuffer, uiPitch, hSrcVObject, iDestX, iDestY, usIndex, s16BPPColor, &ClippingRect);
	}
	else
	{
		Blt8BPPDataTo16BPPBufferOutline(pBuffer, uiPitch, hSrcVObject, iDestX, iDestY, usIndex, s16BPPColor);
	}

	UnLockVideoSurface(dst);
	return TRUE;
}


BOOLEAN BltVideoObjectOutlineShadow(SGPVSurface* const dst, const SGPVObject* const src, const UINT16 usIndex, const INT32 iDestX, const INT32 iDestY)
{
	UINT32 uiPitch;
	UINT16* pBuffer = (UINT16*)LockVideoSurface(dst, &uiPitch);
	if (pBuffer == NULL) return FALSE;

	if (BltIsClipped(src, iDestX, iDestY, usIndex, &ClippingRect))
	{
		Blt8BPPDataTo16BPPBufferOutlineShadowClip(pBuffer, uiPitch, src, iDestX, iDestY, usIndex, &ClippingRect);
	}
	else
	{
		Blt8BPPDataTo16BPPBufferOutlineShadow(pBuffer, uiPitch, src, iDestX, iDestY, usIndex);
	}

	UnLockVideoSurface(dst);
	return TRUE;
}


BOOLEAN BltVideoObjectOnce(SGPVSurface* const dst, const char* const filename, const UINT16 region, const INT32 x, const INT32 y)
{
	SGPVObject* const vo = AddVideoObjectFromFile(filename);
	Assert(vo != NO_VOBJECT);
	if (vo == NO_VOBJECT) return FALSE;
	BltVideoObject(dst, vo, region, x, y);
	DeleteVideoObject(vo);
	return TRUE;
}


#ifdef SGP_VIDEO_DEBUGGING

typedef struct DUMPINFO
{
	UINT32 Counter;
	char Name[256];
	char Code[256];
} DUMPINFO;


static void DumpVObjectInfoIntoFile(const char* filename, BOOLEAN fAppend)
{
	if (guiVObjectSize == 0) return;

	FILE* fp = fopen(filename, fAppend ? "a" : "w");
	Assert(fp != NULL);

	//Allocate enough strings and counters for each node.
	DUMPINFO* const Info = MALLOCNZ(DUMPINFO, guiVObjectSize);

	//Loop through the list and record every unique filename and count them
	UINT32 uiUniqueID = 0;
	for (const VOBJECT_NODE* curr = gpVObjectHead; curr != NULL; curr = curr->next)
	{
		const char* Name = curr->pName;
		const char* Code = curr->pCode;
		BOOLEAN fFound = FALSE;
		for (UINT32 i = 0; i < uiUniqueID; i++)
		{
			if (strcasecmp(Name, Info[i].Name) == 0 && strcasecmp(Code, Info[i].Code) == 0)
			{ //same string
				fFound = TRUE;
				Info[i].Counter++;
				break;
			}
		}
		if (!fFound)
		{
			strcpy(Info[uiUniqueID].Name, Name);
			strcpy(Info[uiUniqueID].Code, Code);
			Info[uiUniqueID].Counter++;
			uiUniqueID++;
		}
	}

	//Now dump the info.
	fprintf(fp, "-----------------------------------------------\n");
	fprintf(fp, "%d unique vObject names exist in %d VObjects\n", uiUniqueID, guiVObjectSize);
	fprintf(fp, "-----------------------------------------------\n\n");
	for (UINT32 i = 0; i < uiUniqueID; i++)
	{
		fprintf(fp, "%d occurrences of %s\n%s\n\n", Info[i].Counter, Info[i].Name, Info[i].Code);
	}
	fprintf(fp, "\n-----------------------------------------------\n\n");

	//Free all memory associated with this operation.
	MemFree(Info);
	fclose(fp);
}


//Debug wrapper for adding vObjects
static void RecordVObject(const char* Filename, UINT32 uiLineNum, const char* pSourceFile)
{
	//record the filename of the vObject (some are created via memory though)
	gpVObjectTail->pName = MALLOCN(char, strlen(Filename) + 1);
	strcpy(gpVObjectTail->pName, Filename);

	//record the code location of the calling creating function.
	char str[256];
	sprintf(str, "%s -- line(%d)", pSourceFile, uiLineNum);
	gpVObjectTail->pCode = MALLOCN(char, strlen(str) + 1);
	strcpy(gpVObjectTail->pCode, str);
}


SGPVObject* AddAndRecordVObjectFromHImage(HIMAGE hImage, UINT32 uiLineNum, const char* pSourceFile)
{
	SGPVObject* const vo = AddStandardVideoObjectFromHImage(hImage);
	if (vo != NO_VOBJECT) RecordVObject("<IMAGE>", uiLineNum, pSourceFile);
	return vo;
}


SGPVObject* AddAndRecordVObjectFromFile(const char* ImageFile, UINT32 uiLineNum, const char* pSourceFile)
{
	SGPVObject* const vo = AddStandardVideoObjectFromFile(ImageFile);
	if (vo != NULL) RecordVObject(ImageFile, uiLineNum, pSourceFile);
	return vo;
}


void PerformVideoInfoDumpIntoFile(const char* filename, BOOLEAN fAppend)
{
	DumpVObjectInfoIntoFile(filename, fAppend);
	DumpVSurfaceInfoIntoFile(filename, TRUE);
}

#endif
