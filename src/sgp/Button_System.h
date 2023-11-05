// by Kris Morness (originally created by Bret Rowden)

#pragma once

#include "MouseSystem.h"

#include <string_theory/string>


#define MAX_BUTTONS     400
#define MAX_BUTTON_PICS 256


// Some GUI_BUTTON system defines
#define BUTTON_NO_IMAGE    -1

//effects how the button is rendered.
#define BUTTON_TYPES (BUTTON_QUICK | BUTTON_GENERIC | BUTTON_HOT_SPOT | BUTTON_CHECKBOX)

//button flags
#define BUTTON_TOGGLE           0x00000000
#define BUTTON_QUICK            0x00000000
#define BUTTON_ENABLED          0x00000001
#define BUTTON_CLICKED_ON       0x00000002
#define BUTTON_GENERIC          0x00000020
#define BUTTON_HOT_SPOT         0x00000040
#define BUTTON_SELFDELETE_IMAGE 0x00000080
#define BUTTON_DELETION_PENDING 0x00000100
#define BUTTON_DIRTY            0x00000400
#define BUTTON_CHECKBOX         0x00001000
#define BUTTON_NEWTOGGLE        0x00002000
#define BUTTON_FORCE_UNDIRTY    0x00004000 // no matter what happens this buttons does NOT get marked dirty
#define BUTTON_NO_DUPLICATE     0x80000000 // Exclude button from duplicate check


extern SGPVSurface* ButtonDestBuffer;

struct GUI_BUTTON;

// GUI_BUTTON callback function type
typedef std::function<void(GUI_BUTTON*, UINT32)> GUI_CALLBACK;

GUI_CALLBACK ButtonCallbackPrimarySecondary(
	GUI_CALLBACK primaryAction,
	GUI_CALLBACK secondaryAction,
	GUI_CALLBACK allEvents = nullptr,
	bool triggerPrimaryOnMouseDown = false
);

// GUI_BUTTON structure definitions.
struct GUI_BUTTON
{
	GUI_BUTTON(UINT32 flags, INT16 left, INT16 top, INT16 width, INT16 height, INT8 priority, GUI_CALLBACK click, GUI_CALLBACK move);
	~GUI_BUTTON();

	bool Clicked() const { return uiFlags & BUTTON_CLICKED_ON; }

	bool Enabled() const { return uiFlags & BUTTON_ENABLED; }

	// Set the text that will be displayed as the FastHelp
	void SetFastHelpText(const ST::string& str);

	void Hide();
	void Show();

	// Draw the button on the screen.
	void Draw();

	void SpecifyDownTextColors(UINT32 fore_color_down, UINT32 shadow_color_down);
	void SpecifyHilitedTextColors(UINT32 fore_color_highlighted, UINT32 shadow_color_highlighted);

	enum Justification
	{
		TEXT_LEFT   = -1,
		TEXT_CENTER =  0,
		TEXT_RIGHT  =  1
	};
	void SpecifyTextJustification(Justification);

	void SpecifyText(const ST::string& str);
	void SpecifyGeneralTextAttributes(const ST::string& str, SGPFont font, UINT32 fore_color, UINT32 shadow_color);
	void SpecifyTextOffsets(INT8 text_x_offset, INT8 text_y_offset, BOOLEAN shift_text);
	void SpecifyTextSubOffsets(INT8 text_x_offset, INT8 text_y_offset, BOOLEAN shift_text);
	void SpecifyTextWrappedWidth(INT16 wrapped_width);

	void AllowDisabledFastHelp();

	enum DisabledStyle
	{
		DISABLED_STYLE_NONE,    // for dummy buttons, panels, etc.  Always displays normal state.
		DISABLED_STYLE_DEFAULT, // if button has text then shade, else hatch
		DISABLED_STYLE_HATCHED, // always hatches the disabled button
		DISABLED_STYLE_SHADED   // always shades the disabled button 25% darker
	};
	void SpecifyDisabledStyle(DisabledStyle);

	/* Note:  Text is always on top
	 * If fShiftImage is true, then the image will shift down one pixel and right
	 * one pixel just like the text does.
	 */
	void SpecifyIcon(SGPVObject const* const icon_, UINT16 const usVideoObjectIndex, INT8 const bXOffset, INT8 const bYOffset, BOOLEAN const);

	// will simply set the cursor for the mouse region the button occupies
	void SetCursor(UINT16 const cursor) { Area.ChangeCursor(cursor); }

	// Coordinates where button is on the screen
	INT16 X()            const { return Area.RegionTopLeftX; }
	INT16 Y()            const { return Area.RegionTopLeftY; }
	INT16 W()            const { return Area.RegionBottomRightX - Area.RegionTopLeftX; }
	INT16 H()            const { return Area.RegionBottomRightY - Area.RegionTopLeftY; }
	INT16 BottomRightX() const { return Area.RegionBottomRightX; }
	INT16 BottomRightY() const { return Area.RegionBottomRightY; }

	INT16 MouseX()    const { return Area.MouseXPos; }
	INT16 MouseY()    const { return Area.MouseYPos; }
	INT16 RelativeX() const { return Area.RelativeXPos; }
	INT16 RelativeY() const { return Area.RelativeYPos; }

	INT32 GetUserData() const { return User.Data; }
	void  SetUserData(INT32 const data) { User.Data = data; }

	template<typename T> T* GetUserPtr() const { return static_cast<T*>(User.Ptr); }
	void SetUserPtr(void* const p) { User.Ptr = p; }

	INT32        IDNum;         // ID Number, contains it's own button number
	BUTTON_PICS* image;         // Image to use (see DOCs for details)
	MouseRegion  Area;          // Mouse System's mouse region to use for this button
	GUI_CALLBACK ClickCallback; // Button Callback when button is clicked
	GUI_CALLBACK MoveCallback;  // Button Callback when mouse moved on this region
	UINT32       uiFlags;       // Button state flags etc.( 32-bit )
	UINT32       uiOldFlags;    // Old flags from previous render loop
	union                       // Place holder for user data etc.
	{
		INT32 Data;
		void* Ptr;
	} User;
	INT8         bDisabledStyle; // Button disabled style

	// For buttons with text
	ST::utf32_buffer codepoints;
	SGPFont      usFont;              // font for text
	UINT32       sForeColor;          // text colors if there is text
	UINT32       sShadowColor;
	UINT32       sForeColorDown;      // text colors when button is down (optional)
	UINT32       sShadowColorDown;
	UINT32       sForeColorHilited;   // text colors when button is down (optional)
	UINT32       sShadowColorHilited;
	INT8         bJustification;      // BUTTON_TEXT_LEFT, BUTTON_TEXT_CENTER, BUTTON_TEXT_RIGHT
	INT8         bTextXOffset;
	INT8         bTextYOffset;
	INT8         bTextXSubOffSet;
	INT8         bTextYSubOffSet;
	BOOLEAN      fShiftText;
	INT16        sWrappedWidth;

	// For buttons with icons (don't confuse this with quickbuttons which have up to 5 states)
	const SGPVObject* icon;
	INT16        usIconIndex;
	INT8         bIconXOffset; // -1 means horizontally centered
	INT8         bIconYOffset; // -1 means vertically centered
	BOOLEAN      fShiftImage;  // if true, icon is shifted +1,+1 when button state is down.

	UINT8        ubToggleButtonActivated;

	UINT8        ubSoundSchemeID;
};


extern GUI_BUTTON* ButtonList[MAX_BUTTONS]; // Button System's Main Button List


class GUIButtonRef
{
	public:
		GUIButtonRef() : btn_id_(0) {}

		GUIButtonRef(GUI_BUTTON* const b) : btn_id_(b->IDNum) {}

		void Reset() { btn_id_ = 0; }

		INT32 ID() const { return btn_id_; }

		GUI_BUTTON* operator ->() const { return ButtonList[btn_id_]; }

		operator GUI_BUTTON*() const { return ButtonList[btn_id_]; }

		static GUIButtonRef NoButton() { GUIButtonRef b; b.btn_id_ = -1; return b; };

	private:
		INT32 btn_id_;
};


/* Initializes the GUI button system for use. Must be called before using any
 * other button functions.
 */
void InitButtonSystem(void);

/* Shuts down and cleans up the GUI button system. Must be called before exiting
 * the program.  Button functions should not be used after calling this
 * function.
 */
void ShutdownButtonSystem(void);

// Loads an image file for use as a button icon.
INT16 LoadGenericButtonIcon(const char* filename);

// Removes a button icon graphic from the system
void UnloadGenericButtonIcon(INT16 GenImg);

// Load images for use with QuickButtons.
BUTTON_PICS* LoadButtonImage(const char* filename, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite);
BUTTON_PICS* LoadButtonImage(char const* filename, INT32 off_normal, INT32 on_normal);

/* Uses a previously loaded quick button image for use with QuickButtons.  The
 * function simply duplicates the vobj!
 */
BUTTON_PICS* UseLoadedButtonImage(BUTTON_PICS* LoadedImg, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite);
BUTTON_PICS* UseLoadedButtonImage(BUTTON_PICS* img, INT32 off_normal, INT32 on_normal);

// Removes a QuickButton image from the system.
void UnloadButtonImage(BUTTON_PICS*);

// Enables an already created button.
void EnableButton(GUIButtonRef);

/* Disables a button. The button remains in the system list, and can be
 * reactivated by calling EnableButton.  Diabled buttons will appear "grayed
 * out" on the screen (unless the graphics for such are not available).
 */
void DisableButton(GUIButtonRef);

void EnableButton(GUIButtonRef, bool enable);

/* Removes a button from the system's list. All memory associated with the
 * button is released.
 */
void RemoveButton(GUIButtonRef&);

void HideButton(GUIButtonRef);
void ShowButton(GUIButtonRef);

void RenderButtons(void);

extern BOOLEAN gfRenderHilights;

/* Creates a QuickButton. QuickButtons only have graphics associated with them.
 * They cannot be re-sized, nor can the graphic be changed.  Providing you have
 * allocated your own image, this is a somewhat simplified function.
 */
GUIButtonRef QuickCreateButton(BUTTON_PICS* image, INT16 x, INT16 y, INT16 priority, GUI_CALLBACK click);
GUIButtonRef QuickCreateButtonNoMove(BUTTON_PICS* const image, const INT16 x, const INT16 y, const INT16 priority, const GUI_CALLBACK click);
GUIButtonRef QuickCreateButtonToggle(BUTTON_PICS* image, INT16 x, INT16 y, INT16 priority, GUI_CALLBACK click);

GUIButtonRef QuickCreateButtonImg(const char* gfx, INT32 grayed, INT32 off_normal, INT32 off_hilite, INT32 on_normal, INT32 on_hilite, INT16 x, INT16 y, INT16 priority, GUI_CALLBACK click);
GUIButtonRef QuickCreateButtonImg(const char* gfx, INT32 off_normal, INT32 on_normal, INT16 x, INT16 y, INT16 priority, GUI_CALLBACK click);

GUIButtonRef CreateCheckBoxButton(INT16 x, INT16 y, const char* filename, INT16 Priority, GUI_CALLBACK ClickCallback);

// Creates an Iconic type button.
GUIButtonRef CreateIconButton(INT16 Icon, INT16 IconIndex, INT16 xloc, INT16 yloc, INT16 w, INT16 h, INT16 Priority, GUI_CALLBACK ClickCallback);

/* Creates a button like HotSpot. HotSpots have no graphics associated with
 * them.
 */
GUIButtonRef CreateHotSpot(INT16 xloc, INT16 yloc, INT16 Width, INT16 Height, INT16 Priority, GUI_CALLBACK ClickCallback);

// Creates a generic button with text on it.
GUIButtonRef CreateTextButton(const ST::string& str, SGPFont font, UINT32 sForeColor, UINT32 sShadowColor, INT16 xloc, INT16 yloc, INT16 w, INT16 h, INT16 Priority, GUI_CALLBACK ClickCallback);

GUIButtonRef CreateIconAndTextButton(BUTTON_PICS* Image, const ST::string& str, SGPFont font, UINT32 sForeColor, UINT32 sShadowColor, UINT32 sForeColorDown, UINT32 sShadowColorDown, INT16 xloc, INT16 yloc, INT16 Priority, GUI_CALLBACK ClickCallback);

/* This is technically not a clickable button, but just a label with text. It is
 * implemented as button */
GUIButtonRef CreateLabel(const ST::string& str, SGPFont font, UINT32 forecolor, UINT32 shadowcolor, INT16 x, INT16 y, INT16 w, INT16 h, INT16 priority);

void MarkAButtonDirty(GUIButtonRef); // will mark only selected button dirty
void MarkButtonsDirty(void);// Function to mark buttons dirty ( all will redraw at next RenderButtons )
void UnMarkButtonDirty(GUIButtonRef);  // unmark button
void UnmarkButtonsDirty(void); // unmark ALL the buttoms on the screen dirty
void ForceButtonUnDirty(GUIButtonRef); // forces button undirty no matter the reason, only lasts one frame


struct ButtonDimensions
{
	UINT32 w;
	UINT32 h;
};

const ButtonDimensions* GetDimensionsOfButtonPic(const BUTTON_PICS*);

UINT32 GetGenericButtonFillColor(void);

void ReleaseAnchorMode(void);
