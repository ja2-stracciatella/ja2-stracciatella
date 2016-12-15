#include "sgp/Font.h"
#include "WordWrap.h"
#include "game/TileEngine/Render_Dirty.h"
#include "Font_Control.h"
#include "sgp/Video.h"
#include "sgp/MemMan.h"
#include "sgp/VSurface.h"


static WRAPPED_STRING* AllocWrappedString(const wchar_t* start, const wchar_t* end)
{
	WRAPPED_STRING* const ws = MALLOCE(WRAPPED_STRING, sString, end - start + 1);
	ws->pNextWrappedString = NULL;
	wchar_t* d = ws->sString;
	for (wchar_t const* i = start; i != end; i++) *d++ = *i;
	*d = L'\0';
	return ws;
}


WRAPPED_STRING* LineWrap(SGPFont const font, UINT16 const usLineWidthPixels, wchar_t const* const pString)
{
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
UINT16 DisplayWrappedString(UINT16 const x, UINT16 y, UINT16 w, UINT8 const gap, SGPFont const font, UINT8 const foreground, const wchar_t* const string, UINT8 const background, UINT32 const flags)
{
	UINT16       total_h = 0;
  UINT16 const h       = GetFontHeight(font) + gap;
	for (WRAPPED_STRING* i = LineWrap(font, w, string); i;)
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
void DrawTextToScreen(wchar_t const* const str, UINT16 x, UINT16 const y, UINT16 const max_w, SGPFont const font, UINT8 const foreground, UINT8 const background, UINT32 const flags)
{
	if (flags & DONT_DISPLAY_TEXT) return;

	INT16 const w = flags & (CENTER_JUSTIFIED | RIGHT_JUSTIFIED | TEXT_SHADOWED | INVALIDATE_TEXT) ?
		StringPixLength(str, font) : 0;

	if (flags & CENTER_JUSTIFIED)
	{
		x += (max_w - w) / 2;
	}
	else if (flags & RIGHT_JUSTIFIED)
	{
		x += max_w - w;
	}

	if (flags & TEXT_SHADOWED)
	{
		UINT16 const h = GetFontHeight(font);
		FRAME_BUFFER->ShadowRect(x - 1, y - 1, x + w, y + h);
	}

	SetFont(font);
	SetFontForeground(foreground);
	SetFontBackground(background);
	if (flags & MARK_DIRTY)
	{
		GDirtyPrint(x, y, str);
	}
	else
	{
		MPrint(x, y, str);
	}

	if (flags & INVALIDATE_TEXT)
	{
		UINT16 const h = GetFontHeight(font);
		InvalidateRegion(x, y, x + w, y + h);
	}
}


static void IanDrawTextToScreen(wchar_t const* const str, wchar_t* const end, UINT16 const x, UINT16 const y, UINT16 const w, SGPFont const font, UINT8 const foreground, UINT8 const background, UINT32 flags, UINT32 const ian_flags)
{
	*end = L'\0';
	if (ian_flags & IAN_WRAP_NO_SHADOW) SetFontShadow(NO_SHADOW);
	flags |= ian_flags & MARK_DIRTY;
	DrawTextToScreen(str, x, y, w, font, foreground, background, flags);
	if (ian_flags & IAN_WRAP_NO_SHADOW) SetFontShadow(DEFAULT_SHADOW);
}


// Pass in, the x,y location for the start of the string,
//					the width of the buffer (how many pixels wide for word wrapping)
//					the gap in between the lines
UINT16 IanDisplayWrappedString(UINT16 const sx, UINT16 const sy, UINT16 const max_w, UINT8 const gap, SGPFont const font, UINT8 const foreground, wchar_t const* const str, UINT8 const background, UINT32 const flags)
{
	wchar_t        line_buf[128];
	wchar_t*       line_pos       = line_buf;
	wchar_t const* i              = str;
	UINT16         cur_max_w      = max_w;
	UINT16         line_w         = 0;
	UINT16         x              = sx;
	UINT16         y              = sy;
	SGPFont        cur_font       = font;
	UINT16         h              = GetFontHeight(cur_font) + gap;
	bool           is_bold        = false;
	UINT8          cur_foreground = foreground;
	UINT16         justification  = LEFT_JUSTIFIED;
	do
	{
		// each character goes towards building a new word
		wchar_t const* word_start = i;
		while (*i != TEXT_SPACE && *i != L'\0') ++i;

		// we hit a space (or end of record), so this is the END of a word!
		switch (word_start[0])
		{
			case TEXT_CODE_CENTER:
				if (justification != CENTER_JUSTIFIED)
				{
					justification = CENTER_JUSTIFIED;
				}
				else	// turn OFF centering...
				{
					// time to draw this line of text (centered)!
					IanDrawTextToScreen(line_buf, line_pos, x, y, cur_max_w, cur_font, cur_foreground, background, justification, flags);

					x  = sx;
					y += h;

					// turn off centering...
					justification = LEFT_JUSTIFIED;
				}

				// reset the line
				line_pos = line_buf;
				line_w   = 0;
				break;

			case TEXT_CODE_NEWLINE:
				// Display what we have up to now
				IanDrawTextToScreen(line_buf, line_pos, x, y, cur_max_w, cur_font, cur_foreground, background, justification, flags);

				x  = sx;
				y += h;

				// reset the line
				line_pos = line_buf;
				line_w   = 0;

				// reset width
				cur_max_w = max_w;
				break;

			case TEXT_CODE_BOLD:
				IanDrawTextToScreen(line_buf, line_pos, x, y, cur_max_w, cur_font, foreground, background, justification, flags);
				// calculate new x position for next time
				x += StringPixLength(line_buf, cur_font);

				// shorten width for next time
				cur_max_w -= line_w;

				is_bold = !is_bold;
				if (is_bold)
				{ // turn bold ON
					SetFontShadow(NO_SHADOW);
					cur_font = FONT10ARIALBOLD;
				}
				else
				{ // turn bold OFF
					cur_font = font;
				}
				h = GetFontHeight(cur_font) + gap;

				// erase line string
				line_pos = line_buf;
				break;

			case TEXT_CODE_NEWCOLOR:
				// change to new color.... but first, write whatever we have in normal now...
				IanDrawTextToScreen(line_buf, line_pos, x, y, cur_max_w, cur_font, cur_foreground, background, justification, flags);
				// calculate new x position for next time
				x += StringPixLength(line_buf, cur_font);

				// the new color value is the next character in the word
				if (word_start[1] != TEXT_SPACE && word_start[1] < 256)
					cur_foreground = (UINT8)word_start[1];

				cur_foreground = 184;

				// shorten width for next time
				cur_max_w -= line_w;

				// erase line string
				line_pos = line_buf;
				break;

			case TEXT_CODE_DEFCOLOR:
				// turn color back to default - write whatever we have in bold now...
				IanDrawTextToScreen(line_buf, line_pos, x, y, cur_max_w, cur_font, cur_foreground, background, justification, flags);
				// calculate new x position for next time
				x += StringPixLength(line_buf, cur_font);

				// shorten width for next time
				cur_max_w -= line_w;

				// erase line string
				line_pos = line_buf;

				// change color back to default color
				cur_foreground = foreground;
				break;

			default: // not a special character
				// get the length (in pixels) of this word
				UINT16 word_w = 0;
				for (wchar_t const* k = word_start; k != i; ++k)
				{
					word_w += GetCharWidth(cur_font, *k);
				}

				// can we fit it onto the length of our "line"?
				if (line_w + word_w >= max_w)
				{ // can't fit this word!
					// Display what we have up to now
					IanDrawTextToScreen(line_buf, line_pos, x, y, cur_max_w, cur_font, cur_foreground, background, justification, flags);

					x  = sx;
					y += h;

					// start off next line string with the word we couldn't fit
					line_pos = line_buf;
					line_w   = 0;

					// reset width
					cur_max_w = max_w;
				}

				// add the word (with the space) to the line
				while (word_start != i) *line_pos++ = *word_start++;
				*line_pos++ = L' ';

				// calc new pixel length for the line
				line_w += word_w + GetCharWidth(cur_font, L' ');
				break;
		}
	}
	while (*i++ != '\0');

	// draw the paragraph
	IanDrawTextToScreen(line_buf, line_pos, x, y, cur_max_w, cur_font, cur_foreground, background, justification, flags);
	y += h;

	// return how many Y pixels we used
	return y - sy;
}


/* This procedure will run through a string and strip out all control
 * characters. This is a nessacary as wcscmp and the like tend not to like
 * control chars in their strings */
void CleanOutControlCodesFromString(wchar_t const* const src, wchar_t* const dst)
{
	// while not end of source string,
	wchar_t const* s = src;
	wchar_t*       d = dst;
	for (;;)
	{
		switch (*s)
		{
			case TEXT_CODE_CENTER:
			case TEXT_CODE_NEWCOLOR:
			case TEXT_CODE_BOLD:
			case TEXT_CODE_DEFCOLOR:
				++s;
				if (*s == TEXT_SPACE)
					++s;
				break;

			case TEXT_CODE_NEWLINE:
				++s;
				break;

			default:
				if ((*d++ = *s++) == L'\0')
					return;
				break;
		}
	}
}


// now variant for grabbing height
UINT16 IanWrappedStringHeight(UINT16 const max_w, UINT8 const gap, SGPFont const font, wchar_t const* const str)
{
	UINT16  line_w             = 0;
	UINT16  n_lines            = 1;
	SGPFont cur_font           = font;
	UINT16  justification      = LEFT_JUSTIFIED;
	bool    is_bold            = FALSE;

	/* simply a cut and paste operation on Ian Display Wrapped, but will not write
	 * string to screen since this all we want to do, everything IanWrapped will
	 * do but without displaying string */

	wchar_t const* i = str;
	do
	{
		// each character goes towards building a new word
		wchar_t const* word_start = i;
		while (*i != TEXT_SPACE && *i != L'\0') i++;

		// we hit a space (or end of record), so this is the END of a word!
		switch (word_start[0])
		{
			case TEXT_CODE_CENTER:
				if (justification != CENTER_JUSTIFIED)
				{
					justification = CENTER_JUSTIFIED;
				}
				else	// turn OFF centering...
				{
					// we just used a line, so note that
					++n_lines;

					// turn off centering...
					justification = LEFT_JUSTIFIED;
				}

				// reset the line length
				line_w = 0;
				break;

			case TEXT_CODE_NEWLINE:
				// we just used a line, so note that
				++n_lines;

				// reset the line length
				line_w = 0;
				break;

			case TEXT_CODE_BOLD:
				is_bold = !is_bold;
				if (is_bold)
				{ // turn bold ON
					cur_font = FONT10ARIALBOLD;
				}
				else
				{ // turn bold OFF
					cur_font = font;
				}
				break;

			case TEXT_CODE_NEWCOLOR:
			case TEXT_CODE_DEFCOLOR:
				break;

			default:
				// get the length (in pixels) of this word
				UINT16 word_w = 0;
				for (wchar_t const* k = word_start; k != i; ++k)
				{
					word_w += GetCharWidth(cur_font, *k);
				}

				// can we fit it onto the length of our "line"?
				if (line_w + word_w > max_w)
				{ // can't fit this word!

					// we just used a line, so note that
					++n_lines;

					// remeasure the line length
					line_w = 0;
				}

				// calc new pixel length for the line
				line_w += word_w + GetCharWidth(cur_font, L' ');
		}
	}
	while (*i++ != L'\0');

	// return how many Y pixels we used
	return n_lines * (GetFontHeight(font) + gap);
}


void ReduceStringLength(wchar_t* pString, size_t Length, UINT32 uiWidthToFitIn, SGPFont const font)
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
