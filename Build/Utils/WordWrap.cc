#include "Font.h"
#include "WordWrap.h"
#include "Render_Dirty.h"
#include "Font_Control.h"
#include "Video.h"
#include "MemMan.h"
#include "VSurface.h"


static WRAPPED_STRING* AllocWrappedString(const wchar_t* start, const wchar_t* end)
{
	WRAPPED_STRING* const ws = MALLOCE(WRAPPED_STRING, sizeof(*ws->sString) * (end - start + 1));
	ws->pNextWrappedString = NULL;
	wchar_t* d = ws->sString;
	for (wchar_t const* i = start; i != end; i++) *d++ = *i;
	*d = L'\0';
	return ws;
}


WRAPPED_STRING* LineWrap(Font const font, UINT16 usLineWidthPixels, UINT16* const pusLineWidthIfWordIsWiderThenWidth, wchar_t const* const pString)
{
	if (pusLineWidthIfWordIsWiderThenWidth)
	{
		*pusLineWidthIfWordIsWiderThenWidth = usLineWidthPixels;
	}

	size_t const max_w = usLineWidthPixels;

	WRAPPED_STRING*  head   = 0;
	WRAPPED_STRING** anchor = &head;

	wchar_t const* i = pString;
	while (*i == L' ') ++i; // Skip leading spaces

	size_t         line_w     = 0;
	wchar_t const* line_start = i;
	wchar_t const* line_end   = i;
	wchar_t const* word_start = i;
	size_t         word_w     = 0;
	for (;; ++i)
	{
		if (*i == L' ')
		{
			line_end = i;
			for (; *i == L' '; ++i)
			{
				line_w += GetCharWidth(font, *i);
			}
			word_start = i;
			word_w     = 0;
		}
		if (*i == L'\0')
		{
			if (line_start != i) // Append last line
			{
				WRAPPED_STRING* const ws = AllocWrappedString(line_start, i);
				*anchor = ws;
			}
			return head;
		}
		size_t const w = GetCharWidth(font, *i);
		word_w += w;
		line_w += w;
		if (line_w > max_w)
		{
			if (line_start == line_end)
			{ // A single word is longer than a line. Split the word.
				line_end   = i;
				word_start = i;
				word_w     = 0;
			}
			WRAPPED_STRING* const ws = AllocWrappedString(line_start, line_end);
			*anchor    = ws;
			anchor     = &ws->pNextWrappedString;
			line_start = word_start;
			line_end   = word_start;
			line_w     = word_w;
		}
		if (*i == '-')
		{ // Allow separation at hyphens
			line_end   = i + 1;
			word_start = i + 1;
			word_w     = 0;
		}
	}
}


// Pass in, the x,y location for the start of the string,
//					the width of the buffer
//					the gap in between the lines
UINT16 DisplayWrappedString(UINT16 const x, UINT16 y, UINT16 w, UINT8 const gap, Font const font, UINT8 const foreground, const wchar_t* const string, UINT8 const background, UINT32 const flags)
{
	UINT16 usLineWidthIfWordIsWiderThenWidth;
	WRAPPED_STRING* i = LineWrap(font, w, &usLineWidthIfWordIsWiderThenWidth, string);
	// if an error occured and a word was bigger then the width passed in, reset the width
	w = usLineWidthIfWordIsWiderThenWidth;

	UINT16       total_h = 0;
  UINT16 const h       = GetFontHeight(font) + gap;
	while (i)
	{
		DrawTextToScreen(i->sString, x, y, w, font, foreground, background, flags);
		WRAPPED_STRING* const del = i;
		i = i->pNextWrappedString;
		MemFree(del);
		total_h += h;
		y       += h;
	}
	return total_h;
}


static void ShadowText(SGPVSurface* dst, const wchar_t* pString, Font, UINT16 usPosX, UINT16 usPosY);


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
void DrawTextToScreen(const wchar_t* pStr, UINT16 usLocX, UINT16 usLocY, UINT16 usWidth, Font const font, UINT8 ubColor, UINT8 ubBackGroundColor, UINT32 ulFlags)
{
	UINT16	usFontHeight=0;
	UINT16	usStringWidth=0;

	if (ulFlags & DONT_DISPLAY_TEXT) return;

	INT16 usPosX;
	INT16 usPosY;
	if (ulFlags & CENTER_JUSTIFIED)
	{
		FindFontCenterCoordinates(usLocX, usLocY, usWidth, GetFontHeight(font), pStr, font, &usPosX, &usPosY);
	}
	else if( ulFlags & RIGHT_JUSTIFIED )
	{
  	FindFontRightCoordinates(usLocX, usLocY, usWidth, GetFontHeight(font), pStr, font, &usPosX, &usPosY);
	}
	else
	{
		usPosX = usLocX;
		usPosY = usLocY;
	}

	SetFontFgBg(font, ubColor, ubBackGroundColor);

	if( ulFlags & TEXT_SHADOWED )
		ShadowText( FRAME_BUFFER, pStr, font, (UINT16)(usPosX-1), (UINT16)(usPosY-1 ) );

	if (ulFlags & MARK_DIRTY)
	{
		GPrintDirty(usPosX, usPosY, pStr);
	}
	else
	{
		MPrint(usPosX, usPosY, pStr);
	}

	if( ulFlags & INVALIDATE_TEXT )
	{
		usFontHeight  = GetFontHeight(font);
		usStringWidth = StringPixLength(pStr, font);

  		InvalidateRegion( usPosX, usPosY, usPosX+usStringWidth, usPosY+usFontHeight );
	}
}


//
// Pass in, the x,y location for the start of the string,
//					the width of the buffer (how many pixels wide for word wrapping)
//					the gap in between the lines
//

UINT16 IanDisplayWrappedString(UINT16 usPosX, UINT16 usPosY, UINT16 usWidth, UINT8 ubGap, Font const font, UINT8 ubColor, const wchar_t* pString, UINT8 ubBackGroundColor, UINT32 uiFlags)
{
	UINT32 draw_flags = uiFlags & MARK_DIRTY;
	UINT16	usSourceCounter=0,usDestCounter=0,usWordLengthPixels,usLineLengthPixels=0,usPhraseLengthPixels=0;
	UINT16	usLinesUsed=1,usLocalWidth=usWidth;
	Font    uiLocalFont = font;
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
							uiLocalFont = font;
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

						ubLocalColor = 184;

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
	return usLinesUsed * (GetFontHeight(font) + ubGap);
}



void CleanOutControlCodesFromString(const wchar_t* pSourceString, wchar_t* pDestString)
{
	INT32		iSourceCounter=0;
	INT32		iDestCounter=0;

	BOOLEAN fRemoveCurrentChar;
	BOOLEAN fRemoveCurrentCharAndNextChar;

	// this procedure will run through a string and strip out all control characters. This is a nessacary as wcscmp and the like tend not to like control chars in thier strings

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
UINT16 IanWrappedStringHeight(UINT16 usWidth, UINT8 ubGap, Font const font, const wchar_t* pString)
{
	UINT16	usSourceCounter=0,usDestCounter=0,usWordLengthPixels,usLineLengthPixels=0;
	UINT16	usLinesUsed=1;
	Font    uiLocalFont = font;
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
							uiLocalFont = font;
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
	return usLinesUsed * (GetFontHeight(font) + ubGap);
}


// Places a shadow the width an height of the string, to PosX, posY
static void ShadowText(SGPVSurface* const dst, const wchar_t* pString, Font const font, UINT16 usPosX, UINT16 usPosY)
{
	UINT32 const uiLength     = StringPixLength(pString, font);
	UINT16 const usFontHeight = GetFontHeight(font);
	dst->ShadowRect(usPosX, usPosY, usPosX + uiLength + 1, usPosY + usFontHeight + 1);
}


void ReduceStringLength(wchar_t* pString, size_t Length, UINT32 uiWidthToFitIn, Font const font)
{
	//if the string is wider then the loaction
	if (StringPixLength(pString, font) <= uiWidthToFitIn) return;

	const wchar_t* const Dots = L"...";
	UINT32 RestWidth = uiWidthToFitIn - StringPixLength(Dots, font);

	//loop through and add each character, 1 at a time
	UINT32 i;
	for (i = 0;; i++)
	{
		UINT32 CharWidth = GetCharWidth(font, pString[i]);
		if (CharWidth > RestWidth) break;
		RestWidth -= CharWidth;
	}

	wcslcpy(pString + i, Dots, Length - i);
}
