#ifndef __TRLE_H
#define __TRLE_H


typedef struct
{
  UINT32 uiOffset;
  UINT32 uiWidth;
  UINT32 uiOffLen;
	INT16	 sOffsetX;
	INT16  sOffsetY;

} TRLEObject;


typedef struct
{
	UINT32           uiHeightEach;
	UINT32           uiTotalElements;
	TRLEObject      *pTRLEObject;
	PTR							 pPixData;
	UINT32					 uiSizePixDataElem;

} TRLEData;


BOOLEAN GetTRLEObjectData( UINT32 uiTotalElements, TRLEObject *pTRLEObject, INT16 ssIndex, UINT32 *pWidth, UINT32 *pOffset, UINT32 *pOffLen, UINT16 *pOffsetX, UINT16 *pOffsetY );

BOOLEAN SetTRLEObjectOffset( UINT32 uiTotalElements, TRLEObject *pTRLEObject, INT16 ssIndex, INT16 sOffsetX, INT16 sOffsetY );

#endif
