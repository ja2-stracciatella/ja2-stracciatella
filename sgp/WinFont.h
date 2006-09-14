#ifndef __WINFONT_
#define __WINFONT_

void InitWinFonts( );
void ShutdownWinFonts( );

INT32 CreateWinFont( INT32 iHeight, INT32 iWidth, INT32 iEscapement,
                     INT32 iWeight, BOOLEAN fItalic,  BOOLEAN fUnderline,  BOOLEAN fStrikeOut, STR16 szFontName, INT32 iCharSet );
void  DeleteWinFont( INT32 iFont );

void SetWinFontBackColor( INT32 iFont, COLORVAL *pColor );
void SetWinFontForeColor( INT32 iFont, COLORVAL *pColor );

void PrintWinFont( UINT32 uiDestBuf, INT32 iFont, INT32 x, INT32 y, wchar_t *pFontString, ...);

INT16 WinFontStringPixLength(const wchar_t *string, INT32 iFont );
INT16 GetWinFontHeight(const wchar_t *string, INT32 iFont );
UINT32	WinFont_mprintf( INT32 iFont, INT32 x, INT32 y, const wchar_t *pFontString, ...);

BOOLEAN DoesWinFontExistOnSystem( STR16 pTypeFaceName, INT32 iCharSet );



#endif
