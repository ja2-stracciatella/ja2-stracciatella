#include "Font.h"
#include "WordWrap.h"
#include "Render_Dirty.h"
#include "Font_Control.h"
#include "Video.h"
#include "VSurface.h"

#include <string_theory/string>

inline bool IsChineseCharacter(char32_t c)
{
	return c >= 0x4e00;
}

inline bool IsControlCharacter(char32_t c)
{
	return c == TEXT_CODE_NEWLINE
		|| c == TEXT_CODE_BOLD
		|| c == TEXT_CODE_CENTER
		|| c == TEXT_CODE_NEWCOLOR
		|| c == TEXT_CODE_DEFCOLOR;
}

inline void SkipSpace(const char32_t** codepoints)
{
	if (*(*codepoints + 1) == TEXT_SPACE)
	{
		++(*codepoints);
	}
}


WrappedString LineWrap(SGPFont font, UINT16 usLineWidthPixels, const ST::utf32_buffer& codepoints)
{
	size_t const max_w = usLineWidthPixels;

	WrappedString result;

	const char32_t* i = codepoints.data();
	while (*i == U' ') ++i; // Skip leading spaces

	size_t         line_w     = 0;
	const char32_t* line_start = i;
	const char32_t* line_end   = i;
	const char32_t* word_start = i;
	size_t         word_w     = 0;
	for (;; ++i)
	{
		if (*i == U' ')
		{
			line_end = i;
			for (; *i == U' '; ++i)
			{
				line_w += GetCharWidth(font, *i);
			}
			word_start = i;
			word_w     = 0;
		}
		if (*i == U'\0')
		{
			if (line_start != i) // Append last line
			{
				result.emplace_back(line_start, i - line_start);
			}
			return result;
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
			result.emplace_back(line_start, line_end - line_start);
			line_start = word_start;
			line_end   = word_start;
			line_w     = word_w;
		}
		if (*i == U'-')
		{ // Allow separation at hyphens
			line_end   = i + 1;
			word_start = i + 1;
			word_w     = 0;
		}
		if (IsChineseCharacter(*i)) // Chinese character
		{
			word_start = i;
			line_end = i;
			word_w = w;
		}
	}
}


// Pass in, the x,y location for the start of the string,
//					the width of the buffer
//					the gap in between the lines
UINT16 DisplayWrappedString(UINT16 x, UINT16 y, UINT16 w, UINT8 gap, SGPFont font, UINT8 foreground, const ST::utf32_buffer& codepoints, UINT8 background, UINT32 flags)
{
	UINT16       total_h = 0;
	UINT16 const h       = GetFontHeight(font) + gap;
	for (auto const& codepoints : LineWrap(font, w, codepoints))
	{
		DrawTextToScreen(codepoints, x, y, w, font, foreground, background, flags);
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
//			do you want to display it using dirty rects, true or false
//			flags for either LEFT_JUSTIFIED, CENTER_JUSTIFIED, RIGHT_JUSTIFIED
void DrawTextToScreen(const ST::utf32_buffer& codepoints, UINT16 x, UINT16 y, UINT16 max_w, SGPFont font, UINT8 foreground, UINT8 background, UINT32 flags)
{
	if (flags & DONT_DISPLAY_TEXT) return;

	INT16 const w = flags & (CENTER_JUSTIFIED | RIGHT_JUSTIFIED | TEXT_SHADOWED | INVALIDATE_TEXT) ?
		StringPixLength(codepoints, font) : 0;

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
		GDirtyPrint(x, y, codepoints);
	}
	else
	{
		MPrint(x, y, codepoints);
	}

	if (flags & INVALIDATE_TEXT)
	{
		UINT16 const h = GetFontHeight(font);
		InvalidateRegion(x, y, x + w, y + h);
	}
}


static void IanDrawTextToScreen(const ST::string& str, UINT16 x, UINT16 y, UINT16 w, SGPFont font, UINT8 foreground, UINT8 background, UINT32 flags, UINT32 ian_flags)
{
	if (ian_flags & IAN_WRAP_NO_SHADOW) SetFontShadow(NO_SHADOW);
	flags |= ian_flags & MARK_DIRTY;
	DrawTextToScreen(CleanOutControlCodesFromString(str), x, y, w, font, foreground, background, flags);
	if (ian_flags & IAN_WRAP_NO_SHADOW) SetFontShadow(DEFAULT_SHADOW);
}


// Pass in, the x,y location for the start of the string,
//					the width of the buffer (how many pixels wide for word wrapping)
//					the gap in between the lines
UINT16 IanDisplayWrappedString(UINT16 sx, UINT16 sy, UINT16 max_w, UINT8 gap, SGPFont font, UINT8 foreground, const ST::utf32_buffer& codepoints, UINT8 background, UINT32 flags)
{
	ST::string line_buf;
	const char32_t* i = codepoints.c_str();
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
		switch (*i)
		{
			case TEXT_CODE_CENTER:
				if (justification != CENTER_JUSTIFIED)
				{
					justification = CENTER_JUSTIFIED;
				}
				else	// turn OFF centering...
				{
					// time to draw this line of text (centered)!
					IanDrawTextToScreen(line_buf, x, y, cur_max_w, cur_font, cur_foreground, background, justification, flags);

					x  = sx;
					y += h;

					// turn off centering...
					justification = LEFT_JUSTIFIED;
				}

				// reset the line
				line_buf.clear();
				line_w   = 0;

				SkipSpace(&i);
				break;

			case TEXT_CODE_NEWLINE:
				// Display what we have up to now
				IanDrawTextToScreen(line_buf, x, y, cur_max_w, cur_font, cur_foreground, background, justification, flags);

				x  = sx;
				y += h;

				// reset the line
				line_buf.clear();
				line_w   = 0;

				// reset width
				cur_max_w = max_w;

				SkipSpace(&i);
				break;

			case TEXT_CODE_BOLD:
				IanDrawTextToScreen(line_buf, x, y, cur_max_w, cur_font, foreground, background, justification, flags);
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
				line_buf.clear();

				SkipSpace(&i);
				break;

			case TEXT_CODE_NEWCOLOR:
				// change to new color.... but first, write whatever we have in normal now...
				IanDrawTextToScreen(line_buf, x, y, cur_max_w, cur_font, cur_foreground, background, justification, flags);
				// calculate new x position for next time
				x += StringPixLength(line_buf, cur_font);

				// the new color value is the next character in the word
				++i;
				if (*i != TEXT_SPACE && *i < 256)
					cur_foreground = (UINT8)*i;

				cur_foreground = 184;

				// shorten width for next time
				cur_max_w -= line_w;

				// erase line string
				line_buf.clear();

				SkipSpace(&i);
				break;

			case TEXT_CODE_DEFCOLOR:
				// turn color back to default - write whatever we have in bold now...
				IanDrawTextToScreen(line_buf, x, y, cur_max_w, cur_font, cur_foreground, background, justification, flags);
				// calculate new x position for next time
				x += StringPixLength(line_buf, cur_font);

				// shorten width for next time
				cur_max_w -= line_w;

				// erase line string
				line_buf.clear();

				// change color back to default color
				cur_foreground = foreground;

				SkipSpace(&i);
				break;

			default: // not a special character
				// get the length (in pixels) of this word
				UINT16 word_w = 0;
				// each character goes towards building a new word
				const char32_t* word_start = i;

				do
				{
					word_w += GetCharWidth(cur_font, *i);
					i++;
				}
				while (*i != TEXT_SPACE
						&& !IsChineseCharacter(*i)
						&& !IsControlCharacter(*i)
						&& *i != U'\0');

				// can we fit it onto the length of our "line"?
				if (line_w + word_w > max_w)
				{ // can't fit this word!
					// Display what we have up to now
					IanDrawTextToScreen(line_buf, x, y, cur_max_w, cur_font, cur_foreground, background, justification, flags);

					x  = sx;
					y += h;

					// start off next line string with the word we couldn't fit
					line_buf.clear();
					line_w   = 0;

					// reset width
					cur_max_w = max_w;
				}

				// calc new pixel length for the line
				line_w += word_w;

				// add the word (with the space) to the line
				while (word_start != i) line_buf += *word_start++;

				if (IsChineseCharacter(*i) || IsControlCharacter(*i))
				{
					//fallback one char, cause 'i' will increase one char at loop
					--i;
				}
				else if (*i != U'\0')
				{
					line_buf += u' ';
					line_w += GetCharWidth(cur_font, u' ');
				}

				break;
		}
	}
	while (*i != U'\0' && *(++i) != U'\0');

	// draw the paragraph
	IanDrawTextToScreen(line_buf, x, y, cur_max_w, cur_font, cur_foreground, background, justification, flags);
	y += h;

	// return how many Y pixels we used
	return y - sy;
}


/* This procedure will run through a string and strip out all control
 * characters. This is a nessacary as wcscmp and the like tend not to like
 * control chars in their strings */
ST::string CleanOutControlCodesFromString(const ST::utf32_buffer& codepoints)
{
	ST::string buf;
	const char32_t* s = codepoints.c_str();
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

			case U'\0':
				return buf;

			default:
				buf += *s++;
				break;
		}
	}
}


// now variant for grabbing height
UINT16 IanWrappedStringHeight(UINT16 max_w, UINT8 gap, SGPFont font, const ST::utf32_buffer& codepoints)
{
	UINT16  line_w             = 0;
	UINT16  n_lines            = 1;
	SGPFont cur_font           = font;
	UINT16  justification      = LEFT_JUSTIFIED;
	bool    is_bold            = false;

	/* simply a cut and paste operation on Ian Display Wrapped, but will not write
	 * string to screen since this all we want to do, everything IanWrapped will
	 * do but without displaying string */

	const char32_t* i = codepoints.c_str();
	do
	{
		// we hit a space (or end of record), so this is the END of a word!
		switch (*i)
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

				if (*(i + 1) == TEXT_SPACE)
				{
					i++;
				}
				break;

			case TEXT_CODE_NEWLINE:
				// we just used a line, so note that
				++n_lines;

				// reset the line length
				line_w = 0;
				if (*(i + 1) == TEXT_SPACE)
				{
					i++;
				}
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

				if (*(i + 1) == TEXT_SPACE)
				{
					i++;
				}
				break;

			case TEXT_CODE_NEWCOLOR:
			case TEXT_CODE_DEFCOLOR:
				if (*(i + 1) == TEXT_SPACE)
				{
					i++;
				}
				break;

			default:
				// get the length (in pixels) of this word
				UINT16 word_w = 0;

				// each character goes towards building a new word
				do
				{
					word_w += GetCharWidth(cur_font, *i);
					i++;
				} while (*i != TEXT_SPACE
						&& !IsChineseCharacter(*i)
						&& !IsControlCharacter(*i)
						&& *i != U'\0');

				// can we fit it onto the length of our "line"?
				if (line_w + word_w > max_w)
				{ // can't fit this word!

					// we just used a line, so note that
					++n_lines;

					// remeasure the line length
					line_w = 0;
				}

				// calc new pixel length for the line
				line_w += word_w;
				if (IsChineseCharacter(*i) || IsControlCharacter(*i))
				{
					//fallback one char, cause 'i' will increase one char at loop
					--i;
				}
				else if (*i != U'\0')
				{
					line_w += GetCharWidth(cur_font, u' ');
				}
		}
	}
	while (*i != U'\0' && *(++i) != U'\0');

	// return how many Y pixels we used
	return n_lines * (GetFontHeight(font) + gap);
}


ST::string ReduceStringLength(const ST::utf32_buffer& codepoints, UINT32 widthToFitIn, SGPFont font)
{
	if (static_cast<UINT32>(StringPixLength(codepoints, font)) <= widthToFitIn) return codepoints;

	const char32_t dot = U'.';
	const UINT32 dotWidth = GetCharWidth(font, dot);
	const size_t numDots = 3;
	const UINT32 dotsWidth = dotWidth * numDots;

	ST::string buf;
	UINT32 width = 0;
	for (char32_t c: codepoints)
	{
		UINT32 charWidth = GetCharWidth(font, c);
		if (width + charWidth + dotsWidth > widthToFitIn) break;
		buf += c;
		width += charWidth;
	}
	for (size_t i = 0; i < numDots; ++i)
	{
		if (width + dotWidth > widthToFitIn) break;
		buf += dot;
		width += dotWidth;
	}
	return buf;
}
