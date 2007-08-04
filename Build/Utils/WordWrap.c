#include "Font.h"
#include "WordWrap.h"
#include "Render_Dirty.h"
#include "Font_Control.h"
#include "Debug.h"
#include "Video.h"
#include "MemMan.h"
#include "VSurface.h"


static WRAPPED_STRING* AllocWrappedString(const wchar_t* str)
{
	WRAPPED_STRING* ws = MemAlloc(sizeof(*ws) + sizeof(*ws->sString) * (wcslen(str) + 1));
	ws->pNextWrappedString = NULL;
	wcscpy(ws->sString, str);
	return ws;
}


WRAPPED_STRING* LineWrap(UINT32 ulFont, UINT16 usLineWidthPixels, UINT16* pusLineWidthIfWordIsWiderThenWidth, const wchar_t* pString)
{
	WRAPPED_STRING FirstWrappedString;
	wchar_t TempString[1024];
	INT16 usCurIndex;
	INT16 usEndIndex;
	INT16 usDestIndex;
	wchar_t DestString[1024];
	UINT16 usCurrentWidthPixels = 0;
	BOOLEAN fTheStringIsToLong = FALSE;
	INT32 iErrorCount = 0;

	memset(&FirstWrappedString, 0, sizeof(FirstWrappedString));

	if (pusLineWidthIfWordIsWiderThenWidth != NULL)
	{
		*pusLineWidthIfWordIsWiderThenWidth = usLineWidthPixels;
	}

	if (pString == NULL) return FALSE;

	wcslcpy(TempString, pString, lengthof(TempString));

	usCurIndex = 0;
	usEndIndex = 0;
	usDestIndex = 0;
	HVOBJECT Font = GetFontObject(ulFont);

	for (BOOLEAN fDone = FALSE; !fDone;)
	{
		//Kris:
		//This is TEMPORARY!!!  Dave, I've added this to get out of the infinite loop by slowing increasing the
		//line width!
		iErrorCount++;
		if (iErrorCount > 300)
		{
			iErrorCount = 0;
			usLineWidthPixels++;
		}

		BOOLEAN fNewLine = FALSE;

		DestString[usDestIndex] = TempString[usCurIndex];
		if (DestString[usDestIndex] == TEXT_CODE_NEWLINE)
		{
			DestString[usDestIndex] = L'\0';
			TempString[usCurIndex]  = L'\0';
			fNewLine = TRUE;
		}

		//If we are at the end of the string
		if (TempString[usCurIndex] == L'\0')
		{
			//get to next WrappedString structure
			WRAPPED_STRING* pWrappedString = &FirstWrappedString;
			while (pWrappedString->pNextWrappedString != NULL)
			{
				pWrappedString = pWrappedString->pNextWrappedString;
			}
			pWrappedString->pNextWrappedString = AllocWrappedString(DestString);

			return FirstWrappedString.pNextWrappedString;
		}

		usCurrentWidthPixels += GetCharWidth(Font, TempString[usCurIndex]);

		if (usCurrentWidthPixels > usLineWidthPixels)
		{
			//if an error has occured, and the string is too long
			if (fTheStringIsToLong) DestString[usDestIndex] = L' ';

			//Go back to begining of word
			while (DestString[usDestIndex] != L' ' && usCurIndex > 0)
			{
				usCurrentWidthPixels -= GetCharWidth(Font, DestString[usDestIndex]);
				usCurIndex--;
				usDestIndex--;
			}
			usEndIndex = usDestIndex;

			if (usEndIndex < 0) usEndIndex = 0;

			// put next line into temp buffer
			DestString[usEndIndex] = L'\0';

			//get to next WrappedString structure
			WRAPPED_STRING* pWrappedString = &FirstWrappedString;
			while (pWrappedString->pNextWrappedString != NULL)
			{
				pWrappedString = pWrappedString->pNextWrappedString;
			}

			if (wcslen(DestString) != 0)
			{
				pWrappedString->pNextWrappedString = AllocWrappedString(DestString);

				usCurrentWidthPixels = 0;
				usDestIndex = 0;
				usCurIndex++;
				usEndIndex = usCurIndex;

				const wchar_t* pCurrentStringLoc = &TempString[usEndIndex];
				//if last line, put line into string structure
				if (StringPixLength(pCurrentStringLoc, ulFont) < usLineWidthPixels)
				{
					// run until end of DestString
					wcscpy(DestString, pCurrentStringLoc);
					for (INT32 iCounter = 0; DestString[iCounter] != L'\0'; iCounter++)
					{
						if (DestString[iCounter] == TEXT_CODE_NEWLINE)
						{
							DestString[iCounter] = L'\0';
							fNewLine = TRUE;
							break;
						}
					}

					//get to next WrappedString structure
					pWrappedString = &FirstWrappedString;
					while (pWrappedString->pNextWrappedString != NULL)
					{
						pWrappedString = pWrappedString->pNextWrappedString;
					}
					pWrappedString->pNextWrappedString = AllocWrappedString(DestString);

					if (fNewLine)
					{
						pWrappedString = &FirstWrappedString;
						while(pWrappedString->pNextWrappedString != NULL)
						pWrappedString = pWrappedString->pNextWrappedString;
						pWrappedString->pNextWrappedString = AllocWrappedString(L" ");
					}

					fDone = TRUE;
				}
				usCurIndex--;
				usDestIndex = -1;
			}
			else
			{
				char zText[1024];
				sprintf(zText, "LineWrap() Error!  The string ( %ls ) has a word ( %ls ) that is too long to fit into the required width of %d!  Please fix!!", pString, &TempString[usCurIndex], usLineWidthPixels);
				DebugMsg(TOPIC_JA2, DBG_LEVEL_3, zText);

				//error
				usLineWidthPixels = 1 + StringPixLength(&TempString[usCurIndex], ulFont);

				if (pusLineWidthIfWordIsWiderThenWidth != NULL)
				{
					*pusLineWidthIfWordIsWiderThenWidth = usLineWidthPixels;
				}

				fTheStringIsToLong = TRUE;

				usCurIndex--;
				usDestIndex--;
			}
		}
		usCurIndex++;
		usDestIndex++;
	}
	return FirstWrappedString.pNextWrappedString;
}


//
// Pass in, the x,y location for the start of the string,
//					the width of the buffer
//					the gap in between the lines
//

UINT16 DisplayWrappedString(UINT16 usPosX, UINT16 usPosY, UINT16 usWidth, UINT8 ubGap, UINT32 uiFont, UINT8 ubColor, const wchar_t* pString, UINT8 ubBackGroundColor, UINT32 uiFlags)
{
	WRAPPED_STRING *pFirstWrappedString, *pTempWrappedString;
	UINT16	uiCounter=0;
	UINT16	usLineWidthIfWordIsWiderThenWidth=0;
  UINT16	usHeight;

  usHeight = GetFontHeight(uiFont);

	pFirstWrappedString = LineWrap(uiFont, usWidth, &usLineWidthIfWordIsWiderThenWidth, pString);

	//if an error occured and a word was bigger then the width passed in, reset the width
	if( usLineWidthIfWordIsWiderThenWidth != usWidth )
		usWidth = usLineWidthIfWordIsWiderThenWidth;

	while(pFirstWrappedString != NULL)
	{
		DrawTextToScreen(pFirstWrappedString->sString, usPosX, usPosY, usWidth, uiFont, ubColor, ubBackGroundColor, uiFlags);

		pTempWrappedString = pFirstWrappedString;
		pFirstWrappedString = pTempWrappedString->pNextWrappedString;
		MemFree( pTempWrappedString );
		pTempWrappedString = NULL;

		uiCounter++;

		usPosY += usHeight + ubGap;
	}

	return uiCounter * (GetFontHeight(uiFont) + ubGap);
}


static void ShadowText(UINT32 uiDestVSurface, const wchar_t* pString, UINT32 uiFont, UINT16 usPosX, UINT16 usPosY);


// DrawTextToScreen	Parameters:
//			The string,
//			X position
//			Y position
//			The width of the area you are drawing in.  It can be 0 for left justified
//			The font
//			the color you want the font
//			the color of the background
//			do you want to display it using dirty rects, TRUE or FALSE
//			flags for either LEFT_JUSTIFIED, CENTER_JUSTIFIED, RIGHT_JUSTIFIED
void DrawTextToScreen(const wchar_t* pStr, UINT16 usLocX, UINT16 usLocY, UINT16 usWidth, UINT32 ulFont, UINT8 ubColor, UINT8 ubBackGroundColor, UINT32 ulFlags)
{
	UINT16	usPosX, usPosY;
	UINT16	usFontHeight=0;
	UINT16	usStringWidth=0;

	if (ulFlags & DONT_DISPLAY_TEXT) return;

	if (ulFlags & CENTER_JUSTIFIED)
	{
		FindFontCenterCoordinates(usLocX, usLocY, usWidth, GetFontHeight(ulFont), pStr, ulFont, &usPosX, &usPosY);
	}
	else if( ulFlags & RIGHT_JUSTIFIED )
	{
  	FindFontRightCoordinates(usLocX, usLocY, usWidth, GetFontHeight(ulFont), pStr, ulFont, &usPosX, &usPosY);
	}
	else
	{
		usPosX = usLocX;
		usPosY = usLocY;
	}

	SetFont(ulFont);

	SetFontForeground(ubColor);
	SetFontBackground(ubBackGroundColor);

	if( ulFlags & TEXT_SHADOWED )
		ShadowText( FRAME_BUFFER, pStr, ulFont, (UINT16)(usPosX-1), (UINT16)(usPosY-1 ) );

	if (ulFlags & MARK_DIRTY) gprintfdirty(usPosX, usPosY, L"%ls", pStr);
	mprintf(usPosX, usPosY, L"%ls", pStr);

	if( ulFlags & INVALIDATE_TEXT )
	{
		  usFontHeight = GetFontHeight(ulFont);
		  usStringWidth = StringPixLength(pStr, ulFont);

  		InvalidateRegion( usPosX, usPosY, usPosX+usStringWidth, usPosY+usFontHeight );
	}
}


//
// Pass in, the x,y location for the start of the string,
//					the width of the buffer (how many pixels wide for word wrapping)
//					the gap in between the lines
//

UINT16 IanDisplayWrappedString(UINT16 usPosX, UINT16 usPosY, UINT16 usWidth, UINT8 ubGap, UINT32 uiFont, UINT8 ubColor, const wchar_t* pString, UINT8 ubBackGroundColor, UINT32 uiFlags)
{
	UINT32 draw_flags = uiFlags & MARK_DIRTY;
	UINT16	usSourceCounter=0,usDestCounter=0,usWordLengthPixels,usLineLengthPixels=0,usPhraseLengthPixels=0;
	UINT16	usLinesUsed=1,usLocalWidth=usWidth;
	UINT32	uiLocalFont=uiFont;
	UINT16	usJustification = LEFT_JUSTIFIED,usLocalPosX=usPosX;
	UINT8		ubLocalColor = ubColor;
	BOOLEAN fBoldOn=FALSE;

	CHAR16	zLineString[128] = L"",zWordString[64]= L"";

	do
	{
		// each character goes towards building a new word
		if (pString[usSourceCounter] != TEXT_SPACE && pString[usSourceCounter] != 0)
		{
			zWordString[usDestCounter++] = pString[usSourceCounter];
		}
		else
		{
			// we hit a space (or end of record), so this is the END of a word!

			// is this a special CODE?
			if (zWordString[0] >= TEXT_CODE_NEWLINE && zWordString[0] <= TEXT_CODE_DEFCOLOR)
			{
				switch(zWordString[0])
				{
					case TEXT_CODE_CENTER:

						if (usJustification != CENTER_JUSTIFIED)
						{
							usJustification = CENTER_JUSTIFIED;

							// erase this word string we've been building - it was just a code
							memset(zWordString,0,sizeof(zWordString));

							// erase the line string, we're starting from scratch
							memset(zLineString,0,sizeof(zLineString));

							// reset the line length - we're starting from scratch
							usLineLengthPixels = 0;

							// reset dest char counter
							usDestCounter = 0;
						}
						else	// turn OFF centering...
						{

							// shadow control
							if( IAN_WRAP_NO_SHADOW & uiFlags )
							{
		            // turn off shadow
		            SetFontShadow( NO_SHADOW );
							}


							// time to draw this line of text (centered)!
							DrawTextToScreen(zLineString, usLocalPosX, usPosY, usLocalWidth, uiLocalFont, ubLocalColor, ubBackGroundColor, usJustification | draw_flags);

              // shadow control
							if( IAN_WRAP_NO_SHADOW & uiFlags )
							{
		            // turn on shadow
		            SetFontShadow( DEFAULT_SHADOW );
							}

							// increment Y position for next time
  						usPosY += GetFontHeight(uiLocalFont) + ubGap;

							// we just used a line, so note that
							usLinesUsed++;

							// reset x position
							usLocalPosX = usPosX;

							// erase line string
							memset(zLineString,0,sizeof(zLineString));

							// erase word string
							memset(zWordString,0,sizeof(zWordString));

							// reset the line length
							usLineLengthPixels = 0;

							// reset dest char counter
							usDestCounter = 0;

							// turn off centering...
							usJustification = LEFT_JUSTIFIED;
						}

						break;



					case TEXT_CODE_NEWLINE:

						// NEWLINE character!

						// shadow control
						if( IAN_WRAP_NO_SHADOW & uiFlags )
						{
		           // turn off shadow
		           SetFontShadow( NO_SHADOW );
						}

						// Display what we have up to now
						DrawTextToScreen(zLineString, usLocalPosX, usPosY, usLocalWidth, uiLocalFont, ubLocalColor, ubBackGroundColor, usJustification | draw_flags);

            // shadow control
						if( IAN_WRAP_NO_SHADOW & uiFlags )
						{
		           // turn on shadow
		           SetFontShadow( DEFAULT_SHADOW );
						}


						// increment Y position for next time
  					usPosY += GetFontHeight(uiLocalFont) + ubGap;

						// we just used a line, so note that
						usLinesUsed++;

						// reset x position
						usLocalPosX = usPosX;

						// erase line string
						memset(zLineString,0,sizeof(zLineString));

						// erase word string
						memset(zWordString,0,sizeof(zWordString));

						// reset the line length
						usLineLengthPixels = 0;

						// reset width
						usLocalWidth = usWidth;

						// reset dest char counter
						usDestCounter = 0;

						break;


					case TEXT_CODE_BOLD:

						if (!fBoldOn)
						{

							// shadow control
						  if( IAN_WRAP_NO_SHADOW & uiFlags )
							{
		             // turn off shadow
		             SetFontShadow( NO_SHADOW );
							}

							// turn bold ON.... but first, write whatever we have in normal now...
							DrawTextToScreen(zLineString, usLocalPosX, usPosY, usLocalWidth, uiLocalFont, ubColor, ubBackGroundColor, usJustification | draw_flags);

							// shadow control
						  if( IAN_WRAP_NO_SHADOW & uiFlags )
							{
		             // turn on shadow
		             SetFontShadow( DEFAULT_SHADOW );
							}

							// calc length of what we just wrote
							usPhraseLengthPixels = StringPixLength(zLineString, uiLocalFont);

							// calculate new x position for next time
							usLocalPosX += usPhraseLengthPixels;

							// shorten width for next time
							usLocalWidth -= usLineLengthPixels;

							// erase line string
							memset(zLineString,0,sizeof(zLineString));

							// erase word string
							memset(zWordString,0,sizeof(zWordString));

							// turn bold ON
							uiLocalFont = FONT10ARIALBOLD;
							SetFontShadow(NO_SHADOW);
							fBoldOn     = TRUE;

							// reset dest char counter
							usDestCounter = 0;
						}
						else
						{


							// shadow control
						  if( IAN_WRAP_NO_SHADOW & uiFlags )
							{
		             // turn off shadow
		             SetFontShadow( NO_SHADOW );
							}

							// turn bold OFF - write whatever we have in bold now...
							DrawTextToScreen(zLineString, usLocalPosX, usPosY, usLocalWidth, uiLocalFont, ubColor, ubBackGroundColor, usJustification | draw_flags);

							// shadow control
						  if( IAN_WRAP_NO_SHADOW & uiFlags )
							{
		             // turn on shadow
		             SetFontShadow( DEFAULT_SHADOW );
							}

  						// calc length of what we just wrote
							usPhraseLengthPixels = StringPixLength(zLineString, uiLocalFont);

							// calculate new x position for next time
							usLocalPosX += usPhraseLengthPixels;

							// shorten width for next time
							usLocalWidth -= usLineLengthPixels;

							// erase line string
							memset(zLineString,0,sizeof(zLineString));

// new by Ian Nov 30th, 1998

/*
DEF: commented out for Beta.  Nov 30

							// measure length of WordString and if length > 1, then we have a word to deal with
							if (wcslen(zWordString) > 1)
							{
								// need to reduce the usSourceCounter by the true word length (not counting the code char)
								usSourceCounter -= (wcslen(zWordString) - 1);
							}

*/
							// erase word string
							memset(zWordString,0,sizeof(zWordString));

							// turn bold OFF
							uiLocalFont = uiFont;
							fBoldOn     = FALSE;

							// reset dest char counter
							usDestCounter = 0;
						}

						break;




					case TEXT_CODE_NEWCOLOR:


					  // shadow control
						if( IAN_WRAP_NO_SHADOW & uiFlags )
						{
		           // turn off shadow
		           SetFontShadow( NO_SHADOW );
						}

						// change to new color.... but first, write whatever we have in normal now...
						DrawTextToScreen(zLineString, usLocalPosX, usPosY, usLocalWidth, uiLocalFont, ubLocalColor, ubBackGroundColor, usJustification | draw_flags);

						 // shadow control
						if( IAN_WRAP_NO_SHADOW & uiFlags )
						{
		           // turn on shadow
		           SetFontShadow( DEFAULT_SHADOW );
						}


						// the new color value is the next character in the word
						if (zWordString[1] != TEXT_SPACE && zWordString[1] < 256)
							ubLocalColor = (UINT8) zWordString[1];


						ubLocalColor = 184;;

						// calc length of what we just wrote
						usPhraseLengthPixels = StringPixLength(zLineString, uiLocalFont);

						// calculate new x position for next time
						usLocalPosX += usPhraseLengthPixels;

						// shorten width for next time
						usLocalWidth -= usLineLengthPixels;

						// erase line string
						memset(zLineString,0,sizeof(zLineString));

						// erase word string
						memset(zWordString,0,sizeof(zWordString));

						// reset dest char counter
						usDestCounter = 0;
						break;



					case TEXT_CODE_DEFCOLOR:

						// shadow control
						if( IAN_WRAP_NO_SHADOW & uiFlags )
						{
		           // turn off shadow
		           SetFontShadow( NO_SHADOW );
						}

						// turn color back to default - write whatever we have in bold now...
						DrawTextToScreen(zLineString, usLocalPosX, usPosY, usLocalWidth, uiLocalFont, ubLocalColor, ubBackGroundColor, usJustification | draw_flags);

						 // shadow control
						if( IAN_WRAP_NO_SHADOW & uiFlags )
						{
		           // turn on shadow
		           SetFontShadow( DEFAULT_SHADOW );
						}

  					// calc length of what we just wrote
						usPhraseLengthPixels = StringPixLength(zLineString, uiLocalFont);

						// calculate new x position for next time
						usLocalPosX += usPhraseLengthPixels;

						// shorten width for next time
						usLocalWidth -= usLineLengthPixels;

						// erase line string
						memset(zLineString,0,sizeof(zLineString));

						// erase word string
						memset(zWordString,0,sizeof(zWordString));

						// change color back to default color
						ubLocalColor = ubColor;

						// reset dest char counter
						usDestCounter = 0;
						break;


				}		// end of switch of CODES

			}
			else // not a special character
			{
				// terminate the string TEMPORARILY
				zWordString[usDestCounter]   = 0;

				// get the length (in pixels) of this word
				usWordLengthPixels = StringPixLength(zWordString, uiLocalFont);

				// add a space (in case we add another word to it)
				zWordString[usDestCounter++] = 32;

				// RE-terminate the string
				zWordString[usDestCounter]   = 0;

				// can we fit it onto the length of our "line" ?
				if ((usLineLengthPixels + usWordLengthPixels) < usWidth)
				{
					// yes we can fit this word.

					// get the length AGAIN (in pixels with the SPACE) for this word
				  usWordLengthPixels = StringPixLength(zWordString, uiLocalFont);

					// calc new pixel length for the line
					usLineLengthPixels += usWordLengthPixels;

					// reset dest char counter
					usDestCounter = 0;

					// add the word (with the space) to the line
					wcscat(zLineString, zWordString);
				}
				else
				{
					// can't fit this word!

					// shadow control
					if( IAN_WRAP_NO_SHADOW & uiFlags )
					{
		         // turn off shadow
		         SetFontShadow( NO_SHADOW );
					}


					// Display what we have up to now
					DrawTextToScreen(zLineString, usLocalPosX, usPosY, usLocalWidth, uiLocalFont, ubLocalColor, ubBackGroundColor, usJustification | draw_flags);

           // shadow control
					if( IAN_WRAP_NO_SHADOW & uiFlags )
					{
		          // turn off shadow
		          SetFontShadow( DEFAULT_SHADOW );
					}


					// increment Y position for next time
					usPosY += GetFontHeight(uiLocalFont) + ubGap;

					// reset x position
					usLocalPosX = usPosX;

					// we just used a line, so note that
					usLinesUsed++;

					// start off next line string with the word we couldn't fit
					wcscpy(zLineString,zWordString);

  				// remeasure the line length
					usLineLengthPixels = StringPixLength(zLineString, uiLocalFont);

					// reset dest char counter
					usDestCounter = 0;

					// reset width
					usLocalWidth = usWidth;
				}
			}		// end of this word was NOT a special code

		}



	} while (pString[usSourceCounter++] != 0);

	 // shadow control
	if( IAN_WRAP_NO_SHADOW & uiFlags )
	{
		 // turn off shadow
		 SetFontShadow( NO_SHADOW );
	}

	// draw the paragraph
	DrawTextToScreen(zLineString, usLocalPosX, usPosY, usLocalWidth, uiLocalFont, ubLocalColor, ubBackGroundColor, usJustification | draw_flags);


	// shadow control
	if( IAN_WRAP_NO_SHADOW & uiFlags )
	{
		 // turn on shadow
		 SetFontShadow( DEFAULT_SHADOW );
	}

	// return how many Y pixels we used
	return usLinesUsed * (GetFontHeight(uiFont) + ubGap);
}



void CleanOutControlCodesFromString(const wchar_t* pSourceString, wchar_t* pDestString)
{
	INT32		iSourceCounter=0;
	INT32		iDestCounter=0;

	BOOLEAN fRemoveCurrentChar;
	BOOLEAN fRemoveCurrentCharAndNextChar;

	// this procedure will run through a STR16 and strip out all control characters. This is a nessacary as wcscmp and the like tend not to like control chars in thier strings

	fRemoveCurrentChar = FALSE;
	fRemoveCurrentCharAndNextChar = FALSE;

	// while not end of source string,
	while( pSourceString[ iSourceCounter ] != 0 )
	{
		if( pSourceString[ iSourceCounter + 1 ] == 0 )
		{
			fRemoveCurrentCharAndNextChar = FALSE;
			fRemoveCurrentChar = TRUE;
		}
		else
		{
			switch ( pSourceString[iSourceCounter] )
			{
				case TEXT_CODE_CENTER:
				case TEXT_CODE_NEWCOLOR:
				case TEXT_CODE_BOLD:
				case TEXT_CODE_DEFCOLOR:

					if( pSourceString[ iSourceCounter + 1 ] == TEXT_SPACE )
					{
						fRemoveCurrentCharAndNextChar = TRUE;
						fRemoveCurrentChar = FALSE;
					}
					else
					{
						fRemoveCurrentCharAndNextChar = FALSE;
						fRemoveCurrentChar = TRUE;
					}

				break;

				case TEXT_CODE_NEWLINE:
					fRemoveCurrentCharAndNextChar = FALSE;
					fRemoveCurrentChar = TRUE;
				break;

				default:
					fRemoveCurrentCharAndNextChar = FALSE;
					fRemoveCurrentChar = FALSE;
				break;
			}
		}

		if( fRemoveCurrentChar )
		{
			iSourceCounter++;
		}
		else if( fRemoveCurrentCharAndNextChar )
		{
			if( pSourceString[ iSourceCounter + 2 ] != 0 )
				iSourceCounter += 2;
			else
				iSourceCounter ++;
		}
		else
		{
			pDestString[ iDestCounter ] = pSourceString[ iSourceCounter ];

			iDestCounter++;
			iSourceCounter++;
		}

		fRemoveCurrentCharAndNextChar = FALSE;
		fRemoveCurrentChar = FALSE;
	}

	pDestString[ iDestCounter ] = L'\0';
}


// now variant for grabbing height
UINT16 IanWrappedStringHeight(UINT16 usWidth, UINT8 ubGap, UINT32 uiFont, const wchar_t* pString)
{
	UINT16	usSourceCounter=0,usDestCounter=0,usWordLengthPixels,usLineLengthPixels=0;
	UINT16	usLinesUsed=1;
	UINT32	uiLocalFont=uiFont;
	UINT16	usJustification = LEFT_JUSTIFIED;
	BOOLEAN fBoldOn=FALSE;
	CHAR16	zLineString[640] = L"",zWordString[640]= L"";

	// simply a cut and paste operation on Ian Display Wrapped, but will not write string to screen
	// since this all we want to do, everything IanWrapped will do but without displaying string

	do
	{
		// each character goes towards building a new word
		if (pString[usSourceCounter] != TEXT_SPACE && pString[usSourceCounter] != 0)
		{
			zWordString[usDestCounter++] = pString[usSourceCounter];
		}
		else
		{
			// we hit a space (or end of record), so this is the END of a word!

			// is this a special CODE?
			if (zWordString[0] >= TEXT_CODE_NEWLINE && zWordString[0] <= TEXT_CODE_DEFCOLOR)
			{
				switch(zWordString[0])
				{
					case TEXT_CODE_CENTER:

						if (usJustification != CENTER_JUSTIFIED)
						{
							usJustification = CENTER_JUSTIFIED;

							// erase this word string we've been building - it was just a code
							memset(zWordString,0,sizeof(zWordString));

							// erase the line string, we're starting from scratch
							memset(zLineString,0,sizeof(zLineString));

							// reset the line length - we're starting from scratch
							usLineLengthPixels = 0;

							// reset dest char counter
							usDestCounter = 0;
						}
						else	// turn OFF centering...
						{
							// we just used a line, so note that
							usLinesUsed++;

							// erase line string
							memset(zLineString,0,sizeof(zLineString));

							// erase word string
							memset(zWordString,0,sizeof(zWordString));

							// reset the line length
							usLineLengthPixels = 0;

							// reset dest char counter
							usDestCounter = 0;

							// turn off centering...
							usJustification = LEFT_JUSTIFIED;
						}

						break;



					case TEXT_CODE_NEWLINE:

						// NEWLINE character!

						// we just used a line, so note that
						usLinesUsed++;

						// erase line string
						memset(zLineString,0,sizeof(zLineString));

						// erase word string
						memset(zWordString,0,sizeof(zWordString));

						// reset the line length
						usLineLengthPixels = 0;

						// reset dest char counter
						usDestCounter = 0;

						break;


					case TEXT_CODE_BOLD:

						if (!fBoldOn)
						{
							// erase line string
							memset(zLineString,0,sizeof(zLineString));

							// erase word string
							memset(zWordString,0,sizeof(zWordString));

							// turn bold ON
							uiLocalFont = FONT10ARIALBOLD;
							SetFontShadow(NO_SHADOW);
							fBoldOn     = TRUE;

							// reset dest char counter
							usDestCounter = 0;
						}
						else
						{
							// erase line string
							memset(zLineString,0,sizeof(zLineString));

							// erase word string
							memset(zWordString,0,sizeof(zWordString));

							// turn bold OFF
							uiLocalFont = uiFont;
							fBoldOn     = FALSE;

							// reset dest char counter
							usDestCounter = 0;
						}

						break;



					case TEXT_CODE_NEWCOLOR:
						// erase line string
						memset(zLineString,0,sizeof(zLineString));

						// erase word string
						memset(zWordString,0,sizeof(zWordString));

						// reset dest char counter
						usDestCounter = 0;
						break;



					case TEXT_CODE_DEFCOLOR:
						// erase line string
						memset(zLineString,0,sizeof(zLineString));

						// erase word string
						memset(zWordString,0,sizeof(zWordString));

						// reset dest char counter
						usDestCounter = 0;
						break;


				}		// end of switch of CODES

			}
			else // not a special character
			{
				// terminate the string TEMPORARILY
				zWordString[usDestCounter]   = 0;

				// get the length (in pixels) of this word
				usWordLengthPixels = StringPixLength(zWordString, uiLocalFont);

				// add a space (in case we add another word to it)
				zWordString[usDestCounter++] = 32;

				// RE-terminate the string
				zWordString[usDestCounter]   = 0;

				// can we fit it onto the length of our "line" ?
				if ((usLineLengthPixels + usWordLengthPixels) <= usWidth)
				{
					// yes we can fit this word.

					// get the length AGAIN (in pixels with the SPACE) for this word
  				usWordLengthPixels = StringPixLength(zWordString, uiLocalFont);

					// calc new pixel length for the line
					usLineLengthPixels += usWordLengthPixels;

					// reset dest char counter
					usDestCounter = 0;

					// add the word (with the space) to the line
					wcscat(zLineString, zWordString);
				}
				else
				{
					// can't fit this word!

					// we just used a line, so note that
					usLinesUsed++;

					// start off next line string with the word we couldn't fit
					wcscpy(zLineString,zWordString);

					// remeasure the line length
  				usLineLengthPixels = StringPixLength(zLineString, uiLocalFont);

					// reset dest char counter
					usDestCounter = 0;
				}
			}		// end of this word was NOT a special code

		}



	} while (pString[usSourceCounter++] != 0);

	SetFontShadow(DEFAULT_SHADOW);


	// return how many Y pixels we used
	return usLinesUsed * (GetFontHeight(uiFont) + ubGap);
}


// Places a shadow the width an height of the string, to PosX, posY
static void ShadowText(UINT32 uiDestVSurface, const wchar_t* pString, UINT32 uiFont, UINT16 usPosX, UINT16 usPosY)
{
	UINT32 uiLength = StringPixLength( pString, uiFont);
	UINT16 usFontHeight = GetFontHeight(uiFont);

	ShadowVideoSurfaceRect( uiDestVSurface, usPosX, usPosY, usPosX+uiLength+1, usPosY+usFontHeight+1 );
}


void ReduceStringLength(wchar_t* pString, size_t Length, UINT32 uiWidthToFitIn, UINT32 uiFont)
{
	//if the string is wider then the loaction
	if (StringPixLength(pString, uiFont) <= uiWidthToFitIn) return;

	HVOBJECT Font = GetFontObject(uiFont);
	const wchar_t* const Dots = L"...";
	UINT32 RestWidth = uiWidthToFitIn - StringPixLength(Dots, uiFont);

	//loop through and add each character, 1 at a time
	UINT32 i;
	for (i = 0;; i++)
	{
		UINT32 CharWidth = GetCharWidth(Font, pString[i]);
		if (CharWidth > RestWidth) break;
		RestWidth -= CharWidth;
	}

	wcslcpy(pString + i, Dots, Length - i);
}
