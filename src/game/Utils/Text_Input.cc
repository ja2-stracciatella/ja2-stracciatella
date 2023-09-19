#include "HImage.h"
#include "Input.h"
#include "Font.h"
#include "VObject.h"
#include "VSurface.h"
#include "Video.h"
#include "Debug.h"
#include "Cursors.h"
#include "Text_Input.h"
#include "Timer_Control.h"
#include "Font_Control.h"
#include "Sound_Control.h"
#include "MouseSystem.h"

#include <string_theory/string>
#include <utility>


BOOLEAN gfNoScroll = FALSE;

struct TextInputColors
{
	//internal values that contain all of the colors for the text editing fields.
	SGPFont usFont;
	UINT16 usTextFieldColor;
	UINT8 ubForeColor, ubShadowColor;
	UINT8 ubHiForeColor, ubHiShadowColor, ubHiBackColor;
	//optional -- no bevelling by default
	BOOLEAN	fBevelling;
	UINT16 usBrighterColor, usDarkerColor;
	//optional -- cursor color defaults to black
	UINT16 usCursorColor;
	//optional colors for disabled fields (defaults to 25% darker shading)
	BOOLEAN fUseDisabledAutoShade;
	UINT8	ubDisabledForeColor;
	UINT8	ubDisabledShadowColor;
	UINT16 usDisabledTextFieldColor;
};

static TextInputColors* pColors = NULL;

//Internal nodes for keeping track of the text and user defined fields.
struct TEXTINPUTNODE
{
	InputType      usInputType;
	UINT8          ubID;
	ST::string     str;
	size_t         numCodepoints;
	size_t         maxCodepoints;
	BOOLEAN        fEnabled;
	BOOLEAN        fUserField;
	MOUSE_REGION   region;
	INPUT_CALLBACK InputCallback;
	TEXTINPUTNODE* next;
	TEXTINPUTNODE* prev;
};

//Stack list containing the head nodes of each level.  Only the top level is the active level.
struct STACKTEXTINPUTNODE
{
	TEXTINPUTNODE *head;
	TEXTINPUTNODE *tail;
	TextInputColors *pColors;
	STACKTEXTINPUTNODE* next;
};

static STACKTEXTINPUTNODE* pInputStack = NULL;

//Internal list vars.  active always points to the currently edited field.
static TEXTINPUTNODE* gpTextInputHead = NULL;
static TEXTINPUTNODE* gpTextInputTail = NULL;
static TEXTINPUTNODE* gpActive = NULL;

//Saving current mode
static TEXTINPUTNODE* pSavedHead = NULL;
static TEXTINPUTNODE* pSavedTail = NULL;
static TextInputColors* pSavedColors = NULL;
static UINT16 gusTextInputCursor = CURSOR_IBEAM;


// Zap the four pointers that control text input. This is a separate
// function because the tail pointer was often forgotten before.
static void ZapTextInputPointers()
{
	gpTextInputHead = nullptr;
	gpTextInputTail = nullptr;
	gpActive = nullptr;
	pColors = nullptr;
}


//Saves the current text input mode by pushing it onto our stack, then starts a new
//one.
static void PushTextInputLevel(void)
{
	STACKTEXTINPUTNODE* const pNewLevel = new STACKTEXTINPUTNODE{};
	pNewLevel->head = gpTextInputHead;
	pNewLevel->tail = gpTextInputTail;
	pNewLevel->pColors = pColors;
	pNewLevel->next = pInputStack;
	pInputStack = pNewLevel;
	DisableAllTextFields();
}


//After the currently text input mode is removed, we then restore the previous one
//automatically.  Assert failure in this function will expose cases where you are trigger
//happy with killing non-existant text input modes.
static void PopTextInputLevel(void)
{
	STACKTEXTINPUTNODE *pLevel;
	gpTextInputHead = pInputStack->head;
	gpTextInputTail = pInputStack->tail;
	pColors = pInputStack->pColors;
	pLevel = pInputStack;
	pInputStack = pInputStack->next;
	delete pLevel;
	pLevel = NULL;
	EnableAllTextFields();
}


//flags for determining various editing modes.
static bool gfEditingText = false;
static BOOLEAN gfTextInputMode = FALSE;

void SetEditingStatus(bool bIsEditing)
{
	if (bIsEditing != gfEditingText)
	{
		gfEditingText = bIsEditing;
		if (bIsEditing)
		{
			SDL_StartTextInput();
		}
		else
		{
			SDL_StopTextInput();
		}
	}
}

//values that contain the hiliting positions and the cursor position.
static size_t gubCursorPos = 0;
static size_t gubStartHilite = 0;


//Simply initiates that you wish to begin inputting text.  This should only apply to screen
//initializations that contain fields that edit text.  It also verifies and clears any existing
//fields.  Your input loop must contain the function HandleTextInput and processed if the gfTextInputMode
//flag is set else process your regular input handler.  Note that this doesn't mean you are necessarily typing,
//just that there are text fields in your screen and may be inactive.  The TAB key cycles through your text fields,
//and special fields can be defined which will call a void functionName( UINT16 usFieldNum )
void InitTextInputMode()
{
	if( gpTextInputHead )
	{
		//Instead of killing all of the currently existing text input fields, they will now (Jan16 '97)
		//be pushed onto a stack, and preserved until we are finished with the new mode when they will
		//automatically be re-instated when the new text input mode is killed.
		PushTextInputLevel();
		//KillTextInputMode();
	}
	ZapTextInputPointers();
	pColors = new TextInputColors{};
	gfTextInputMode = TRUE;
	SetEditingStatus(FALSE);
	pColors->fBevelling = FALSE;
	pColors->fUseDisabledAutoShade = TRUE;
	pColors->usCursorColor = 0;
}

//A hybrid version of InitTextInput() which uses a specific scheme.  JA2's editor uses scheme 1, so
//feel free to add new schemes.
void InitTextInputModeWithScheme( UINT8 ubSchemeID )
{
	InitTextInputMode();
	switch( ubSchemeID )
	{
		case DEFAULT_SCHEME:  //yellow boxes with black text, with bluish bevelling
			SetTextInputFont(FONT12POINT1);
			Set16BPPTextFieldColor( Get16BPPColor(FROMRGB(250, 240, 188) ) );
			SetBevelColors( Get16BPPColor(FROMRGB(136, 138, 135)), Get16BPPColor(FROMRGB(24, 61, 81)) );
			SetTextInputRegularColors( FONT_BLACK, FONT_BLACK );
			SetTextInputHilitedColors( FONT_GRAY2, FONT_GRAY2, FONT_METALGRAY );
			break;
	}
}


// Clear any existing fields, and end text input mode.
void KillTextInputMode()
{
	TEXTINPUTNODE* i = gpTextInputHead;
	if (!i) return;
	while (i)
	{
		if (i->maxCodepoints > 0)
		{
			MSYS_RemoveRegion(&i->region);
		}
		delete std::exchange(i, i->next);
	}
	delete pColors;

	if (pInputStack)
	{
		PopTextInputLevel();
		SetActiveField(0);
	}
	else
	{
		gfTextInputMode = FALSE;
		SetEditingStatus(FALSE);
		ZapTextInputPointers();
	}
}


//Kills all levels of text input modes.  When you init a second consecutive text input mode, without
//first removing them, the existing mode will be preserved.  This function removes all of them in one
//call, though doing so "may" reflect poor coding style, though I haven't thought about any really
//just uses for it :(
void KillAllTextInputModes()
{
	while( gpTextInputHead )
		KillTextInputMode();
}


static TEXTINPUTNODE* AllocateTextInputNode(BOOLEAN const start_editing)
{
	TEXTINPUTNODE* const n = new TEXTINPUTNODE{};
	n->fEnabled = TRUE;
	if (!gpTextInputHead)
	{ // First entry, so we start with text input.
		SetEditingStatus(start_editing);
		gpTextInputHead = n;
		gpActive        = n;
		n->ubID         = 0;
	}
	else
	{ // Add to the end of the list.
		TEXTINPUTNODE* const tail = gpTextInputTail;
		tail->next = n;
		n->prev    = tail;
		n->ubID    = tail->ubID + 1;
	}
	gpTextInputTail = n;
	return n;
}


static void MouseClickedInTextRegionCallback(MOUSE_REGION* reg, UINT32 reason);
static void MouseMovedInTextRegionCallback(MOUSE_REGION* reg, UINT32 reason);


/* After calling InitTextInputMode, you want to define one or more text input
 * fields.  The order of calls to this function dictate the TAB order from
 * traversing from one field to the next.  This function adds mouse regions and
 * processes them for you, as well as deleting them when you are done. */
void AddTextInputField(INT16 sLeft, INT16 sTop, INT16 sWidth, INT16 sHeight, INT8 bPriority, const ST::string& str, size_t maxCodepoints, InputType usInputType)
{
	TEXTINPUTNODE* const n = AllocateTextInputNode(TRUE);
	//Setup the information for the node
	n->usInputType = usInputType;	//setup the filter type
	// All 24hourclock inputtypes have 5 codepoints.  01:23
	if (usInputType == INPUTTYPE_24HOURCLOCK) maxCodepoints = 5;
	Assert(maxCodepoints > 0);
	// Allocate and copy the string.
	n->str = str;
	n->numCodepoints = str.to_utf32().size();
	n->maxCodepoints = maxCodepoints;
	Assert(n->numCodepoints <= maxCodepoints);

	// If this is the first field, then hilight it.
	if (gpTextInputHead == n)
	{
		gubStartHilite = 0;
		gubCursorPos   = n->numCodepoints;
	}
	// Setup the region.
	MSYS_DefineRegion(&n->region, sLeft, sTop, sLeft + sWidth, sTop + sHeight, bPriority, gusTextInputCursor, MouseMovedInTextRegionCallback, MouseClickedInTextRegionCallback);
	n->region.SetUserPtr(n);
}


/* This allows you to insert special processing functions and modes that can't
 * be determined here.  An example would be a file dialog where there would be a
 * file list.  This file list would be accessed using the Win95 convention by
 * pressing TAB.  In there, your key presses would be handled differently and by
 * adding a userinput field, you can make this hook into your function to
 * accomplish this.  In a filedialog, alpha characters would be used to jump to
 * the file starting with that letter, and setting the field in the text input
 * field.  Pressing TAB again would place you back in the text input field.
 * All of that stuff would be handled externally, except for the TAB keys. */
void AddUserInputField(INPUT_CALLBACK const userFunction)
{
	TEXTINPUTNODE* const n = AllocateTextInputNode(FALSE);
	n->fUserField    = TRUE;
	n->InputCallback = userFunction;
}


static TEXTINPUTNODE* GetTextInputField(UINT8 const id)
{
	for (TEXTINPUTNODE* i = gpTextInputHead; i; i = i->next)
	{
		if (i->ubID == id) return i;
	}
	return 0;
}

//Returns the gpActive field ID number.  It'll return -1 if no field is active.
INT16 GetActiveFieldID()
{
	if( gpActive )
		return gpActive->ubID;
	return -1;
}

//This is a useful call made from an external user input field.  Using the previous file dialog example, this
//call would be made when the user selected a different filename in the list via clicking or scrolling with
//the arrows, or even using alpha chars to jump to the appropriate filename.
void SetInputFieldString(UINT8 ubField, const ST::string& str)
{
	TEXTINPUTNODE* const curr = GetTextInputField(ubField);
	if (!curr) return;

	if (!str.empty())
	{
		curr->str = str;
		curr->numCodepoints = str.to_utf32().size();
		Assert(curr->numCodepoints <= curr->maxCodepoints);
	}
	else if (!curr->fUserField)
	{
		curr->str.clear();
		curr->numCodepoints = 0;
	}
	else
	{
		SLOGA("Attempting to illegally set text into user field {}", curr->ubID);
	}
}


ST::string GetStringFromField(UINT8 const ubField)
{
	TEXTINPUTNODE const* const n = GetTextInputField(ubField);
	return n ? n->str : ST::string();
}


INT32 GetNumericStrictValueFromField(UINT8 const id)
{
	ST::utf32_buffer codepoints = GetStringFromField(id).to_utf32();
	if (codepoints.size() == 0) return -1; // Blank string, so return -1
	/* Convert the string to a number. This ensures that non-numeric values
	 * automatically return -1. */
	INT32 total = 0;
	for (char32_t c : codepoints)
	{
		if (c < U'0' || U'9' < c) return -1;
		total = total * 10 + (c - U'0');
	}
	return total;
}


//Converts a number to a numeric strict value.  If the number is negative, the
//field will be blank.
void SetInputFieldStringWithNumericStrictValue( UINT8 ubField, INT32 iNumber )
{
	TEXTINPUTNODE* const curr = GetTextInputField(ubField);
	if (!curr) return;

	AssertMsg(!curr->fUserField, ST::format("Attempting to illegally set text into user field {}", curr->ubID));
	if (iNumber < 0) //negative number converts to blank string
	{
		curr->str.clear();
		curr->numCodepoints = 0;
	}
	else
	{
		INT32 iMax = (INT32)pow(10.0, curr->maxCodepoints);
		if (iNumber > iMax) //set string to max value based on number of chars.
			curr->str = ST::format("{}", iMax - 1);
		else	//set string to the number given
			curr->str = ST::format("{}", iNumber);
		curr->numCodepoints = curr->str.to_utf32().size();
	}
}


// Set the active field to the specified ID passed.
void SetActiveField(UINT8 const id)
{
	TEXTINPUTNODE* const n = GetTextInputField(id);
	if (!n)            return;
	if (n == gpActive) return;
	if (!n->fEnabled)  return;

	if (gpActive && gpActive->InputCallback) {
		gpActive->InputCallback(gpActive->ubID, FALSE);
	}

	gpActive = n;
	if (n->maxCodepoints > 0)
	{
		gubStartHilite = 0;
		gubCursorPos   = n->numCodepoints;
		SetEditingStatus(TRUE);
	}
	else
	{
		SetEditingStatus(FALSE);
		if (n->InputCallback) n->InputCallback(n->ubID, TRUE);
	}
}


static void RenderInactiveTextFieldNode(TEXTINPUTNODE const*);


void SelectNextField()
{
	BOOLEAN fDone = FALSE;
	TEXTINPUTNODE *pStart;

	if( !gpActive )
		return;
	if (gpActive->maxCodepoints > 0)
		RenderInactiveTextFieldNode( gpActive );
	else if( gpActive->InputCallback )
		(gpActive->InputCallback)(gpActive->ubID, FALSE );
	pStart = gpActive;
	while( !fDone )
	{
		gpActive = gpActive->next;
		if( !gpActive )
			gpActive = gpTextInputHead;
		if( gpActive->fEnabled )
		{
			fDone = TRUE;
			if (gpActive->maxCodepoints > 0)
			{
				gubStartHilite = 0;
				gubCursorPos = gpActive->numCodepoints;
				SetEditingStatus(TRUE);
			}
			else
			{
				SetEditingStatus(FALSE);
				if( gpActive->InputCallback )
					(gpActive->InputCallback)(gpActive->ubID, TRUE);
			}
		}
		if( gpActive == pStart )
		{
			SetEditingStatus(FALSE);
			return;
		}
	}
}


static void SelectPrevField(void)
{
	BOOLEAN fDone = FALSE;
	TEXTINPUTNODE *pStart;

	if( !gpActive )
		return;
	if (gpActive->maxCodepoints > 0)
		RenderInactiveTextFieldNode( gpActive );
	else if( gpActive->InputCallback )
		(gpActive->InputCallback)(gpActive->ubID, FALSE );
	pStart = gpActive;
	while( !fDone )
	{
		gpActive = gpActive->prev;
		if( !gpActive )
			gpActive = gpTextInputTail;
		if( gpActive->fEnabled )
		{
			fDone = TRUE;
			if (gpActive->maxCodepoints > 0)
			{
				gubStartHilite = 0;
				gubCursorPos = gpActive->numCodepoints;
				SetEditingStatus(TRUE);
			}
			else
			{
				SetEditingStatus(FALSE);
				if( gpActive->InputCallback )
					(gpActive->InputCallback)(gpActive->ubID, TRUE);
			}
		}
		if( gpActive == pStart )
		{
			SetEditingStatus(FALSE);
			return;
		}
	}
}

//These allow you to customize the general color scheme of your text input boxes.  I am assuming that
//under no circumstances would a user want a different color for each field.  It follows the Win95 convention
//that all text input boxes are exactly the same color scheme.  However, these colors can be set at anytime,
//but will effect all of the colors.
void SetTextInputFont(SGPFont const font)
{
	pColors->usFont = font;
}


void Set16BPPTextFieldColor( UINT16 usTextFieldColor )
{
	pColors->usTextFieldColor = usTextFieldColor;
}

void SetTextInputRegularColors( UINT8 ubForeColor, UINT8 ubShadowColor )
{
	pColors->ubForeColor = ubForeColor;
	pColors->ubShadowColor = ubShadowColor;
}

void SetTextInputHilitedColors( UINT8 ubForeColor, UINT8 ubShadowColor, UINT8 ubBackColor )
{
	pColors->ubHiForeColor = ubForeColor;
	pColors->ubHiShadowColor = ubShadowColor;
	pColors->ubHiBackColor = ubBackColor;
}

void SetBevelColors( UINT16 usBrighterColor, UINT16 usDarkerColor )
{
	pColors->fBevelling = TRUE;
	pColors->usBrighterColor = usBrighterColor;
	pColors->usDarkerColor = usDarkerColor;
}

void SetCursorColor( UINT16 usCursorColor )
{
	pColors->usCursorColor = usCursorColor;
}


static void AddChar(char32_t c);
static void DeleteHilitedText(void);
static void HandleRegularInput(char32_t c);
static void RemoveChars(size_t pos, size_t n);


BOOLEAN HandleTextInput(InputAtom const* const a)
{
	gfNoScroll = FALSE;
	// Not in text input mode
	if (!gfTextInputMode) return FALSE;
	// Unless we are psycho typers, we only want to process these key events.
	if (a->usEvent != TEXT_INPUT && a->usEvent != KEY_DOWN && a->usEvent != KEY_REPEAT) return FALSE;
	// Currently in a user field, so return unless TAB is pressed.
	if (!gfEditingText && a->usParam != SDLK_TAB) return FALSE;

	if (a->usEvent == TEXT_INPUT) {
		/* If the key has no character associated, bail out */
		AssertMsg(a->codepoints.size() > 0, "TEXT_INPUT event sent null character");
		DeleteHilitedText();
		for (char32_t c : a->codepoints)
		{
			HandleRegularInput(c);
		}
		return TRUE;
	}

	switch (a->usKeyState)
	{
		case 0:
			switch (a->usParam)
			{
				/* ESC and ENTER must be handled externally, due to the infinite uses
				 * for them. */
				case SDLK_ESCAPE: return FALSE; // ESC is equivalent to cancel

				case SDLK_RETURN: // ENTER is to confirm.
					PlayJA2Sample(REMOVING_TEXT, BTNVOLUME, 1, MIDDLEPAN);
					return FALSE;

				case SDLK_TAB:
					/* Always select the next field, even when a user defined field is
					 * currently selected. The order in which you add your text and user
					 * fields dictates the cycling order when TAB is pressed. */
					SelectNextField();
					return TRUE;

				case SDLK_LEFT:
					gfNoScroll = TRUE;
					if (gubCursorPos != 0) --gubCursorPos;
					gubStartHilite = gubCursorPos;
					return TRUE;

				case SDLK_RIGHT:
					if (gubCursorPos < gpActive->numCodepoints) ++gubCursorPos;
					gubStartHilite = gubCursorPos;
					return TRUE;

				case SDLK_END:
					gubCursorPos   = gpActive->numCodepoints;
					gubStartHilite = gubCursorPos;
					return TRUE;

				case SDLK_HOME:
					gubCursorPos   = 0;
					gubStartHilite = gubCursorPos;
					return TRUE;

				case SDLK_DELETE:
					/* DEL either deletes the selected text, or the character to the right
					 * of the cursor if applicable. */
					if (gubStartHilite != gubCursorPos)
					{
						DeleteHilitedText();
					}
					else if (gubCursorPos < gpActive->numCodepoints)
					{
						RemoveChars(gubCursorPos, 1);
					}
					else
					{
						return TRUE;
					}
					break;

				case SDLK_BACKSPACE:
					/* Delete the selected text, or the character to the left of the
					 * cursor if applicable. */
					if (gubStartHilite != gubCursorPos)
					{
						DeleteHilitedText();
					}
					else if (gubCursorPos > 0)
					{
						gubStartHilite = --gubCursorPos;
						RemoveChars(gubCursorPos, 1);
					}
					else
					{
						return TRUE;
					}
					break;

				default:
					return TRUE;
			}
			break;

		case SHIFT_DOWN:
			switch (a->usParam)
			{
				case SDLK_TAB: // See comment for non-shifted TAB above
					SelectPrevField();
					return TRUE;

				case SDLK_LEFT:
					gfNoScroll = TRUE;
					if (gubCursorPos != 0) --gubCursorPos;
					return TRUE;

				case SDLK_RIGHT:
					if (gubCursorPos < gpActive->numCodepoints) ++gubCursorPos;
					return TRUE;

				case SDLK_END:
					gubCursorPos = gpActive->numCodepoints;
					return TRUE;

				case SDLK_HOME:
					gubCursorPos = 0;
					return TRUE;

				default:
					return TRUE;

			}

		case CTRL_DOWN:
			switch (a->usParam)
			{
#if 0
				case SDLK_c: ExecuteCopyCommand();  return TRUE;
				case SDLK_x: ExecuteCutCommand();   return TRUE;
				case SDLK_v: ExecutePasteCommand(); return TRUE;
#endif

				case SDLK_DELETE:
					// Delete the entire text field, regardless of hilighting.
					gubStartHilite = 0;
					gubCursorPos   = gpActive->numCodepoints;
					DeleteHilitedText();
					break;

				default: return FALSE;
			}
			break;

		default: return FALSE;
	}

	PlayJA2Sample(ENTERING_TEXT, BTNVOLUME, 1, MIDDLEPAN);
	return TRUE;
}


// All input types are handled in this function.
static void HandleRegularInput(char32_t c)
{
	TEXTINPUTNODE const& n = *gpActive;
	switch (n.usInputType)
	{
		case INPUTTYPE_NUMERICSTRICT:
			if (U'0' <= c && c <= U'9') AddChar(c);
			break;

		case INPUTTYPE_FULL_TEXT:
			if (IsPrintableChar(c)) AddChar(c);
			break;

		case INPUTTYPE_DOSFILENAME: // DOS file names
			if ((U'A' <= c && c <= U'Z') ||
					(U'a' <= c && c <= U'z') ||
					/* Cannot begin a new filename with a number */
					(U'0' <= c && c <= U'9' && gubCursorPos != 0) ||
					c == U'_' || c == U'.')
			{
				AddChar(c);
			}
			break;

		case INPUTTYPE_COORDINATE: // coordinates such as a9, z78, etc.
			// First char is an lower case alpha, subsequent chars are numeric
			if (gubCursorPos == 0)
			{
				if (U'a' <= c && c <= U'z')
				{
					AddChar(c);
				}
				else if (U'A' <= c && c <= U'Z')
				{
					AddChar(static_cast<char32_t>(c + 32)); // Convert to lowercase
				}
			}
			else
			{
				if (U'0' <= c && c <= U'9') AddChar(c);
			}
			break;

		case INPUTTYPE_24HOURCLOCK:
			switch (gubCursorPos)
			{
				case 0:
					if (U'0' <= c && c <= U'2') AddChar(c);
					break;

				case 1:
					if (U'0' <= c && c <= U'9')
					{
						if (n.str[0] == '2' && c > U'3') break;
						AddChar(c);
					}
					if (n.str[2] == '\0')
					{
						AddChar(U':');
					}
					else
					{
						gubStartHilite = ++gubCursorPos;
					}
					break;

				case 2:
					if (c == U':')
					{
						AddChar(c);
					}
					else if (U'0' <= c && c <= U'9')
					{
						AddChar(U':');
						AddChar(c);
					}
					break;

				case 3:
					if (U'0' <= c && c <= U'5') AddChar(c);
					break;

				case 4:
					if (U'0' <= c && c <= U'9') AddChar(c);
					break;
			}
			break;
	}
}


static void AddChar(char32_t c)
{
	PlayJA2Sample(ENTERING_TEXT, BTNVOLUME, 1, MIDDLEPAN);
	TEXTINPUTNODE& n   = *gpActive;
	if (n.numCodepoints >= n.maxCodepoints) return;
	// Insert character after cursor
	if (gubCursorPos < n.numCodepoints)
	{
		ST::utf32_buffer codepoints = n.str.to_utf32();
		size_t i = 0;
		n.str.clear();
		while (i < gubCursorPos)
		{
			n.str += codepoints[i++];
		}
		n.str += c;
		while (i < codepoints.size())
		{
			n.str += codepoints[i++];
		}
	}
	else
	{
		n.str += c;
	}
	++n.numCodepoints;
	gubStartHilite = ++gubCursorPos;
}


static void DeleteHilitedText(void)
{
	size_t start = gubStartHilite;
	size_t end   = gubCursorPos;
	if (start == end) return;
	if (start >  end) Swap(start, end);
	gubStartHilite = start;
	gubCursorPos   = start;
	RemoveChars(start, end - start);
}


static void RemoveChars(size_t const pos, size_t const n)
{
	TEXTINPUTNODE& t = *gpActive;
	Assert(pos + n <= t.numCodepoints);
	ST::utf32_buffer codepoints = t.str.to_utf32();
	size_t i = 0;
	t.str.clear();
	while (i < pos)
	{
		t.str += codepoints[i++];
	}
	i += n;
	while (i < codepoints.size())
	{
		t.str += codepoints[i++];
	}
	t.numCodepoints = codepoints.size() - n;
}


static void SetActiveFieldMouse(MOUSE_REGION const* const r)
{
	TEXTINPUTNODE* const n = r->GetUserPtr<TEXTINPUTNODE>();
	if (n == gpActive) return;
	// Deselect the current text edit region if applicable, then set the new one.
	if (gpActive && gpActive->InputCallback) {
		gpActive->InputCallback(gpActive->ubID, FALSE);
	}

	RenderInactiveTextFieldNode(gpActive);
	gpActive = n;
}


static size_t CalculateCursorPos(INT32 const click_x)
{
	SGPFont const font     = pColors->usFont;
	ST::utf32_buffer codepoints = gpActive->str.to_utf32();
	INT32                char_pos = 0;
	size_t               i;
	for (i = 0; codepoints[i] != U'\0'; ++i)
	{
		char_pos += GetCharWidth(font, codepoints[i]);
		if (char_pos >= click_x) break;
	}
	return i;
}


//Internally used to continue highlighting text
static void MouseMovedInTextRegionCallback(MOUSE_REGION* const reg, UINT32 const reason)
{
	if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsMainFingerDown()) return;

	if (reason & MSYS_CALLBACK_REASON_MOVE       ||
			reason & MSYS_CALLBACK_REASON_LOST_MOUSE ||
			reason & MSYS_CALLBACK_REASON_GAIN_MOUSE)
	{
		SetActiveFieldMouse(reg);
		if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
		{
			if (gusMouseYPos < reg->RegionTopLeftY)
			{
				gubCursorPos = 0;
			}
			else if (gusMouseYPos > reg->RegionBottomRightY)
			{
				gubCursorPos = gpActive->numCodepoints;
			}
		}
		else
		{
			gubCursorPos = static_cast<UINT8>(CalculateCursorPos(gusMouseXPos - reg->RegionTopLeftX));
		}
	}
}


//Internally used to calculate where to place the cursor.
static void MouseClickedInTextRegionCallback(MOUSE_REGION* const reg, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		SetActiveFieldMouse(reg);
		//Signifies that we are typing text now.
		SetEditingStatus(TRUE);
		UINT8 const pos = static_cast<UINT8>(CalculateCursorPos(gusMouseXPos - reg->RegionTopLeftX));
		gubCursorPos   = pos;
		gubStartHilite = pos;
	}
}


static void RenderBackgroundField(TEXTINPUTNODE const* const n)
{
	INT16           const  tlx  = n->region.RegionTopLeftX;
	INT16           const  tly  = n->region.RegionTopLeftY;
	INT16           const  brx  = n->region.RegionBottomRightX;
	INT16           const  bry  = n->region.RegionBottomRightY;
	TextInputColors const& clrs = *pColors;

	if (clrs.fBevelling)
	{
		ColorFillVideoSurfaceArea(FRAME_BUFFER,	tlx,     tly,     brx, bry, clrs.usDarkerColor);
		ColorFillVideoSurfaceArea(FRAME_BUFFER,	tlx + 1, tly + 1, brx, bry, clrs.usBrighterColor);
	}

	UINT16 const colour = n->fEnabled || clrs.fUseDisabledAutoShade ?
		clrs.usTextFieldColor :
		clrs.usDisabledTextFieldColor;
	ColorFillVideoSurfaceArea(FRAME_BUFFER,	tlx + 1, tly + 1, brx - 1, bry - 1, colour);

	InvalidateRegion(tlx, tly, brx, bry);
}


/* Required in your screen loop to update the values, as well as blinking the
 * cursor. */
static void RenderActiveTextField(void)
{
	TEXTINPUTNODE const* const n = gpActive;
	if (!n || n->maxCodepoints == 0) return;

	SaveFontSettings();
	RenderBackgroundField(n);

	TextInputColors const& clrs  = *pColors;
	SGPFont         const  font  = clrs.usFont;
	UINT16          const  h     = GetFontHeight(font);
	INT32           const  y     = n->region.RegionTopLeftY + (n->region.RegionBottomRightY - n->region.RegionTopLeftY - h) / 2;
	ST::utf32_buffer codepoints = n->str.to_utf32();
	size_t start = gubStartHilite;
	size_t end = gubCursorPos;
	if (start != end)
	{ // Some or all of the text is hilighted, so we will use a different method.
		// Sort the hilite order.
		if (start > end) Swap(start, end);
		// Traverse the string one character at a time, and draw the highlited part differently.
		UINT32 x = n->region.RegionTopLeftX + 3;
		for (size_t i = 0; i < codepoints.size(); ++i)
		{
			if (start <= i && i < end)
			{ // In highlighted part of text
				SetFontAttributes(font, clrs.ubHiForeColor, clrs.ubHiShadowColor, clrs.ubHiBackColor);
			}
			else
			{ // In regular part of text
				SetFontAttributes(font, clrs.ubForeColor, clrs.ubShadowColor, 0);
			}
			x += MPrintChar(x, y, codepoints[i]);
		}
	}
	else
	{
		SetFontAttributes(font, clrs.ubForeColor, clrs.ubShadowColor, 0);
		MPrint(n->region.RegionTopLeftX + 3, y, codepoints);
	}

	// Draw the blinking ibeam cursor during the on blink period.
	if (gfEditingText && n->maxCodepoints > 0 && GetJA2Clock() % 1000 < TEXT_CURSOR_BLINK_INTERVAL)
	{
		INT32          x = n->region.RegionTopLeftX + 2;
		for (size_t i = 0; i < gubCursorPos; ++i)
		{
			Assert(codepoints[i] != U'\0');
			x += GetCharWidth(font, codepoints[i]);
		}
		ColorFillVideoSurfaceArea(FRAME_BUFFER, x, y, x + 1, y + h, clrs.usCursorColor);
	}

	RestoreFontSettings();
}


void RenderInactiveTextField(UINT8 const id)
{
	TEXTINPUTNODE const* const n = GetTextInputField(id);
	if (!n || n->maxCodepoints == 0) return;
	SaveFontSettings();
	SetFontAttributes(pColors->usFont, pColors->ubForeColor, pColors->ubShadowColor);
	RenderBackgroundField(n);
	UINT16 const offset = (n->region.RegionBottomRightY - n->region.RegionTopLeftY - GetFontHeight(pColors->usFont)) / 2;
	MPrint(n->region.RegionTopLeftX + 3, n->region.RegionTopLeftY + offset, n->str);
	RestoreFontSettings();
}


static void RenderInactiveTextFieldNode(TEXTINPUTNODE const* const n)
{
	if (!n || n->maxCodepoints == 0) return;

	SaveFontSettings();
	TextInputColors const& clrs     = *pColors;
	bool            const  disabled = !n->fEnabled && clrs.fUseDisabledAutoShade;
	if (disabled)
	{ // Use the color scheme specified by the user
		SetFontAttributes(clrs.usFont, clrs.ubDisabledForeColor, clrs.ubDisabledShadowColor);
	}
	else
	{
		SetFontAttributes(clrs.usFont, clrs.ubForeColor, clrs.ubShadowColor);
	}
	RenderBackgroundField(n);
	MOUSE_REGION const& r = n->region;
	UINT16       const  y = r.RegionTopLeftY + (r.RegionBottomRightY - r.RegionTopLeftY - GetFontHeight(clrs.usFont)) / 2;
	MPrint(r.RegionTopLeftX + 3, y, n->str);
	RestoreFontSettings();

	if (disabled)
	{
		FRAME_BUFFER->ShadowRect(r.RegionTopLeftX, r.RegionTopLeftY, r.RegionBottomRightX, r.RegionBottomRightY);
	}
}


// Use when you do a full interface update.
void RenderAllTextFields()
{
	// Render all of the other text input levels first
	for (STACKTEXTINPUTNODE const* stack = pInputStack; stack; stack = stack->next)
	{
		for (TEXTINPUTNODE const* i = stack->head; i; i = i->next)
		{
			RenderInactiveTextFieldNode(i);
		}
	}

	// Render the current text input level
	for (TEXTINPUTNODE const* i = gpTextInputHead; i; i = i->next)
	{
		if (i != gpActive)
		{
			RenderInactiveTextFieldNode(i);
		}
		else
		{
			RenderActiveTextField();
		}
	}
}

void DisableTextField(UINT8 const id)
{
	TEXTINPUTNODE* const n = GetTextInputField(id);
	if (!n)            return;
	if (gpActive == n) SelectNextField();
	if (!n->fEnabled)  return;
	n->region.Disable();
	n->fEnabled = FALSE;
}


void EnableTextFields(UINT8 const first_id, UINT8 const last_id)
{
	for (TEXTINPUTNODE* i = gpTextInputHead; i; i = i->next)
	{
		if (i->ubID < first_id || last_id < i->ubID) continue;
		if (i->fEnabled) continue;
		i->region.Enable();
		i->fEnabled = TRUE;
	}
}


void DisableTextFields(UINT8 const first_id, UINT8 const last_id)
{
	for (TEXTINPUTNODE* i = gpTextInputHead; i; i = i->next)
	{
		if (i->ubID < first_id || last_id < i->ubID) continue;
		if (!i->fEnabled) continue;
		if (gpActive == i) SelectNextField();
		i->region.Disable();
		i->fEnabled = FALSE;
	}
}


void EnableAllTextFields()
{
	for (TEXTINPUTNODE* i = gpTextInputHead; i; i = i->next)
	{
		if (i->fEnabled) continue;
		i->region.Enable();
		i->fEnabled = TRUE;
	}
	if (!gpActive) gpActive = gpTextInputHead;
}


void DisableAllTextFields()
{
	gpActive = 0;
	for (TEXTINPUTNODE* i = gpTextInputHead; i; i = i->next)
	{
		if (!i->fEnabled) continue;
		i->region.Disable();
		i->fEnabled = FALSE;
	}
}


BOOLEAN EditingText()
{
	return gfEditingText;
}

BOOLEAN TextInputMode()
{
	return gfTextInputMode;
}


//Saves the current text input mode, then removes it and activates the previous text input mode,
//if applicable.  The second function restores the settings.  Doesn't currently support nested
//calls.
void SaveAndRemoveCurrentTextInputMode()
{
	Assert(!pSavedHead); // Attempting to save text input stack head, when one already exists?
	pSavedHead = gpTextInputHead;
	pSavedColors = pColors;
	if( pInputStack )
	{
		gpTextInputHead = pInputStack->head;
		gpTextInputTail = pInputStack->tail;
		pColors = pInputStack->pColors;
		gpActive = nullptr;
	}
	else
	{
		ZapTextInputPointers();
	}
}

void RestoreSavedTextInputMode()
{
	Assert(pSavedHead); // Attempting to restore saved text input stack head, when one doesn't exist?
	gpTextInputHead = std::exchange(pSavedHead, nullptr);
	gpTextInputTail = std::exchange(pSavedTail, nullptr);
	pColors = std::exchange(pSavedColors, nullptr);
	gpActive = nullptr;
}


void SetTextInputCursor(UINT16 const new_cursor)
{
	gusTextInputCursor = new_cursor;
}


//Utility functions for the INPUTTYPE_24HOURCLOCK input type.
UINT16 GetExclusive24HourTimeValueFromField( UINT8 ubField )
{
	TEXTINPUTNODE const* const curr = GetTextInputField(ubField);
	AssertMsg(curr, ST::format("GetExclusive24HourTimeValueFromField: Invalid field {}", ubField));
	if (!curr) return 0xffff;

	UINT16 usTime;
	if (curr->usInputType != INPUTTYPE_24HOURCLOCK)
		return 0xffff; //illegal!
	//First validate the hours 00-23
	if ((curr->str[0] == '2' && curr->str[1] >= '0' && //20-23
			curr->str[1] <='3') ||
			(curr->str[0] >= '0' && curr->str[0] <= '1' && // 00-19
			curr->str[1] >= '0' && curr->str[1] <= '9'))
	{ //Next, validate the colon, and the minutes 00-59
		if (curr->str[2] == ':' && curr->str[5] == '\0' && //	:
				curr->str[3] >= '0' && curr->str[3] <= '5' && // 0-5
				curr->str[4] >= '0' && curr->str[4] <= '9')   // 0-9
		{
			//Hours
			usTime = ((curr->str[0]-0x30) * 10 + curr->str[1]-0x30) * 60;
			//Minutes
			usTime += (curr->str[3]-0x30) * 10 + curr->str[4]-0x30;
			return usTime;
		}
	}
	// invalid
	return 0xffff;
}

//Utility functions for the INPUTTYPE_24HOURCLOCK input type.
void SetExclusive24HourTimeValue( UINT8 ubField, UINT16 usTime )
{
	//First make sure the time is a valid time.  If not, then use 23:59
	if( usTime == 0xffff )
	{
		SetInputFieldString(ubField, {});
		return;
	}
	usTime = std::min(1439, int(usTime));

	TEXTINPUTNODE* const curr = GetTextInputField(ubField);
	if (!curr) return;

	AssertMsg(!curr->fUserField, ST::format("Attempting to illegally set text into user field {}", curr->ubID));
	curr->str.clear();
	curr->str += static_cast<char>((usTime / 600) + 0x30); //10 hours
	curr->str += static_cast<char>((usTime / 60 % 10) + 0x30); //1 hour
	usTime %= 60;                                  //truncate the hours
	curr->str += ':';
	curr->str += static_cast<char>((usTime / 10) + 0x30); //10 minutes
	curr->str += static_cast<char>((usTime % 10) + 0x30); //1 minute;
}
