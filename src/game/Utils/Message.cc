#include "Buffer.h"
#include "Debug.h"
#include "Directories.h"
#include "Font.h"
#include "LoadSaveData.h"
#include "Types.h"
#include "Font_Control.h"
#include "Message.h"
#include "Timer_Control.h"
#include "Render_Dirty.h"
#include "RenderWorld.h"
#include "SGPFile.h"
#include "Interface.h"
#include "Map_Screen_Interface_Bottom.h"
#include "WordWrap.h"
#include "Sound_Control.h"
#include "SoundMan.h"
#include "Dialogue_Control.h"
#include "Game_Clock.h"
#include "VSurface.h"
#include "JAScreens.h"
#include "ScreenIDs.h"
#include "UILayout.h"

#include <array>
#include <deque>
#include <memory>
#include <string_theory/string>


struct ScrollStringSt
{
	ST::string pString;
	VIDEO_OVERLAY* video_overlay;
	UINT16  usColor;
	BOOLEAN fBeginningOfNewString;
	UINT32  uiTimeOfLastUpdate;

	ScrollStringSt() = default;
	ScrollStringSt(const ST::string& str, UINT16 usColor, BOOLEAN fStartOfNewString)
	: pString(str), video_overlay(nullptr), usColor(usColor), fBeginningOfNewString(fStartOfNewString) {}
};


#define MAX_LINE_COUNT 6
#define X_START 2
#define Y_START (SCREEN_HEIGHT - 150)
#define MAX_AGE 10000
#define LINE_WIDTH 320
#define MAP_LINE_WIDTH 300
#define WIDTH_BETWEEN_NEW_STRINGS 5

#define DEBUG_COLOR FONT_RED
#define DIALOGUE_COLOR FONT_WHITE
#define INTERFACE_COLOR FONT_YELLOW

#define MAP_SCREEN_MESSAGE_FONT TINYFONT1

UINT8 gubStartOfMapScreenMessageList = 0;
static UINT8 gubEndOfMapScreenMessageList = 0;

// index of the current string we are looking at
UINT8 gubCurrentMapMessageString = 0;

// are allowed to beep on message scroll?
BOOLEAN fOkToBeepNewMessage = TRUE;


static std::array<std::shared_ptr<ScrollStringSt>, MAX_LINE_COUNT> gpDisplayList;
static std::array<std::shared_ptr<ScrollStringSt>, 256> gMapScreenMessageList;
static std::deque<std::shared_ptr<ScrollStringSt>> pStringS;

static BOOLEAN fScrollMessagesHidden = FALSE;
static UINT32  uiStartOfPauseTime = 0;

static void RemoveStringVideoOverlay(ScrollStringSt* pStringSt)
{
	// error check, remove one not there
	if (pStringSt->video_overlay == NULL) return;
	RemoveVideoOverlay(pStringSt->video_overlay);
	pStringSt->video_overlay = NULL;
}


static void SetStringVideoOverlayPosition(ScrollStringSt* pStringSt, UINT16 usX, UINT16 usY)
{
	// Donot update if not allocated!
	if (pStringSt->video_overlay == NULL) return;
	SetVideoOverlayPos(pStringSt->video_overlay, usX, usY);
}


static void BlitString(VIDEO_OVERLAY* pBlitter)
{
	if (fScrollMessagesHidden) return;

	SetFontAttributes(pBlitter->uiFontID, pBlitter->ubFontFore, DEFAULT_SHADOW, pBlitter->ubFontBack);
	SGPVSurface::Lock l(pBlitter->uiDestBuff);
	MPrintBuffer(l.Buffer<UINT16>(), l.Pitch(), pBlitter->sX, pBlitter->sY, pBlitter->codepoints);
}


// this function will go through list of display strings and clear them all out
void ClearDisplayedListOfTacticalStrings(void)
{
	for (UINT32 cnt = 0; cnt < MAX_LINE_COUNT; cnt++)
	{
		if (gpDisplayList[cnt] != NULL)
		{
			RemoveStringVideoOverlay(gpDisplayList[cnt].get());
			gpDisplayList[cnt] = NULL;
		}
	}
}


static void PlayNewMessageSound(void);


void ScrollString(void)
{
	// UPDATE TIMER
	UINT32 suiTimer = GetJA2Clock();

	if (guiCurrentScreen == MAP_SCREEN)
	{
		return;
	}

	// Do not update if we are scrolling.
	if (gfScrollPending || g_scroll_inertia) return;

	// messages hidden
	if (fScrollMessagesHidden)
	{
		return;
	}

	INT32 iNumberOfMessagesOnQueue = pStringS.size();
	INT32 iMaxAge = MAX_AGE;

	BOOLEAN fDitchLastMessage = (iNumberOfMessagesOnQueue > 0 && gpDisplayList[MAX_LINE_COUNT - 1] != NULL);

	if (iNumberOfMessagesOnQueue * 1000 >= iMaxAge)
	{
		iNumberOfMessagesOnQueue = iMaxAge / 1000;
	}
	else if (iNumberOfMessagesOnQueue < 0)
	{
		iNumberOfMessagesOnQueue = 0;
	}

	//AGE
	for (UINT32 cnt = 0; cnt < MAX_LINE_COUNT; cnt++)
	{
		if (gpDisplayList[cnt] != NULL)
		{
			if (fDitchLastMessage && cnt == MAX_LINE_COUNT - 1)
			{
				gpDisplayList[cnt]->uiTimeOfLastUpdate = iMaxAge;
			}
				// CHECK IF WE HAVE AGED
			if (suiTimer - gpDisplayList[cnt]->uiTimeOfLastUpdate > (UINT32)(iMaxAge - 1000 * iNumberOfMessagesOnQueue))
			{
				RemoveStringVideoOverlay(gpDisplayList[cnt].get());
				gpDisplayList[cnt] = NULL;
			}
		}
	}


	// CHECK FOR FREE SPOTS AND ADD ANY STRINGS IF WE HAVE SOME TO ADD!

	// FIRST CHECK IF WE HAVE ANY IN OUR QUEUE
	auto& head = pStringS.front();
	if (!head) return;

	// CHECK IF WE HAVE A SLOT!
	// CHECK OUR LAST SLOT!
	if (gpDisplayList[MAX_LINE_COUNT - 1]) return;

	// MOVE ALL UP!
	// cpy, then move
	for (UINT32 cnt = MAX_LINE_COUNT - 1; cnt > 0; cnt--)
	{
		gpDisplayList[cnt] = gpDisplayList[cnt - 1];
	}

	INT32 iNumberOfNewStrings = 0; // the count of new strings, so we can update position by WIDTH_BETWEEN_NEW_STRINGS pixels in the y

	// now add in the new string
	gpDisplayList[0] = head;
	head->video_overlay = RegisterVideoOverlay(BlitString, X_START, Y_START, TINYFONT1, head->usColor, FONT_MCOLOR_BLACK, head->pString);
	if (head->fBeginningOfNewString)
	{
		iNumberOfNewStrings++;
	}

	// set up age
	head->uiTimeOfLastUpdate = GetJA2Clock();

	// now move
	for (UINT32 cnt = 0; cnt <= MAX_LINE_COUNT - 1; cnt++)
	{
		// Adjust position!
		if (gpDisplayList[cnt] != NULL)
		{
			SetStringVideoOverlayPosition(gpDisplayList[cnt].get(), X_START, Y_START - cnt * GetFontHeight(SMALLFONT1) - WIDTH_BETWEEN_NEW_STRINGS * iNumberOfNewStrings);

			// start of new string, increment count of new strings, for spacing purposes
			if (gpDisplayList[cnt]->fBeginningOfNewString)
			{
				iNumberOfNewStrings++;
			}
		}
	}

	// WE NOW HAVE A FREE SPACE, INSERT!

	// Adjust head!
	pStringS.pop_front();

	//check if new meesage we have not seen since mapscreen..if so, beep
	if (fOkToBeepNewMessage &&
			gpDisplayList[MAX_LINE_COUNT - 2] == NULL &&
			(guiCurrentScreen == GAME_SCREEN || guiCurrentScreen == MAP_SCREEN) &&
			!gfFacePanelActive)
	{
		PlayNewMessageSound();
	}
}


void DisableScrollMessages(void)
{
	// will stop the scroll of messages in tactical and hide them during an NPC's dialogue
	// disble video overlay for tatcitcal scroll messages
	EnableDisableScrollStringVideoOverlay(FALSE);
}


void EnableScrollMessages(void)
{
	EnableDisableScrollStringVideoOverlay(TRUE);
}


void HideMessagesDuringNPCDialogue(void)
{
	// will stop the scroll of messages in tactical and hide them during an NPC's dialogue
	fScrollMessagesHidden = TRUE;
	uiStartOfPauseTime = GetJA2Clock();

	for (INT32 cnt = 0; cnt < MAX_LINE_COUNT; cnt++)
	{
		const auto& s = gpDisplayList[cnt];
		if (s != NULL)
		{
			RestoreExternBackgroundRectGivenID(s->video_overlay->background);
			EnableVideoOverlay(FALSE, s->video_overlay);
		}
	}
}


void UnHideMessagesDuringNPCDialogue(void)
{
	fScrollMessagesHidden = FALSE;

	for (INT32 cnt = 0; cnt < MAX_LINE_COUNT; cnt++)
	{
		const auto& s = gpDisplayList[cnt];
		if (s != NULL)
		{
			s->uiTimeOfLastUpdate += GetJA2Clock() - uiStartOfPauseTime;
			EnableVideoOverlay(TRUE, s->video_overlay);
		}
	}
}


static void TacticalScreenMsg(UINT16 usColor, UINT8 ubPriority, const ST::string& str);


// new screen message
void ScreenMsg(UINT16 usColor, UINT8 ubPriority, const ST::string& str)
{
	// pass onto tactical message and mapscreen message
	TacticalScreenMsg(usColor, ubPriority, str);
	MapScreenMessage(usColor, ubPriority, str);

	if (guiCurrentScreen == MAP_SCREEN)
	{
		PlayNewMessageSound( );
	}
	else
	{
		fOkToBeepNewMessage = TRUE;
	}
}


// this function sets up the string into several single line structures
static void TacticalScreenMsg(UINT16 colour, UINT8 const priority, const ST::string& str)
{
	if (IsTimeBeingCompressed()) return;

	switch (priority)
	{
		case MSG_DIALOG:    colour = DIALOGUE_COLOR;  break;
		case MSG_INTERFACE: colour = INTERFACE_COLOR; break;
	}

	BOOLEAN new_string = TRUE;
	for (auto const& codepoints : LineWrap(TINYFONT1, LINE_WIDTH, str))
	{
		pStringS.push_back(std::make_shared<ScrollStringSt>(codepoints, colour, new_string));
		new_string = FALSE;
	}
}


static void AddStringToMapScreenMessageList(const ST::string& pString, UINT16 usColor, BOOLEAN fStartOfNewString);


// this function sets up the string into several single line structures
void MapScreenMessage(UINT16 usColor, UINT8 ubPriority, const ST::string& str)
{
#if defined _DEBUG
	ST::string DestString = str;
#else
	ST::string const& DestString{ str };
#endif

	switch (ubPriority)
	{
		case MSG_UI_FEEDBACK:
			// An imeediate feedback message. Do something else!
			BeginUIMessage(FALSE, DestString);
			return;

		case MSG_SKULL_UI_FEEDBACK:
			BeginUIMessage(TRUE, DestString);
			return;

		case MSG_DEBUG:
#if defined _DEBUG
			DestString = ST::format("Debug: {}", DestString);
			usColor = DEBUG_COLOR;
#else
			return;
#endif

		case MSG_DIALOG:    usColor = DIALOGUE_COLOR;  break;
		case MSG_INTERFACE: usColor = INTERFACE_COLOR; break;
	}

	if (DestString.empty()) return;

	BOOLEAN fNewString = TRUE;
	for (auto const& codepoints : LineWrap(MAP_SCREEN_MESSAGE_FONT, MAP_LINE_WIDTH, DestString))
	{
		AddStringToMapScreenMessageList(codepoints, usColor, fNewString);
		fNewString = FALSE;
	}

	MoveToEndOfMapScreenMessageList();
}


// add string to the map screen message list
static void AddStringToMapScreenMessageList(const ST::string& pString, UINT16 usColor, BOOLEAN fStartOfNewString)
{
	auto pStringSt = std::make_shared<ScrollStringSt>(pString, usColor, fStartOfNewString);

	// Figure out which queue slot index we're going to use to store this
	// If queue isn't full, this is easy, if is is full, we'll re-use the oldest slot
	// Must always keep the wraparound in mind, although this is easy enough with a static, fixed-size queue.

	// always store the new message at the END index, overwriting the previous one
	gMapScreenMessageList[gubEndOfMapScreenMessageList].swap(pStringSt);

	// increment the end
	gubEndOfMapScreenMessageList = (gubEndOfMapScreenMessageList + 1) % 256;

	// if queue is full, end will now match the start
	if (gubEndOfMapScreenMessageList == gubStartOfMapScreenMessageList)
	{
		// if that's so, increment the start
		gubStartOfMapScreenMessageList = (gubStartOfMapScreenMessageList + 1) % 256;
	}
}


void DisplayStringsInMapScreenMessageList(void)
{
	SetFontDestBuffer(FRAME_BUFFER, STD_SCREEN_X + 17, STD_SCREEN_Y + 360 + 6, STD_SCREEN_X + 407, STD_SCREEN_Y + 360 + 101);

	SetFont(MAP_SCREEN_MESSAGE_FONT);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(DEFAULT_SHADOW);

	UINT8 ubCurrentStringIndex = gubCurrentMapMessageString;

	INT16 sY = STD_SCREEN_Y + 377;
	UINT16 usSpacing = GetFontHeight(MAP_SCREEN_MESSAGE_FONT);

	for (UINT8 ubLinesPrinted = 0; ubLinesPrinted < MAX_MESSAGES_ON_MAP_BOTTOM; ubLinesPrinted++)
	{
		// reached the end of the list?
		if (ubCurrentStringIndex == gubEndOfMapScreenMessageList)
		{
			break;
		}

		const ScrollStringSt* const s = gMapScreenMessageList[ubCurrentStringIndex].get();
		if (s == NULL) break;

		SetFontForeground(s->usColor);
		MPrint(STD_SCREEN_X + 20, sY, s->pString);

		sY += usSpacing;

		// next message index to print (may wrap around)
		ubCurrentStringIndex = (ubCurrentStringIndex + 1) % 256;
	}

	SetFontDestBuffer(FRAME_BUFFER);
}


void EnableDisableScrollStringVideoOverlay(BOOLEAN fEnable)
{
	/* will go through the list of video overlays for the tactical scroll message
	 * system, and enable/disable video overlays depending on fEnable */
	for (INT8 bCounter = 0; bCounter < MAX_LINE_COUNT; bCounter++)
	{
		const auto& s = gpDisplayList[bCounter];
		if (s != NULL)
		{
			EnableVideoOverlay(fEnable, s->video_overlay);
		}
	}
}


// play beep when new message is added
static void PlayNewMessageSound(void)
{
	// play a new message sound, if there is one playing, do nothing
	static UINT32 uiSoundId = NO_SAMPLE;

	// if a sound is playing, don't play new one
	if (uiSoundId != NO_SAMPLE && SoundIsPlaying(uiSoundId))
	{
		return;
	}

	uiSoundId = PlayJA2SampleFromFile(SOUNDSDIR "/newbeep.wav", MIDVOLUME, 1, MIDDLEPAN);
}


static std::shared_ptr<ScrollStringSt> ExtractScrollStringFromFile(HWFILE const f, bool stracLinuxFormat)
{
	UINT32 size;
	f->read(&size, sizeof(size));
	if (size == 0) return 0;

	auto s = std::make_shared<ScrollStringSt>();
	{
		SGP::Buffer<uint8_t> data(size);
		f->read(data, size);
		DataReader reader(data);
		if(stracLinuxFormat)
		{
			size_t const len = size / 4;
			s->pString = reader.readUTF32(len);
		}
		else
		{
			size_t const len = size / 2;
			s->pString = reader.readUTF16(len);
		}
	}

	BYTE data[28];
	f->read(data, sizeof(data));

	DataReader d{data};
	EXTR_SKIP(d, 4)
	EXTR_U32(d, s->uiTimeOfLastUpdate)
	EXTR_SKIP(d, 16)
	EXTR_U16(d, s->usColor)
	EXTR_BOOL(d, s->fBeginningOfNewString)
	EXTR_SKIP(d, 1)
	Assert(d.getConsumed() == lengthof(data));

	return s;
}


static void InjectScrollStringIntoFile(HWFILE const f, ScrollStringSt const* const s)
{
	if(!s)
	{
		UINT32 const size = 0;
		f->write(&size, sizeof(size));
		return;
	}

	ST::utf16_buffer utf16data = s->pString.to_utf16();
	UINT32 const size = static_cast<UINT32>(2 * (utf16data.size() + 1));
	f->write(&size, sizeof(size));
	f->write(utf16data.c_str(), size);

	BYTE data[28];
	DataWriter d{data};
	INJ_SKIP(d, 4)
	INJ_U32(d, s->uiTimeOfLastUpdate)
	INJ_SKIP(d, 16)
	INJ_U16(d, s->usColor)
	INJ_BOOL(d, s->fBeginningOfNewString)
	INJ_SKIP(d, 1)
	Assert(d.getConsumed() == lengthof(data));

	f->write(data, sizeof(data));
}


void SaveMapScreenMessagesToSaveGameFile(HWFILE const hFile)
{
	// write to the begining of the message list
	hFile->write(&gubEndOfMapScreenMessageList, sizeof(UINT8));

	hFile->write(&gubStartOfMapScreenMessageList, sizeof(UINT8));

	// write the current message string
	hFile->write(&gubCurrentMapMessageString, sizeof(UINT8));

	//Loopthrough all the messages
	for (const auto& i : gMapScreenMessageList)
	{
		InjectScrollStringIntoFile(hFile, i.get());
	}
}


void LoadMapScreenMessagesFromSaveGameFile(HWFILE const hFile, bool stracLinuxFormat)
{
	// clear tactical message queue
	ClearTacticalMessageQueue();

	gubEndOfMapScreenMessageList   = 0;
	gubStartOfMapScreenMessageList = 0;
	gubCurrentMapMessageString     = 0;

	// Read to the begining of the message list
	hFile->read(&gubEndOfMapScreenMessageList, sizeof(UINT8));

	// Read the current message string
	hFile->read(&gubStartOfMapScreenMessageList, sizeof(UINT8));

	// Read the current message string
	hFile->read(&gubCurrentMapMessageString, sizeof(UINT8));

	//Loopthrough all the messages
	for (auto& i : gMapScreenMessageList)
	{
		i = ExtractScrollStringFromFile(hFile, stracLinuxFormat);
	}

	// this will set a valid value for gubFirstMapscreenMessageIndex, which isn't being saved/restored
	MoveToEndOfMapScreenMessageList();
}

void ClearTacticalMessageQueue(void)
{
	ClearDisplayedListOfTacticalStrings();
	pStringS.clear();
}


void FreeGlobalMessageList(void)
{
	gMapScreenMessageList.fill(nullptr);
	gubEndOfMapScreenMessageList   = 0;
	gubStartOfMapScreenMessageList = 0;
	gubCurrentMapMessageString     = 0;
}


UINT8 GetRangeOfMapScreenMessages(void)
{
	// NOTE: End is non-inclusive, so start/end 0/0 means no messages, 0/1 means 1 message, etc.
	if (gubStartOfMapScreenMessageList <= gubEndOfMapScreenMessageList)
	{
		return gubEndOfMapScreenMessageList - gubStartOfMapScreenMessageList;
	}
	else
	{
		// this should always be 255 now, since this only happens when queue fills up, and we never remove any messages
		return 256 + gubEndOfMapScreenMessageList - gubStartOfMapScreenMessageList;
	}
}
