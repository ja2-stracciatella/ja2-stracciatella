#include "Directories.h"
#include "Font.h"
#include "Laptop.h"
#include "EMail.h"
#include "VObject.h"
#include "Debug.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Soldier_Profile.h"
#include "IMP_Compile_Character.h"
#include "IMP_Portraits.h"
#include "AIMMembers.h"
#include "Random.h"
#include "Text.h"
#include "LaptopSave.h"
#include "Finances.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"
#include "UILayout.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>


#define MAX_MESSAGES_PAGE 18 // max number of messages per page

#define VIEWER_X (155 + STD_SCREEN_X)
#define VIEWER_Y (70 + 21 + STD_SCREEN_Y)
#define MAIL_STRING_SIZE 320


enum EMailSortCriteria
{
	SENDER,
	RECEIVED,
	SUBJECT,
	READ
};


struct Page
{
	Email* Mail[MAX_MESSAGES_PAGE];
	Page* Next;
};


struct Record
{
	ST::string pRecord;
	Record* Next;
};


struct EmailPageInfoStruct
{
	Record* pFirstRecord;
	Record* pLastRecord;
	INT32 iPageNumber;
};


Email* pEmailList;
static Page* pPageList;
static INT32 iLastPage=-1;
static INT32 iCurrentPage=0;
Email* MailToDelete;
BOOLEAN fUnReadMailFlag=FALSE;
BOOLEAN fNewMailFlag=FALSE;
BOOLEAN fOldNewMailFlag=FALSE;
BOOLEAN fDisplayMessageFlag=FALSE;
static BOOLEAN fOldDisplayMessageFlag = FALSE;
static BOOLEAN fReDrawMessageFlag = FALSE;
BOOLEAN fOpenMostRecentUnReadFlag = FALSE;
static INT32 iViewerPositionY = 0;

static Email* CurrentMail;
static Email* PreviousMail;
static INT32 giMessagePage = -1;
static INT32 giNumberOfPagesToCurrentEmail = -1;
SGPVObject* guiEmailWarning;

#define EMAIL_TOP_BAR_HEIGHT 22

#define MIDDLE_X 0+LAPTOP_SCREEN_UL_X
#define MIDDLE_Y (72 + EMAIL_TOP_BAR_HEIGHT + STD_SCREEN_Y)
#define MIDDLE_WIDTH 19


// new graphics
#define EMAIL_LIST_WINDOW_Y 22

// email columns
#define SENDER_X LAPTOP_SCREEN_UL_X+65

#define DATE_X LAPTOP_SCREEN_UL_X+428

#define SUBJECT_X LAPTOP_SCREEN_UL_X+175
#define SUBJECT_WIDTH					254	//526-245
#define INDIC_X (128 + STD_SCREEN_X)

#define LINE_WIDTH 592-121

#define MESSAGE_X 5 //17
#define MESSAGE_WIDTH 528-125//150
#define MESSAGE_COLOR FONT_BLACK
#define MESSAGE_GAP 2


#define MESSAGE_HEADER_WIDTH 209-151
#define MESSAGE_HEADER_X VIEWER_X+4


#define EMAIL_WARNING_X (210 + STD_SCREEN_X)
#define EMAIL_WARNING_Y (140 + STD_SCREEN_Y)
#define EMAIL_WARNING_WIDTH 254
#define EMAIL_WARNING_HEIGHT 138


#define NEW_BTN_X EMAIL_WARNING_X +(338-245)
#define NEW_BTN_Y EMAIL_WARNING_Y +(278-195)

#define EMAIL_TEXT_FONT			FONT10ARIAL
#define MESSAGE_FONT			EMAIL_TEXT_FONT
#define EMAIL_HEADER_FONT		FONT14ARIAL
#define EMAIL_WARNING_FONT		FONT12ARIAL


// the max number of pages to an email
#define MAX_NUMBER_EMAIL_PAGES 100

#define NEXT_PAGE_X LAPTOP_UL_X + 562
#define NEXT_PAGE_Y (51 + STD_SCREEN_Y)

#define PREVIOUS_PAGE_X NEXT_PAGE_X - 21

#define ENVELOPE_BOX_X (116 + STD_SCREEN_X)

#define FROM_BOX_X (166 + STD_SCREEN_X)

#define SUBJECT_BOX_X (276 + STD_SCREEN_X)

#define DATE_BOX_X (530 + STD_SCREEN_X)

#define FROM_BOX_Y (51 + EMAIL_TOP_BAR_HEIGHT + STD_SCREEN_Y)

#define EMAIL_TITLE_FONT FONT14ARIAL
#define EMAIL_TITLE_X (140 + STD_SCREEN_X)
#define EMAIL_TITLE_Y (33 + STD_SCREEN_Y)
#define VIEWER_MESSAGE_BODY_START_Y VIEWER_Y+72
#define MIN_MESSAGE_HEIGHT_IN_LINES 5


#define INDENT_Y_OFFSET 310
#define INDENT_X_OFFSET 325
#define INDENT_X_WIDTH ( 544 - 481 )

// the position of the page number being displayed in the email program
#define PAGE_NUMBER_X (516 + STD_SCREEN_X)
#define PAGE_NUMBER_Y (58 + STD_SCREEN_Y)

// defines for location of message 'title'/'headers'

#define MESSAGE_FROM_Y VIEWER_Y+28

#define MESSAGE_DATE_Y MESSAGE_FROM_Y

#define MESSAGE_SUBJECT_Y MESSAGE_DATE_Y+16


#define SUBJECT_LINE_X VIEWER_X+47
#define SUBJECT_LINE_Y VIEWER_Y+42
#define SUBJECT_LINE_WIDTH 278-47


// maximum size of a email message page, so not to overrun the bottom of the screen
#define MAX_EMAIL_MESSAGE_PAGE_SIZE ( GetFontHeight( MESSAGE_FONT ) + MESSAGE_GAP ) * 20


// X button position
#define BUTTON_X VIEWER_X + 396
#define BUTTON_Y VIEWER_Y + 3 // was + 25
#define BUTTON_LOWER_Y BUTTON_Y + 22
#define PREVIOUS_PAGE_BUTTON_X VIEWER_X + 302
#define NEXT_PAGE_BUTTON_X VIEWER_X +395
#define DELETE_BUTTON_X NEXT_PAGE_BUTTON_X
#define LOWER_BUTTON_Y BUTTON_Y + 299


static BOOLEAN fSortDateUpwards = FALSE;
static BOOLEAN fSortSenderUpwards = FALSE;
static BOOLEAN fSortSubjectUpwards = FALSE;
static BOOLEAN gfPageButtonsWereCreated = FALSE;

// mouse regions
static MOUSE_REGION pEmailRegions[MAX_MESSAGES_PAGE];
static MOUSE_REGION pScreenMask;
static MOUSE_REGION pDeleteScreenMask;

// the email info struct to speed up email
static EmailPageInfoStruct pEmailPageInfo[MAX_NUMBER_EMAIL_PAGES];

//buttons
static GUIButtonRef giMessageButton;
static GUIButtonRef giDeleteMailButton[2];
static GUIButtonRef giSortButton[4];
static GUIButtonRef giNewMailButton;
static GUIButtonRef giMailMessageButtons[3];
static GUIButtonRef giMailPageButtons[2];
static MOUSE_REGION g_mail_scroll_region;


// the message record list, for the currently displayed message
static Record* pMessageRecordList = NULL;

// video handles
static SGPVObject* guiEmailTitle;
static SGPVObject* guiEmailBackground;
static SGPVObject* guiEmailIndicator;
static SGPVObject* guiEmailMessage;
static SGPVObject* guiMAILDIVIDER;


// the enumeration of headers
enum{
	FROM_HEADER=0,
	SUBJECT_HEADER,
	RECD_HEADER,
};


// whther or not we need to redraw the new mail box
BOOLEAN fReDrawNewMailFlag = FALSE;
static INT32 iTotalHeight = 0;


static void CreateNextPreviousEmailPageButtons(void);
static void EmailBtnCallBackPrimary(MOUSE_REGION* pRegion, UINT32 iReason);
static void EmailBtnCallBackSecondary(MOUSE_REGION* pRegion, UINT32 iReason);
static void EmailBtnCallBackScroll(MOUSE_REGION* pRegion, UINT32 iReason);


static void InitializeMouseRegions(void)
{
	// init mouseregions
	for (INT32 i = 0; i < MAX_MESSAGES_PAGE; ++i)
	{
		const UINT16 x = MIDDLE_X;
		const UINT16 y = MIDDLE_Y + MIDDLE_WIDTH * i;
		const UINT16 w = LINE_WIDTH;
		const UINT16 h = MIDDLE_WIDTH;
		MOUSE_REGION* const r = &pEmailRegions[i];
		MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_NORMAL + 2, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MouseCallbackPrimarySecondary(EmailBtnCallBackPrimary, EmailBtnCallBackSecondary, EmailBtnCallBackScroll));
		MSYS_SetRegionUserData(r, 0, i);
	}

	CreateNextPreviousEmailPageButtons();
}


static void DeleteEmailMouseRegions()
{
	FOR_EACH(MOUSE_REGION, i, pEmailRegions)     MSYS_RemoveRegion(&*i);
	FOR_EACH(GUIButtonRef, i, giMailPageButtons) RemoveButton(*i);
}


void GameInitEmail()
{
	pEmailList=NULL;
	pPageList=NULL;

	CurrentMail = NULL;
	PreviousMail = NULL;

	iLastPage=-1;

	iCurrentPage=0;
	MailToDelete = NULL;

	// reset display message flag
	fDisplayMessageFlag=FALSE;

	// reset page being displayed
	giMessagePage = 0;
}


static void CreateMailScreenButtons(void);


void EnterEmail()
{
	// load graphics

	iCurrentPage = LaptopSaveInfo.iCurrentEmailPage;

	// title bar
	guiEmailTitle = AddVideoObjectFromFile(LAPTOPDIR "/programtitlebar.sti");

	// the list background
	guiEmailBackground = AddVideoObjectFromFile(LAPTOPDIR "/mailwindow.sti");

	// the indication/notification box
	guiEmailIndicator = AddVideoObjectFromFile(LAPTOPDIR "/mailindicator.sti");

	// the message background
	guiEmailMessage = AddVideoObjectFromFile(LAPTOPDIR "/emailviewer.sti");

	// the message background
	guiMAILDIVIDER = AddVideoObjectFromFile(LAPTOPDIR "/maillistdivider.sti");

	// initialize mouse regions
	InitializeMouseRegions();

	// create buttons
	CreateMailScreenButtons( );

	// marks these buttons dirty
	MarkButtonsDirty( );

	// reset current page of the message being displayed
	giMessagePage = 0;

	// render email background and text
	RenderEmail();
}


static void AddDeleteRegionsToMessageRegion(INT32 iViewerY);
static void ClearOutEmailMessageRecordsList(void);
static void DestroyMailScreenButtons(void);


void ExitEmail()
{
	LaptopSaveInfo.iCurrentEmailPage = iCurrentPage;

	// clear out message record list
	ClearOutEmailMessageRecordsList( );

	// displayed message?...get rid of it
	if(fDisplayMessageFlag)
	{
		fDisplayMessageFlag = FALSE;
		AddDeleteRegionsToMessageRegion( 0 );
		fDisplayMessageFlag = TRUE;
		fReDrawMessageFlag = TRUE;
	}
	else
	{
		CurrentMail = NULL;
	}

	// delete mail notice?...get rid of it
	if (MailToDelete != NULL)
	{
		MailToDelete = NULL;
		CreateDestroyDeleteNoticeMailButton();
	}

	// remove all mouse regions in use in email
	DeleteEmailMouseRegions();

	// remove video objects being used by email screen
	DeleteVideoObject(guiEmailTitle);
	DeleteVideoObject(guiEmailBackground);
	DeleteVideoObject(guiMAILDIVIDER);
	DeleteVideoObject(guiEmailIndicator);
	DeleteVideoObject(guiEmailMessage);

	// remove buttons
	DestroyMailScreenButtons( );
}


static BOOLEAN DisplayDeleteNotice(Email* pMail);
static void DisplayEmailList(void);
static INT32 DisplayEmailMessage(Email* pMail);
static void HandleEmailViewerButtonStates(void);
static void OpenMostRecentUnreadEmail(void);
static void UpDateMessageRecordList(void);
static void UpdateStatusOfNextPreviousButtons(void);


void HandleEmail( void )
{

	INT32 iViewerY = 0;
	static BOOLEAN fEmailListBeenDrawAlready = FALSE;


	// check if email message record list needs to be updated
	UpDateMessageRecordList( );

	// does email list need to be draw, or can be drawn
	if (!fDisplayMessageFlag && !fNewMailFlag && MailToDelete == NULL && !fEmailListBeenDrawAlready)
	{
		DisplayEmailList();
		fEmailListBeenDrawAlready = TRUE;
	}
	// if the message flag, show message
	else if((fDisplayMessageFlag)&&(fReDrawMessageFlag))
	{
		// redisplay list
		DisplayEmailList();

		// this simply redraws message without button manipulation
		iViewerY = DisplayEmailMessage(CurrentMail);
		fEmailListBeenDrawAlready = FALSE;

	}
	else if((fDisplayMessageFlag)&&(!fOldDisplayMessageFlag))
	{

		// redisplay list
		DisplayEmailList();

		// this simply redraws message with button manipulation
		iViewerY = DisplayEmailMessage(CurrentMail);
		AddDeleteRegionsToMessageRegion( iViewerY );
		fEmailListBeenDrawAlready = FALSE;

	}

	// not displaying anymore?
	if (!fDisplayMessageFlag && fOldDisplayMessageFlag)
	{
		// then clear it out
		ClearOutEmailMessageRecordsList( );
	}


	// if new message is being displayed...check to see if it's buttons need to be created or destroyed
	AddDeleteRegionsToMessageRegion( 0 );

	// same with delete notice
	CreateDestroyDeleteNoticeMailButton();

	// if delete notice needs to be displayed?...display it
	if (MailToDelete != NULL) DisplayDeleteNotice(MailToDelete);


	// update buttons
	HandleEmailViewerButtonStates( );

	// handle buttons states
	UpdateStatusOfNextPreviousButtons( );

	if (fOpenMostRecentUnReadFlag)
	{
		// enter email due to email icon on program panel
		OpenMostRecentUnreadEmail( );
		fOpenMostRecentUnReadFlag = FALSE;

	}
}


static void DisplayTextOnTitleBar(void);
static void DisplayWhichPageOfEmailProgramIsDisplayed(void);
static void DrawLineDividers(void);
static void ReDisplayBoxes(void);


void RenderEmail( void )
{
	BltVideoObject(FRAME_BUFFER, guiEmailBackground, 0, LAPTOP_SCREEN_UL_X, EMAIL_LIST_WINDOW_Y + LAPTOP_SCREEN_UL_Y);
	BltVideoObject(FRAME_BUFFER, guiEmailTitle,      0, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y - 2);

	// show text on titlebar
	DisplayTextOnTitleBar( );

	DisplayEmailList( );

	// redraw line dividers
	DrawLineDividers( );

	BltVideoObject(FRAME_BUFFER, guiLaptopBACKGROUND, 0, STD_SCREEN_X + 108, STD_SCREEN_Y + 23);

	ReDisplayBoxes( );

	BlitTitleBarIcons(  );

	// show which page we are on
	DisplayWhichPageOfEmailProgramIsDisplayed( );

	InvalidateScreen();
}


void AddEmailWithSpecialData(INT32 iMessageOffset, INT32 iMessageLength, UINT8 ubSender, INT32 iDate, INT32 iFirstData, UINT32 uiSecondData )
{
	AddEmailMessage(iMessageOffset, iMessageLength, iDate, ubSender, FALSE, iFirstData, uiSecondData);
}


void AddEmail(INT32 iMessageOffset, INT32 iMessageLength, UINT8 ubSender, INT32 iDate)
{
	AddEmailMessage(iMessageOffset, iMessageLength, iDate, ubSender, FALSE, 0, 0);
}


void AddPreReadEmail(INT32 iMessageOffset, INT32 iMessageLength, UINT8 ubSender, INT32 iDate)
{
	AddEmailMessage(iMessageOffset, iMessageLength, iDate, ubSender, TRUE, 0, 0);
}


static ST::string LoadEMailText(UINT32 entry)
{
	return GCM->loadEncryptedString(BINARYDATADIR "/email.edt", MAIL_STRING_SIZE * entry, MAIL_STRING_SIZE);
}


static void AddMessageToPages(Email* Mail);
static ST::string ReplaceMercNameAndAmountWithProperData(const ST::string& pFinishedString, const Email* pMail);


void AddEmailMessage(INT32 iMessageOffset, INT32 iMessageLength, INT32 iDate, UINT8 ubSender, BOOLEAN fAlreadyRead, INT32 iFirstData, UINT32 uiSecondData)
{
	// will add a message to the list of messages

	// add new element onto list
	Email* const pTempEmail = new Email{};

	// copy offset and length of the actual message in email.edt
	pTempEmail->usOffset =(UINT16)iMessageOffset;
	pTempEmail->usLength =(UINT16)iMessageLength;

	// copy date and sender id's
	pTempEmail->iDate=iDate;
	pTempEmail->ubSender=ubSender;

	// the special data
	pTempEmail->iFirstData = iFirstData;
	pTempEmail->uiSecondData = uiSecondData;

	ST::string pSubject = LoadEMailText(iMessageOffset);
	pSubject = ReplaceMercNameAndAmountWithProperData(pSubject, pTempEmail);
	pTempEmail->pSubject = ST::format(" {}", pSubject);

	// place into list
	Email* pEmail = pEmailList;
	if(pEmail)
	{
		// list exists, place at end
		while (pEmail->Next != NULL) pEmail = pEmail->Next;
		pEmail->Next = pTempEmail;
	}
	else
	{
		// no list, becomes head of a new list
		pEmailList = pTempEmail;
	}
	pTempEmail->Prev = pEmail;

	// reset Next ptr
	pTempEmail->Next=NULL;

	// set flag that new mail has arrived
	fNewMailFlag=TRUE;

	// add this message to the pages of email
	AddMessageToPages(pTempEmail);

	// reset read flag of this particular message
	pTempEmail->fRead=fAlreadyRead;
}


static void RemoveEmailMessage(Email* Mail)
{
	Email* Next = Mail->Next;
	Email* Prev = Mail->Prev;
	if (Next != NULL) Next->Prev = Prev;
	if (Prev != NULL)
	{
		Prev->Next = Next;
	}
	else
	{
		Assert(pEmailList == Mail);
		pEmailList = Next;
	}
	delete Mail;
}


static void AddEmailPage(void)
{
	Page* const p = new Page{};
	FOR_EACH(Email*, i, p->Mail) *i = 0;
	p->Next = NULL;

	if (pPageList)
	{
		Page* last = pPageList;
		while (last->Next) last = last->Next;
		last->Next = p;
	}
	else
	{
		pPageList = p;
	}

	iLastPage++;
}


static void AddMessageToPages(Email* Mail)
{
	// go to end of page list
	Page* pPage = pPageList;
	INT32 iCounter=0;
	if(!pPage)
		AddEmailPage();
	pPage=pPageList;
	while (pPage->Next != NULL && pPage->Mail[MAX_MESSAGES_PAGE - 1] != NULL)
		pPage=pPage->Next;
	// if list is full, add new page
	while(iCounter <MAX_MESSAGES_PAGE)
	{
		if (pPage->Mail[iCounter] == NULL) break;
		iCounter++;
	}
	if(iCounter==MAX_MESSAGES_PAGE)
	{
		AddEmailPage();
		AddMessageToPages(Mail);
	}
	else
	{
		pPage->Mail[iCounter] = Mail;
	}
}


static void SortMessages(EMailSortCriteria Criterium)
{
	Email* NewList = NULL;

	for (Email* List = pEmailList; List != NULL;)
	{
		Email* Mail = List;
		List = List->Next;

		Email* InsAfter = NULL;
		for (Email* Other = NewList; Other != NULL; Other = Other->Next)
		{
			INT Order; // XXX HACK000E
			switch (Criterium)
			{
				case RECEIVED:
					Order = Mail->iDate - Other->iDate;
					if (fSortDateUpwards) Order = -Order;
					break;

				case SENDER:
					Order = pSenderNameList[Mail->ubSender].compare(pSenderNameList[Other->ubSender]);
					if (fSortSenderUpwards) Order = -Order;
					break;

				case SUBJECT:
					Order = Mail->pSubject.compare(Other->pSubject);
					if (fSortSubjectUpwards) Order = -Order;
					break;

				case READ:
					Order = Other->fRead - Mail->fRead;
					break;

				default: abort(); // HACK000E
			}
			if (Order > 0) break;
			InsAfter = Other;
		}
		Mail->Prev = InsAfter;
		if (InsAfter == NULL)
		{
			Mail->Next = NewList;
			NewList = Mail;
		}
		else
		{
			Mail->Next = InsAfter->Next;
			InsAfter->Next = Mail;
		}
		if (Mail->Next != NULL) Mail->Next->Prev = Mail;
	}
	pEmailList = NewList;

	fReDrawScreenFlag = TRUE;
}


static void ClearPages(void)
{
	// run through list of message pages and set to -1
	for (Page* i = pPageList; i;)
	{
		Page* const next = i->Next;
		delete i;
		i = next;
	}

	pPageList = NULL;
	iLastPage = -1;
}


static void PlaceMessagesinPages(void)
{
	Email* pEmail = pEmailList;
	// run through the list of messages and add to pages
	ClearPages();
	while(pEmail)
	{
		AddMessageToPages(pEmail);
		pEmail=pEmail->Next;

	}
	if(iCurrentPage >iLastPage)
		iCurrentPage=iLastPage;
}


// Draw the icon, sender, date, subject
static void DrawEmailSummary(INT32 y, const Email* e)
{
	const BOOLEAN read = e->fRead;
	const SGPFont font = read ? MESSAGE_FONT : FONT10ARIALBOLD;

	// will draw the icon for letter in mail list depending if the mail has been read or not
	BltVideoObject(FRAME_BUFFER, guiEmailIndicator, read ? 0 : 1, INDIC_X, y + 2);

	SetFont(font);

	ST::string pTempSubject = e->pSubject;
	pTempSubject = ReduceStringLength(pTempSubject, SUBJECT_WIDTH - 10, font);
	MPrint(SUBJECT_X, y + 4, pTempSubject);
	MPrint(SENDER_X,  y + 4, pSenderNameList[e->ubSender]);

	// draw date of message being displayed in mail viewer
	MPrint(DATE_X, y + 4, ST::format("{} {}", pDayStrings, e->iDate / (24 * 60)));
}


static Page* GetCurrentPage(void)
{
	Page* i = pPageList;
	if (i == NULL) return i;

	INT32 PageID = 0;
	while (i->Next != NULL && PageID++ != iCurrentPage) i = i->Next;
	return i;
}


static void DisplayEmailList(void)
{
	// look at current page, and display

	// if current page ever ends up negative, reset to 0
	if (iCurrentPage == -1) iCurrentPage = 0;

	const Page* const p = GetCurrentPage();
	if (p == NULL) return;

	// now we have current page, display it
	SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);

	// draw each line of the list for this page
	INT32 y = MIDDLE_Y;
	FOR_EACH(Email* const, e, p->Mail)
	{
		if (!*e) break;
		DrawEmailSummary(y, *e);
		y += MIDDLE_WIDTH;
	}

	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_LR_Y);

	SetFontShadow(DEFAULT_SHADOW);
}


void LookForUnread()
{
	BOOLEAN fStatusOfNewEmailFlag = fUnReadMailFlag;

	// simply runrs through list of messages, if any unread, set unread flag

	Email* pA = pEmailList;

	// reset unread flag
	fUnReadMailFlag=FALSE;

	// look for unread mail
	while(pA)
	{
		// unread mail found, set flag
		if(!(pA->fRead))
			fUnReadMailFlag=TRUE;
		pA=pA->Next;
	}

	if( fStatusOfNewEmailFlag != fUnReadMailFlag )
	{
		//Since there is no new email, get rid of the hepl text
		CreateFileAndNewEmailIconFastHelpText( LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_MAIL, (BOOLEAN )!fUnReadMailFlag );
	}
}


static void PrevListPage()
{
	if (iCurrentPage == 0) return;
	--iCurrentPage;
	RenderEmail();
	MarkButtonsDirty();
}


static void NextListPage()
{
	if (iCurrentPage == iLastPage) return;
	++iCurrentPage;
	RenderEmail();
	MarkButtonsDirty();
}


static void EmailBtnCallBackPrimary(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if(fDisplayMessageFlag)
		return;

	Page* pPage = GetCurrentPage();
	if (pPage == NULL) return;

	// error check
	INT32 iCount = MSYS_GetRegionUserData(pRegion, 0);

	Email* Mail = pPage->Mail[iCount];

	// invalid message
	if (Mail == NULL)
	{
		fDisplayMessageFlag=FALSE;
		return;
	}
	// Get email and display
	fDisplayMessageFlag=TRUE;
	giMessagePage = 0;
	PreviousMail = CurrentMail;
	CurrentMail = Mail;
}

static void EmailBtnCallBackSecondary(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if(fDisplayMessageFlag)
		return;

	Page* pPage = GetCurrentPage();
	if (pPage == NULL)
	{
		HandleRightButtonUpEvent();
		return;
	}

	INT32 iCount = MSYS_GetRegionUserData(pRegion, 0);

	giMessagePage = 0;

	Email* Mail = pPage->Mail[iCount];
	if (Mail == NULL)
	{
		// no mail here, handle right button up event
		HandleRightButtonUpEvent( );
		return;
	}
	else
	{
		MailToDelete = Mail;
	}
}

static void EmailBtnCallBackScroll(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if(fDisplayMessageFlag)
		return;

	if (iReason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		PrevListPage();
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		NextListPage();
	}
}


static void BtnMessageXCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_ANY_BUTTON_UP)
	{
		// X button has been pressed and let up, this means to stop displaying the currently displayed message

		// reset display message flag
		fDisplayMessageFlag = FALSE;

		// reset page being displayed
		giMessagePage = 0;

		// force update of entire screen
		fPausedReDrawScreenFlag = TRUE;
	}
}


static Record* GetFirstRecordOnThisPage(Record* const RecordList, INT32 const iPage)
{
	// get the first record on this page - build pages up until this point

	Record* CurrentRecord = NULL;

	INT32 iCurrentPositionOnThisPage = 0;
	INT32 iCurrentPage =0;



	// null record list, nothing to do
	if( RecordList == NULL )
	{
		return ( CurrentRecord );
	}

	CurrentRecord = RecordList;

	// while we are not on the current page
	SGPFont const font = MESSAGE_FONT;
	while( iCurrentPage < iPage )
	{
		// build record list to this point
		for (;;)
		{
			UINT16 const h = IanWrappedStringHeight(MESSAGE_WIDTH, MESSAGE_GAP, font, CurrentRecord->pRecord);
			if (iCurrentPositionOnThisPage + h > MAX_EMAIL_MESSAGE_PAGE_SIZE) break;

			// still room on this page
			iCurrentPositionOnThisPage += h;

			// next record
			CurrentRecord = CurrentRecord -> Next;

			// check if we have gone too far?
			if( CurrentRecord == NULL )
			{
				return( CurrentRecord );
			}
		}

		// reset position
		iCurrentPositionOnThisPage = 0;

		// next page
		iCurrentPage++;
	}

	return ( CurrentRecord );
}


static void DisplayEmailMessageSubjectDateFromLines(Email* pMail, INT32 iViewerY);
static void DisplayNumberOfPagesToThisEmail(INT32 iViewerY);
static void DrawEmailMessageDisplayTitleText(INT32 iViewerY);
static void HandleAnySpecialEmailMessageEvents(INT32 iMessageId);
static void HandleMailSpecialMessages(UINT16 usMessageId, Email* pMail);
static void PreProcessEmail(Email* pMail);


static INT32 DisplayEmailMessage(Email* const m)
{
	if (!m) return 0;

	// reset redraw email message flag
	fReDrawMessageFlag = FALSE;

	// we KNOW the player is going to "read" this, so mark it as so
	m->fRead = TRUE;

	INT32 const offset = m->usOffset;
	HandleAnySpecialEmailMessageEvents(offset);
	HandleMailSpecialMessages(offset, m);

	PreProcessEmail(m);

	INT32 const by = iViewerPositionY;

	BltVideoObject(FRAME_BUFFER, guiEmailMessage,  0, VIEWER_X,     VIEWER_Y + by);
	BltVideoObject(FRAME_BUFFER, guiTITLEBARICONS, 0, VIEWER_X + 5, VIEWER_Y + by + 2);

	DisplayEmailMessageSubjectDateFromLines(m, by);
	DrawEmailMessageDisplayTitleText(by);

	UINT16 const h = GetFontHeight(MESSAGE_FONT);
	INT32        y = VIEWER_MESSAGE_BODY_START_Y + by;

	BltVideoObject(FRAME_BUFFER, guiEmailMessage, 1, VIEWER_X, y);
	y += h;

	// Blit the text background based on height
	for (INT32 i = 1; i < iTotalHeight / h; ++i)
	{
		BltVideoObject(FRAME_BUFFER, guiEmailMessage, 1, VIEWER_X, y);
		y += h;
	}

	BOOLEAN onlyOnePage = giNumberOfPagesToCurrentEmail <= 2;

	// The bottom piece to the message viewer
	BltVideoObject(FRAME_BUFFER, guiEmailMessage, onlyOnePage ? 2 : 3, VIEWER_X, y);

	// Draw body of text. Any particular email can encompass more than one
	// "record" in the email file. Draw each record (length is number of records)
	if (Record const* i = pEmailPageInfo[giMessagePage].pFirstRecord)
	{
		for (INT32 y = VIEWER_MESSAGE_BODY_START_Y + by + h;;)
		{
			y += IanDisplayWrappedString(VIEWER_X + MESSAGE_X + 4, y, MESSAGE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, MESSAGE_COLOR, i->pRecord, 0, IAN_WRAP_NO_SHADOW);

			i = i->Next;
			if (!i) break;
			if ( pEmailPageInfo[giMessagePage    ].pLastRecord != i) continue;
			if (!pEmailPageInfo[giMessagePage + 1].pFirstRecord)     continue;
			break;
		}
	}

	if(!onlyOnePage)
	{
		DisplayNumberOfPagesToThisEmail(by);
	}

	InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y, LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_LR_Y);

	return by;
}


static void BtnNewOkback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		fNewMailFlag=FALSE;
	}
}


static GUIButtonRef MakeButtonNewMail(INT32 image, INT16 x, INT16 y, GUI_CALLBACK click)
{
	GUIButtonRef const btn = QuickCreateButtonImg(LAPTOPDIR "/newmailbuttons.sti", image, image + 3, x, y, MSYS_PRIORITY_HIGHEST - 1, click);
	btn->SetCursor(CURSOR_LAPTOP_SCREEN);
	return btn;
}


static void PrevMailPage()
{
	if (giMessagePage == 0) return;
	--giMessagePage;
	MarkButtonsDirty();
	fReDrawScreenFlag = TRUE;
}


static void NextMailPage()
{
	if (giMessagePage + 1 >= giNumberOfPagesToCurrentEmail - 1) return;
	++giMessagePage;
	MarkButtonsDirty();
	fReDrawScreenFlag = TRUE;
}


static void MailScrollRegionCallback(MOUSE_REGION* const, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		PrevMailPage();
	}
	else if (reason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		NextMailPage();
	}
}


static void BtnDeleteCallback(GUI_BUTTON* btn, UINT32 iReason);
static void BtnNextEmailPageCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnPreviousEmailPageCallback(GUI_BUTTON* btn, UINT32 reason);


static void AddDeleteRegionsToMessageRegion(INT32 iViewerY)
{
	// will create/destroy mouse region for message display

	if((fDisplayMessageFlag)&&(!fOldDisplayMessageFlag))
	{

		// set old flag
		fOldDisplayMessageFlag=TRUE;

		// add X button
		giMessageButton = QuickCreateButtonImg(LAPTOPDIR "/x.sti", 0, 1, BUTTON_X + 2, BUTTON_Y + iViewerY + 1, MSYS_PRIORITY_HIGHEST - 1, BtnMessageXCallback);
		giMessageButton->SetCursor(CURSOR_LAPTOP_SCREEN);

		if( giNumberOfPagesToCurrentEmail > 2 )
		{
			// add next and previous mail page buttons
			{
				INT16 const y = LOWER_BUTTON_Y + iViewerY + 2;
				giMailMessageButtons[0] = MakeButtonNewMail(0, PREVIOUS_PAGE_BUTTON_X, y, BtnPreviousEmailPageCallback);
				giMailMessageButtons[1] = MakeButtonNewMail(1, NEXT_PAGE_BUTTON_X,     y, BtnNextEmailPageCallback);
			}
			{
				UINT16 const x = VIEWER_X + MESSAGE_X + 1;
				UINT16 const y = VIEWER_MESSAGE_BODY_START_Y + iViewerPositionY;
				UINT16 const w = MESSAGE_WIDTH + 3;
				UINT16 const h = 227;
				MSYS_DefineRegion(&g_mail_scroll_region, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MailScrollRegionCallback);
			}
			gfPageButtonsWereCreated = TRUE;
		}

		giMailMessageButtons[2] = MakeButtonNewMail(2, DELETE_BUTTON_X, BUTTON_LOWER_Y + iViewerY + 2, BtnDeleteCallback);

		// force update of screen
		fReDrawScreenFlag=TRUE;
	}
	else if((!fDisplayMessageFlag)&&(fOldDisplayMessageFlag))
	{
		// delete region
		fOldDisplayMessageFlag=FALSE;
		RemoveButton(giMessageButton);

		// net/previous email page buttons
		if( gfPageButtonsWereCreated )
		{
			MSYS_RemoveRegion(&g_mail_scroll_region);
			RemoveButton(giMailMessageButtons[0] );
			RemoveButton(giMailMessageButtons[1] );
			gfPageButtonsWereCreated = FALSE;
		}
		RemoveButton(giMailMessageButtons[2] );
		// force update of screen
		fReDrawScreenFlag=TRUE;
	}
}


static GUIButtonRef MakeButtonYesNo(INT32 image, INT16 x, GUI_CALLBACK click)
{
	GUIButtonRef const btn = QuickCreateButtonImg(LAPTOPDIR "/yesnobuttons.sti", image, image + 1, x, NEW_BTN_Y, MSYS_PRIORITY_HIGHEST - 2, click);
	btn->SetCursor(CURSOR_LAPTOP_SCREEN);
	return btn;
}


void CreateDestroyNewMailButton()
{
	static BOOLEAN fOldNewMailFlag=FALSE;

	// check if we are video conferencing, if so, do nothing
	if (gubVideoConferencingMode != AIM_VIDEO_NOT_DISPLAYED_MODE)
	{
		return ;
	}


	if((fNewMailFlag)&&(!fOldNewMailFlag))
	{
		// create new mail dialog box button

		// set old flag (stating button has been created)
		fOldNewMailFlag=TRUE;

		giNewMailButton = MakeButtonYesNo(0, NEW_BTN_X + 10, BtnNewOkback);

		// set up screen mask region
		MSYS_DefineRegion(&pScreenMask, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST - 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, LapTopScreenCallBack);
		MarkAButtonDirty(giNewMailButton);
		fReDrawScreenFlag = TRUE;
	}
	else if((!fNewMailFlag)&&(fOldNewMailFlag))
	{
		// reset old flag
		fOldNewMailFlag=FALSE;

		// remove the button
		RemoveButton(giNewMailButton);

		// remove screen mask
		MSYS_RemoveRegion( &pScreenMask );

		// redraw screen
		fPausedReDrawScreenFlag=TRUE;
	}
}


void DisplayNewMailBox(void)
{
	// will display a new mail box whenever new mail has arrived

	// check if we are video conferencing, if so, do nothing
	if (gubVideoConferencingMode != AIM_VIDEO_NOT_DISPLAYED_MODE) return;

	// not even set, leave NOW!
	if (!fNewMailFlag) return;

	BltVideoObject(FRAME_BUFFER, guiEmailWarning,  0, EMAIL_WARNING_X,     EMAIL_WARNING_Y);
	BltVideoObject(FRAME_BUFFER, guiTITLEBARICONS, 0, EMAIL_WARNING_X + 5, EMAIL_WARNING_Y + 2);

	SetFontAttributes(EMAIL_HEADER_FONT, FONT_WHITE);

	// print warning
	MPrint(EMAIL_WARNING_X + 30, EMAIL_WARNING_Y + 8, pEmailTitleText);

	SetFontAttributes(EMAIL_WARNING_FONT, FONT_BLACK, NO_SHADOW);

	// printf warning string
	MPrint(EMAIL_WARNING_X + 60, EMAIL_WARNING_Y + 63, pNewMailStrings);

	// invalidate region
	InvalidateRegion( EMAIL_WARNING_X, EMAIL_WARNING_Y, EMAIL_WARNING_X + 270, EMAIL_WARNING_Y + 200 );

	// mark button
	MarkAButtonDirty(giNewMailButton);

	// reset shadow
	SetFontShadow( DEFAULT_SHADOW );
}


void ReDrawNewMailBox(void)
{ // check to see if the new mail region needs to be redrawn
	if (!fReDrawNewMailFlag) return;
	fReDrawNewMailFlag = FALSE;
	if (!fNewMailFlag) return;
	DisplayNewMailBox();
}


static void NextRegionButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		NextListPage();
	}
}


static void BtnPreviousEmailPageCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		PrevMailPage();
	}
}


static void BtnNextEmailPageCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		NextMailPage();
	}
}


static void PreviousRegionButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		PrevListPage();
	}
}


static void BtnDeleteNoback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		MailToDelete = NULL;
		fReDrawScreenFlag = TRUE;
	}
}


static void DeleteEmail(void);


static void BtnDeleteYesback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		fReDrawScreenFlag = TRUE;
		DeleteEmail();
	}
}


void CreateDestroyDeleteNoticeMailButton()
{
	static BOOLEAN fOldDeleteMailFlag=FALSE;
	if (MailToDelete != NULL && !fOldDeleteMailFlag)
	{
		// confirm delete email buttons

		// YES/NO buttons
		fOldDeleteMailFlag=TRUE;
		giDeleteMailButton[0] = MakeButtonYesNo(0, NEW_BTN_X +  1, BtnDeleteYesback);
		giDeleteMailButton[1] = MakeButtonYesNo(2, NEW_BTN_X + 40, BtnDeleteNoback);

		// set up screen mask to prevent other actions while delete mail box is destroyed
		MSYS_DefineRegion(&pDeleteScreenMask, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST - 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, LapTopScreenCallBack);

		// force update
		fReDrawScreenFlag = TRUE;

	}
	else if (MailToDelete == NULL && fOldDeleteMailFlag)
	{
		// clear out the buttons and screen mask
		fOldDeleteMailFlag=FALSE;
		RemoveButton( giDeleteMailButton[0] );
		RemoveButton( giDeleteMailButton[1] );

		// the region
		MSYS_RemoveRegion(&pDeleteScreenMask);

		// force refresh
		fReDrawScreenFlag=TRUE;
	}
}


static BOOLEAN DisplayDeleteNotice(Email* pMail)
{
	// will display a delete mail box whenever delete mail has arrived
	if( !fReDrawScreenFlag )
	{
		// no redraw flag, leave
		return( FALSE );
	}

	// error check.. no valid message passed
	if( pMail == NULL )
	{
		return ( FALSE );
	}

	BltVideoObject(FRAME_BUFFER, guiEmailWarning, 0, EMAIL_WARNING_X, EMAIL_WARNING_Y);

	SetFontAttributes(EMAIL_HEADER_FONT, FONT_WHITE);

	BltVideoObject(FRAME_BUFFER, guiTITLEBARICONS, 0, EMAIL_WARNING_X + 5, EMAIL_WARNING_Y + 2);

	// title
	MPrint(EMAIL_WARNING_X + 30, EMAIL_WARNING_Y + 8, pEmailTitleText);

	SetFontAttributes(EMAIL_WARNING_FONT, FONT_BLACK, NO_SHADOW);

	// draw text based on mail being read or not
	if((pMail->fRead))
		MPrint(EMAIL_WARNING_X + 95, EMAIL_WARNING_Y + 65, pDeleteMailStrings[0]);
	else
		MPrint(EMAIL_WARNING_X + 70, EMAIL_WARNING_Y + 65, pDeleteMailStrings[1]);


	// invalidate screen area, for refresh

	if( ! fNewMailFlag )
	{
		// draw buttons
		MarkButtonsDirty( );
		InvalidateRegion(EMAIL_WARNING_X, EMAIL_WARNING_Y ,EMAIL_WARNING_X+EMAIL_WARNING_WIDTH,EMAIL_WARNING_Y+EMAIL_WARNING_HEIGHT);
	}

	// reset font shadow
	SetFontShadow(DEFAULT_SHADOW);

	return ( TRUE );
}


static void DeleteEmail(void)
{

	// error check, invalid mail, or not time to delete mail
	if (MailToDelete == NULL) return;
	// remove the message
	RemoveEmailMessage(MailToDelete);
	MailToDelete = NULL;

	// stop displaying message, if so
	fDisplayMessageFlag = FALSE;

	// upadte list
	PlaceMessagesinPages();

	// if all of a sudden we are beyond last page, move back one
	if(iCurrentPage > iLastPage)
		iCurrentPage=iLastPage;

	// rerender mail list
	RenderEmail();

	fReDrawScreenFlag=TRUE;

	InvalidateScreen();
}


static void FromCallback(GUI_BUTTON *btn, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		// sort messages based on sender name, then replace into pages of email
		fSortSenderUpwards = !fSortSenderUpwards;
		SortMessages(SENDER);
		PlaceMessagesinPages();
	}
}


static void SubjectCallback(GUI_BUTTON *btn, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		// sort message on subject and reorder list
		fSortSubjectUpwards = !fSortSubjectUpwards;
		SortMessages(SUBJECT);
		PlaceMessagesinPages();
	}
}


static void BtnDeleteCallback(GUI_BUTTON *btn, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		MailToDelete = CurrentMail;
	}
}


static void DateCallback(GUI_BUTTON *btn, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		// sort messages based on date recieved and reorder lsit
		fSortDateUpwards = !fSortDateUpwards;
		SortMessages(RECEIVED);
		PlaceMessagesinPages();
	}
}


static void ReadCallback(GUI_BUTTON *btn, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		// sort messages based on date recieved and reorder lsit
		SortMessages(READ);
		PlaceMessagesinPages();
	}
}


static void DisplayTextOnTitleBar(void)
{
	// draw email screen title text
	SetFontAttributes(EMAIL_TITLE_FONT, FONT_WHITE);
	MPrint(EMAIL_TITLE_X, EMAIL_TITLE_Y, pEmailTitleText);
}


static void DestroyMailScreenButtons(void)
{
	// this function will destory the buttons used in the email screen

	// the sort email buttons
	RemoveButton( giSortButton[0] );
	RemoveButton( giSortButton[1] );
	RemoveButton( giSortButton[2] );
	RemoveButton( giSortButton[3] );
}


static void MakeButton(UINT idx, INT16 x, GUI_CALLBACK click, const ST::string& text)
{
	GUIButtonRef const btn = QuickCreateButtonImg(LAPTOPDIR "/mailbuttons.sti", idx, idx + 4, x, FROM_BOX_Y, MSYS_PRIORITY_HIGHEST - 1, click);
	giSortButton[idx] = btn;
	btn->SetCursor(CURSOR_LAPTOP_SCREEN);
	if (!text.empty())
	{
		btn->SpecifyGeneralTextAttributes(text, EMAIL_WARNING_FONT, FONT_BLACK, FONT_BLACK);
	}
}


static void CreateMailScreenButtons(void)
{
	// create sort buttons, right now - not finished
	MakeButton(0, ENVELOPE_BOX_X, ReadCallback,    {});
	MakeButton(1, FROM_BOX_X,     FromCallback,    pEmailHeaders[FROM_HEADER]);
	MakeButton(2, SUBJECT_BOX_X,  SubjectCallback, pEmailHeaders[SUBJECT_HEADER]);
	MakeButton(3, DATE_BOX_X,     DateCallback,    pEmailHeaders[RECD_HEADER]);
}


static void DisplayEmailMessageSubjectDateFromLines(Email* pMail, INT32 iViewerY)
{
	// this procedure will draw the title/headers to From, Subject, Date fields in the display
	// message box

	SetFontAttributes(MESSAGE_FONT, FONT_BLACK, NO_SHADOW);

	// all headers, but not info are right justified
	RightAlign const alignment{ MESSAGE_HEADER_WIDTH };

	// print from
	MPrint(MESSAGE_HEADER_X - 20, MESSAGE_FROM_Y + iViewerY, pEmailHeaders[0], alignment);

	// the actual from info
	MPrint( MESSAGE_HEADER_X+MESSAGE_HEADER_WIDTH-13, MESSAGE_FROM_Y + iViewerY, pSenderNameList[pMail->ubSender]);

	// print date
	MPrint(MESSAGE_HEADER_X + 168, MESSAGE_DATE_Y + iViewerY, pEmailHeaders[2], alignment);

	// the actual date info
	MPrint(MESSAGE_HEADER_X + 235, MESSAGE_DATE_Y + iViewerY, ST::format("{}", pMail->iDate / (24 * 60)));

	// print subject
	MPrint(MESSAGE_HEADER_X - 20, MESSAGE_SUBJECT_Y + iViewerY, pEmailHeaders[1], alignment);

 	// the actual subject info
	IanDisplayWrappedString(SUBJECT_LINE_X + 2, SUBJECT_LINE_Y + 2 + iViewerY, SUBJECT_LINE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, MESSAGE_COLOR, pMail->pSubject, 0, 0);

	// reset shadow
	SetFontShadow(DEFAULT_SHADOW);
}


static void DrawEmailMessageDisplayTitleText(INT32 iViewerY)
{
	// this procedure will display the title of the email message display box
	SetFontAttributes(EMAIL_HEADER_FONT, FONT_WHITE);
	MPrint(VIEWER_X + 30, VIEWER_Y + 8 + iViewerY, pEmailTitleText);
}


static void DrawLineDividers(void)
{
	// this function draws divider lines between lines of text
	INT32 iCounter=0;

	for(iCounter=1; iCounter < 19; iCounter++)
	{
		BltVideoObject(FRAME_BUFFER, guiMAILDIVIDER, 0, INDIC_X - 10, MIDDLE_Y + iCounter * MIDDLE_WIDTH - 1);
	}
}


static void ClearOutEmailMessageRecordsList(void)
{
	Record* pTempRecord;
	INT32 iCounter = 0;

	// runt hrough list freeing records up
	while(pMessageRecordList)
	{
		// set temp to current
		pTempRecord = pMessageRecordList;

		// next element
		pMessageRecordList = pMessageRecordList -> Next;

		delete pTempRecord;
	}

	for( iCounter = 0; iCounter < MAX_NUMBER_EMAIL_PAGES; iCounter++ )
	{
		pEmailPageInfo[ iCounter ].pFirstRecord = NULL;
		pEmailPageInfo[ iCounter ].pLastRecord = NULL;
		pEmailPageInfo[ iCounter ].iPageNumber = iCounter;
	}

	// null out list
	pMessageRecordList = NULL;
}


static void AddEmailRecordToList(const ST::string& text)
{
	Record* const e = new Record{};
	e->Next = NULL;
	e->pRecord = text;

	// Append node to list
	Record** anchor = &pMessageRecordList;
	while (*anchor != NULL) anchor = &(*anchor)->Next;
	*anchor = e;
}


static void UpDateMessageRecordList(void)
{

	// simply checks to see if old and new message ids are the same, if so, do nothing
	// otherwise clear list

	if (CurrentMail != PreviousMail)
	{
		// if chenged, clear list
		ClearOutEmailMessageRecordsList( );

		// set prev to current
		PreviousMail = CurrentMail;
	}

}


static void HandleAnySpecialEmailMessageEvents(INT32 iMessageId)
{
	// handles any special message events
	switch( iMessageId )
	{
		case( IMP_EMAIL_AGAIN ):
			SetBookMark(IMP_BOOKMARK);
			break;
		case( IMP_EMAIL_INTRO ):
			SetBookMark(IMP_BOOKMARK);
			break;
	}
}


static void ReDisplayBoxes(void)
{
	// the email message itself
	if(fDisplayMessageFlag)
	{
		// this simply redraws message with button manipulation
		DisplayEmailMessage(CurrentMail);
	}

	if (MailToDelete != NULL) DisplayDeleteNotice(MailToDelete);

	if(fNewMailFlag)
	{
		// if new mail, redisplay box
		DisplayNewMailBox( );
	}
}


static void HandleIMPCharProfileResultsMessage(void);
static void ModifyInsuranceEmails(UINT16 usMessageId, Email* pMail, UINT8 ubNumberOfRecords);


static void HandleMailSpecialMessages(UINT16 usMessageId, Email* pMail)
{
	// this procedure will handle special cases of email messages that are not stored in email.edt, or need special processing
	switch( usMessageId )
	{
		case( IMP_EMAIL_PROFILE_RESULTS ):

			HandleIMPCharProfileResultsMessage( );
		break;
		case( MERC_INTRO ):
			SetBookMark( MERC_BOOKMARK );
			fReDrawScreenFlag = TRUE;
		break;


		case INSUR_PAYMENT:
		case INSUR_SUSPIC:
		case INSUR_SUSPIC_2:
		case INSUR_INVEST_OVER:
			ModifyInsuranceEmails(usMessageId, pMail, INSUR_PAYMENT_LENGTH);
			break;

		case INSUR_1HOUR_FRAUD:
			ModifyInsuranceEmails(usMessageId, pMail, INSUR_1HOUR_FRAUD_LENGTH);
			break;

		case MERC_NEW_SITE_ADDRESS:
			//Set the book mark so the player can access the site
			SetBookMark( MERC_BOOKMARK );
			break;

		case MERC_DIED_ON_OTHER_ASSIGNMENT:
			ModifyInsuranceEmails(usMessageId, pMail, MERC_DIED_ON_OTHER_ASSIGNMENT_LENGTH);
			break;

		case AIM_MEDICAL_DEPOSIT_REFUND:
		case AIM_MEDICAL_DEPOSIT_NO_REFUND:
		case AIM_MEDICAL_DEPOSIT_PARTIAL_REFUND:
			ModifyInsuranceEmails(usMessageId, pMail, AIM_MEDICAL_DEPOSIT_REFUND_LENGTH);
			break;
	}
}



#define IMP_RESULTS_INTRO_LENGTH 9

#define IMP_RESULTS_PERSONALITY_INTRO IMP_RESULTS_INTRO_LENGTH
#define IMP_RESULTS_PERSONALITY_INTRO_LENGTH 5
#define IMP_PERSONALITY_NORMAL IMP_RESULTS_PERSONALITY_INTRO + IMP_RESULTS_PERSONALITY_INTRO_LENGTH
#define IMP_PERSONALITY_LENGTH 4
#define IMP_PERSONALITY_HEAT IMP_PERSONALITY_NORMAL + IMP_PERSONALITY_LENGTH
#define IMP_PERSONALITY_NERVOUS IMP_PERSONALITY_HEAT + IMP_PERSONALITY_LENGTH
#define IMP_PERSONALITY_CLAUSTROPHOBIC IMP_PERSONALITY_NERVOUS + IMP_PERSONALITY_LENGTH
#define IMP_PERSONALITY_NONSWIMMER IMP_PERSONALITY_CLAUSTROPHOBIC + IMP_PERSONALITY_LENGTH
#define IMP_PERSONALITY_FEAR_OF_INSECTS IMP_PERSONALITY_NONSWIMMER + IMP_PERSONALITY_LENGTH
#define IMP_PERSONALITY_FORGETFUL IMP_PERSONALITY_FEAR_OF_INSECTS + IMP_PERSONALITY_LENGTH + 1
#define IMP_PERSONALITY_PSYCHO IMP_PERSONALITY_FORGETFUL + IMP_PERSONALITY_LENGTH
#define IMP_RESULTS_ATTITUDE_INTRO IMP_PERSONALITY_PSYCHO + IMP_PERSONALITY_LENGTH + 1
#define IMP_RESULTS_ATTITUDE_LENGTH 5
#define IMP_ATTITUDE_LENGTH 5
#define IMP_ATTITUDE_NORMAL IMP_RESULTS_ATTITUDE_INTRO + IMP_RESULTS_ATTITUDE_LENGTH
#define IMP_ATTITUDE_FRIENDLY IMP_ATTITUDE_NORMAL + IMP_ATTITUDE_LENGTH
#define IMP_ATTITUDE_LONER IMP_ATTITUDE_FRIENDLY + IMP_ATTITUDE_LENGTH + 1
#define IMP_ATTITUDE_OPTIMIST IMP_ATTITUDE_LONER + IMP_ATTITUDE_LENGTH + 1
#define IMP_ATTITUDE_PESSIMIST IMP_ATTITUDE_OPTIMIST + IMP_ATTITUDE_LENGTH + 1
#define IMP_ATTITUDE_AGGRESSIVE IMP_ATTITUDE_PESSIMIST + IMP_ATTITUDE_LENGTH + 1
#define IMP_ATTITUDE_ARROGANT IMP_ATTITUDE_AGGRESSIVE + IMP_ATTITUDE_LENGTH + 1
#define IMP_ATTITUDE_ASSHOLE IMP_ATTITUDE_ARROGANT + IMP_ATTITUDE_LENGTH + 1
#define IMP_ATTITUDE_COWARD IMP_ATTITUDE_ASSHOLE + IMP_ATTITUDE_LENGTH
#define IMP_RESULTS_SKILLS IMP_ATTITUDE_COWARD + IMP_ATTITUDE_LENGTH + 1
#define IMP_RESULTS_SKILLS_LENGTH 7
#define IMP_SKILLS_IMPERIAL_SKILLS IMP_RESULTS_SKILLS + IMP_RESULTS_SKILLS_LENGTH + 1
#define IMP_SKILLS_IMPERIAL_MARK IMP_SKILLS_IMPERIAL_SKILLS + 1
#define IMP_SKILLS_IMPERIAL_MECH IMP_SKILLS_IMPERIAL_SKILLS + 2
#define IMP_SKILLS_IMPERIAL_EXPL IMP_SKILLS_IMPERIAL_SKILLS + 3
#define IMP_SKILLS_IMPERIAL_MED  IMP_SKILLS_IMPERIAL_SKILLS + 4

#define IMP_SKILLS_NEED_TRAIN_SKILLS IMP_SKILLS_IMPERIAL_MED + 1
#define IMP_SKILLS_NEED_TRAIN_MARK IMP_SKILLS_NEED_TRAIN_SKILLS + 1
#define IMP_SKILLS_NEED_TRAIN_MECH IMP_SKILLS_NEED_TRAIN_SKILLS + 2
#define IMP_SKILLS_NEED_TRAIN_EXPL IMP_SKILLS_NEED_TRAIN_SKILLS + 3
#define IMP_SKILLS_NEED_TRAIN_MED IMP_SKILLS_NEED_TRAIN_SKILLS + 4

#define IMP_SKILLS_NO_SKILL IMP_SKILLS_NEED_TRAIN_MED + 1
#define IMP_SKILLS_NO_SKILL_MARK  IMP_SKILLS_NO_SKILL + 1
#define IMP_SKILLS_NO_SKILL_MECH  IMP_SKILLS_NO_SKILL + 2
#define IMP_SKILLS_NO_SKILL_EXPL  IMP_SKILLS_NO_SKILL + 3
#define IMP_SKILLS_NO_SKILL_MED   IMP_SKILLS_NO_SKILL + 4

#define IMP_SKILLS_SPECIAL_INTRO IMP_SKILLS_NO_SKILL_MED + 1
#define IMP_SKILLS_SPECIAL_INTRO_LENGTH 2
#define IMP_SKILLS_SPECIAL_LOCK IMP_SKILLS_SPECIAL_INTRO + IMP_SKILLS_SPECIAL_INTRO_LENGTH
#define IMP_SKILLS_SPECIAL_HAND IMP_SKILLS_SPECIAL_LOCK + 1
#define IMP_SKILLS_SPECIAL_ELEC IMP_SKILLS_SPECIAL_HAND + 1
#define IMP_SKILLS_SPECIAL_NIGHT IMP_SKILLS_SPECIAL_ELEC + 1
#define IMP_SKILLS_SPECIAL_THROW IMP_SKILLS_SPECIAL_NIGHT + 1
#define IMP_SKILLS_SPECIAL_TEACH IMP_SKILLS_SPECIAL_THROW + 1
#define IMP_SKILLS_SPECIAL_HEAVY IMP_SKILLS_SPECIAL_TEACH + 1
#define IMP_SKILLS_SPECIAL_AUTO IMP_SKILLS_SPECIAL_HEAVY + 1
#define IMP_SKILLS_SPECIAL_STEALTH IMP_SKILLS_SPECIAL_AUTO + 1
#define IMP_SKILLS_SPECIAL_AMBI IMP_SKILLS_SPECIAL_STEALTH + 1
#define IMP_SKILLS_SPECIAL_THIEF IMP_SKILLS_SPECIAL_AMBI + 1
#define IMP_SKILLS_SPECIAL_MARTIAL IMP_SKILLS_SPECIAL_THIEF + 1
#define IMP_SKILLS_SPECIAL_KNIFE IMP_SKILLS_SPECIAL_MARTIAL + 1

#define IMP_RESULTS_PHYSICAL IMP_SKILLS_SPECIAL_KNIFE + 1
#define IMP_RESULTS_PHYSICAL_LENGTH 7

#define IMP_PHYSICAL_SUPER IMP_RESULTS_PHYSICAL + IMP_RESULTS_PHYSICAL_LENGTH
#define IMP_PHYSICAL_SUPER_LENGTH 1

#define IMP_PHYSICAL_SUPER_HEALTH IMP_PHYSICAL_SUPER + IMP_PHYSICAL_SUPER_LENGTH
#define IMP_PHYSICAL_SUPER_AGILITY IMP_PHYSICAL_SUPER_HEALTH + 1
#define IMP_PHYSICAL_SUPER_DEXTERITY IMP_PHYSICAL_SUPER_AGILITY + 1
#define IMP_PHYSICAL_SUPER_STRENGTH IMP_PHYSICAL_SUPER_DEXTERITY + 1
#define IMP_PHYSICAL_SUPER_LEADERSHIP IMP_PHYSICAL_SUPER_STRENGTH + 1
#define IMP_PHYSICAL_SUPER_WISDOM IMP_PHYSICAL_SUPER_LEADERSHIP + 1

#define IMP_PHYSICAL_LOW IMP_PHYSICAL_SUPER_WISDOM + 1
#define IMP_PHYSICAL_LOW_LENGTH 1

#define IMP_PHYSICAL_LOW_HEALTH IMP_PHYSICAL_LOW + IMP_PHYSICAL_LOW_LENGTH
#define IMP_PHYSICAL_LOW_AGILITY IMP_PHYSICAL_LOW_HEALTH + 1
#define IMP_PHYSICAL_LOW_DEXTERITY IMP_PHYSICAL_LOW_AGILITY + 2
#define IMP_PHYSICAL_LOW_STRENGTH IMP_PHYSICAL_LOW_DEXTERITY + 1
#define IMP_PHYSICAL_LOW_LEADERSHIP IMP_PHYSICAL_LOW_STRENGTH + 1
#define IMP_PHYSICAL_LOW_WISDOM IMP_PHYSICAL_LOW_LEADERSHIP + 1


#define IMP_PHYSICAL_VERY_LOW IMP_PHYSICAL_LOW_WISDOM + 1
#define IMP_PHYSICAL_VERY_LOW_LENGTH 1

#define IMP_PHYSICAL_VERY_LOW_HEALTH IMP_PHYSICAL_VERY_LOW + IMP_PHYSICAL_VERY_LOW_LENGTH
#define IMP_PHYSICAL_VERY_LOW_AGILITY IMP_PHYSICAL_VERY_LOW_HEALTH + 1
#define IMP_PHYSICAL_VERY_LOW_DEXTERITY IMP_PHYSICAL_VERY_LOW_AGILITY + 1
#define IMP_PHYSICAL_VERY_LOW_STRENGTH IMP_PHYSICAL_VERY_LOW_DEXTERITY + 1
#define IMP_PHYSICAL_VERY_LOW_LEADERSHIP IMP_PHYSICAL_VERY_LOW_STRENGTH + 1
#define IMP_PHYSICAL_VERY_LOW_WISDOM IMP_PHYSICAL_VERY_LOW_LEADERSHIP + 1


#define IMP_PHYSICAL_END IMP_PHYSICAL_VERY_LOW_WISDOM + 1
#define IMP_PHYSICAL_END_LENGTH 3

#define IMP_RESULTS_PORTRAIT  IMP_PHYSICAL_END + IMP_PHYSICAL_END_LENGTH
#define IMP_RESULTS_PORTRAIT_LENGTH 6


#define IMP_PORTRAIT_MALE_1 IMP_RESULTS_PORTRAIT + IMP_RESULTS_PORTRAIT_LENGTH
#define IMP_PORTRAIT_MALE_2 IMP_PORTRAIT_MALE_1 + 4
#define IMP_PORTRAIT_MALE_3 IMP_PORTRAIT_MALE_2 + 4
#define IMP_PORTRAIT_MALE_4 IMP_PORTRAIT_MALE_3 + 4
#define IMP_PORTRAIT_MALE_5 IMP_PORTRAIT_MALE_4 + 4
#define IMP_PORTRAIT_MALE_6 IMP_PORTRAIT_MALE_5 + 4

#define IMP_PORTRAIT_FEMALE_1 IMP_PORTRAIT_MALE_6 + 4
#define IMP_PORTRAIT_FEMALE_2 IMP_PORTRAIT_FEMALE_1 + 4
#define IMP_PORTRAIT_FEMALE_3 IMP_PORTRAIT_FEMALE_2 + 4
#define IMP_PORTRAIT_FEMALE_4 IMP_PORTRAIT_FEMALE_3 + 4
#define IMP_PORTRAIT_FEMALE_5 IMP_PORTRAIT_FEMALE_4 + 4
#define IMP_PORTRAIT_FEMALE_6 IMP_PORTRAIT_FEMALE_5 + 4



#define IMP_RESULTS_END IMP_PORTRAIT_FEMALE_6 + 1
#define IMP_RESULTS_END_LENGTH 3


enum SkillBits
{
	SKILL_NONE = 0,
	SKILL_MECH = 1 << 0,
	SKILL_MARK = 1 << 1,
	SKILL_MED  = 1 << 2,
	SKILL_EXPL = 1 << 3
};
ENUM_BITSET(SkillBits)


enum PhysicalBits
{
	PHYS_NONE = 0,
	PHYS_HLTH = 1 << 0,
	PHYS_DEX  = 1 << 1,
	PHYS_STR  = 1 << 2,
	PHYS_AGI  = 1 << 3,
	PHYS_WIS  = 1 << 4,
	PHYS_LDR  = 1 << 5
};
ENUM_BITSET(PhysicalBits)


static ST::string LoadIMPResultText(UINT32 Offset)
{
	return GCM->loadEncryptedString(BINARYDATADIR "/impass.edt", MAIL_STRING_SIZE * Offset, MAIL_STRING_SIZE);
}


static void AddIMPResultText(UINT32 Offset)
{
	ST::string Text = LoadIMPResultText(Offset);
	AddEmailRecordToList(Text);
}


static void AddSkillTraitText(MERCPROFILESTRUCT const& imp, SkillTrait const Skill, UINT32 const Offset)
{
	if (HasSkillTrait(imp, Skill)) AddIMPResultText(Offset);
}


static void HandleIMPCharProfileResultsMessage(void)
{
	// special case, IMP profile return
	INT32 iOffSet;
	INT32 iEndOfSection;

	INT32 iRand = Random(32767);

	if (pMessageRecordList != NULL) return;
	// list doesn't exist, reload

	MERCPROFILESTRUCT const& imp = GetProfile(PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId);

	// load intro
	iEndOfSection = IMP_RESULTS_INTRO_LENGTH;
	for (INT32 i = 0; i < iEndOfSection; ++i)
	{
		ST::string pString = LoadIMPResultText(i);

		// have to place players name into string for first record
		if (i == 0)
		{
			ST::string zTemp = ST::format(" {}", imp.zName);
			pString += zTemp;
		}

		AddEmailRecordToList(pString);
	}

	// now the personality intro
	iOffSet = IMP_RESULTS_PERSONALITY_INTRO;
	iEndOfSection = IMP_RESULTS_PERSONALITY_INTRO_LENGTH + 1;
	for (INT32 i = 0; i < iEndOfSection; ++i) AddIMPResultText(iOffSet + i);

	// personality itself
	switch (imp.bPersonalityTrait)
	{
		// normal as can be
		case NO_PERSONALITYTRAIT: iOffSet = IMP_PERSONALITY_NORMAL;          break;
		case HEAT_INTOLERANT:     iOffSet = IMP_PERSONALITY_HEAT;            break;
		case NERVOUS:             iOffSet = IMP_PERSONALITY_NERVOUS;         break;
		case CLAUSTROPHOBIC:      iOffSet = IMP_PERSONALITY_CLAUSTROPHOBIC;  break;
		case NONSWIMMER:          iOffSet = IMP_PERSONALITY_NONSWIMMER;      break;
		case FEAR_OF_INSECTS:     iOffSet = IMP_PERSONALITY_FEAR_OF_INSECTS; break;
		case FORGETFUL:           iOffSet = IMP_PERSONALITY_FORGETFUL;       break;
		case PSYCHO:              iOffSet = IMP_PERSONALITY_PSYCHO;          break;
	}

	// personality tick
	//  DEF: removed 1/12/99, cause it was changing the length of email that were already calculated
	//               AddIMPResultText(iOffSet + Random(IMP_PERSONALITY_LENGTH - 1) + 1);
	AddIMPResultText(iOffSet + 1);

	// persoanlity paragraph
	AddIMPResultText(iOffSet + IMP_PERSONALITY_LENGTH);

	// extra paragraph for bugs
	if (imp.bPersonalityTrait == FEAR_OF_INSECTS)
	{
		// persoanlity paragraph
		AddIMPResultText(iOffSet + IMP_PERSONALITY_LENGTH + 1);
	}

	// attitude intro
	// now the personality intro
	iOffSet = IMP_RESULTS_ATTITUDE_INTRO;
	iEndOfSection = IMP_RESULTS_ATTITUDE_LENGTH;
	for (INT32 i = 0; i < iEndOfSection; ++i) AddIMPResultText(iOffSet + i);

		// personality itself
	switch (imp.bAttitude)
	{
		// normal as can be
		case ATT_NORMAL:     iOffSet = IMP_ATTITUDE_NORMAL;     break;
		case ATT_FRIENDLY:   iOffSet = IMP_ATTITUDE_FRIENDLY;   break;
		case ATT_LONER:      iOffSet = IMP_ATTITUDE_LONER;      break;
		case ATT_OPTIMIST:   iOffSet = IMP_ATTITUDE_OPTIMIST;   break;
		case ATT_PESSIMIST:  iOffSet = IMP_ATTITUDE_PESSIMIST;  break;
		case ATT_AGGRESSIVE: iOffSet = IMP_ATTITUDE_AGGRESSIVE; break;
		case ATT_ARROGANT:   iOffSet = IMP_ATTITUDE_ARROGANT;   break;
		case ATT_ASSHOLE:    iOffSet = IMP_ATTITUDE_ASSHOLE;    break;
		case ATT_COWARD:     iOffSet = IMP_ATTITUDE_COWARD;     break;
	}

	// attitude title
	AddIMPResultText(iOffSet);

	// attitude tick
	//  DEF: removed 1/12/99, cause it was changing the length of email that were already calculated
	//               AddIMPResultText(iOffSet + Random(IMP_ATTITUDE_LENGTH - 2) + 1);
	AddIMPResultText(iOffSet + 1);

	// attitude paragraph
	AddIMPResultText(iOffSet + IMP_ATTITUDE_LENGTH - 1);

	//check for second paragraph
	if (iOffSet != IMP_ATTITUDE_NORMAL)
	{
		// attitude paragraph
		AddIMPResultText(iOffSet + IMP_ATTITUDE_LENGTH);
	}


	// skills
	// now the skills intro
	iOffSet = IMP_RESULTS_SKILLS;
	iEndOfSection = IMP_RESULTS_SKILLS_LENGTH;
	for (INT32 i = 0; i < iEndOfSection; ++i) AddIMPResultText(iOffSet + i);


	SkillBits Skill;

	Skill = SKILL_NONE;
	if (imp.bMarksmanship >= SUPER_SKILL_VALUE) Skill |= SKILL_MARK;
	if (imp.bMedical      >= SUPER_SKILL_VALUE) Skill |= SKILL_MED;
	if (imp.bMechanical   >= SUPER_SKILL_VALUE) Skill |= SKILL_MECH;
	if (imp.bExplosive    >= SUPER_SKILL_VALUE) Skill |= SKILL_EXPL;

	if (Skill != SKILL_NONE) AddIMPResultText(IMP_SKILLS_IMPERIAL_SKILLS);

	if (Skill & SKILL_MARK) AddIMPResultText(IMP_SKILLS_IMPERIAL_MARK);
	if (Skill & SKILL_MED)  AddIMPResultText(IMP_SKILLS_IMPERIAL_MED);
	if (Skill & SKILL_MECH) AddIMPResultText(IMP_SKILLS_IMPERIAL_MECH);
	if (Skill & SKILL_EXPL) AddIMPResultText(IMP_SKILLS_IMPERIAL_EXPL);


	// now the needs training values
	Skill = SKILL_NONE;
	if (imp.bMarksmanship > NO_CHANCE_IN_HELL_SKILL_VALUE && imp.bMarksmanship <= NEEDS_TRAINING_SKILL_VALUE) Skill |= SKILL_MARK;
	if (imp.bMedical      > NO_CHANCE_IN_HELL_SKILL_VALUE && imp.bMedical      <= NEEDS_TRAINING_SKILL_VALUE) Skill |= SKILL_MED;
	if (imp.bMechanical   > NO_CHANCE_IN_HELL_SKILL_VALUE && imp.bMechanical   <= NEEDS_TRAINING_SKILL_VALUE) Skill |= SKILL_MECH;
	if (imp.bExplosive    > NO_CHANCE_IN_HELL_SKILL_VALUE && imp.bExplosive    <= NEEDS_TRAINING_SKILL_VALUE) Skill |= SKILL_EXPL;

	if (Skill != SKILL_NONE) AddIMPResultText(IMP_SKILLS_NEED_TRAIN_SKILLS);

	if (Skill & SKILL_MARK) AddIMPResultText(IMP_SKILLS_NEED_TRAIN_MARK);
	if (Skill & SKILL_MED)  AddIMPResultText(IMP_SKILLS_NEED_TRAIN_MED);
	if (Skill & SKILL_MECH) AddIMPResultText(IMP_SKILLS_NEED_TRAIN_MECH);
	if (Skill & SKILL_EXPL) AddIMPResultText(IMP_SKILLS_NEED_TRAIN_EXPL);


	// and the no chance in hell of doing anything useful values
	Skill = SKILL_NONE;
	if (imp.bMarksmanship <= NO_CHANCE_IN_HELL_SKILL_VALUE) Skill |= SKILL_MARK;
	if (imp.bMedical      <= NO_CHANCE_IN_HELL_SKILL_VALUE) Skill |= SKILL_MED;
	if (imp.bMechanical   <= NO_CHANCE_IN_HELL_SKILL_VALUE) Skill |= SKILL_MECH;
	if (imp.bExplosive    <= NO_CHANCE_IN_HELL_SKILL_VALUE) Skill |= SKILL_EXPL;

	if (Skill != SKILL_NONE) AddIMPResultText(IMP_SKILLS_NO_SKILL);

	if (Skill & SKILL_MECH) AddIMPResultText(IMP_SKILLS_NO_SKILL_MECH);
	if (Skill & SKILL_MARK) AddIMPResultText(IMP_SKILLS_NO_SKILL_MARK);
	if (Skill & SKILL_MED)  AddIMPResultText(IMP_SKILLS_NO_SKILL_MED);
	if (Skill & SKILL_EXPL) AddIMPResultText(IMP_SKILLS_NO_SKILL_EXPL);


	// now the specialized skills
	// imperial skills
	iOffSet = IMP_SKILLS_SPECIAL_INTRO;
	iEndOfSection = IMP_SKILLS_SPECIAL_INTRO_LENGTH;
	for (INT32 i = 0; i < iEndOfSection; ++i) AddIMPResultText(iOffSet + i);

	AddSkillTraitText(imp, KNIFING,     IMP_SKILLS_SPECIAL_KNIFE);
	AddSkillTraitText(imp, LOCKPICKING, IMP_SKILLS_SPECIAL_LOCK);
	AddSkillTraitText(imp, HANDTOHAND,  IMP_SKILLS_SPECIAL_HAND);
	AddSkillTraitText(imp, ELECTRONICS, IMP_SKILLS_SPECIAL_ELEC);
	AddSkillTraitText(imp, NIGHTOPS,    IMP_SKILLS_SPECIAL_NIGHT);
	AddSkillTraitText(imp, THROWING,    IMP_SKILLS_SPECIAL_THROW);
	AddSkillTraitText(imp, TEACHING,    IMP_SKILLS_SPECIAL_TEACH);
	AddSkillTraitText(imp, HEAVY_WEAPS, IMP_SKILLS_SPECIAL_HEAVY);
	AddSkillTraitText(imp, AUTO_WEAPS,  IMP_SKILLS_SPECIAL_AUTO);
	AddSkillTraitText(imp, STEALTHY,    IMP_SKILLS_SPECIAL_STEALTH);
	AddSkillTraitText(imp, AMBIDEXT,    IMP_SKILLS_SPECIAL_AMBI);
	AddSkillTraitText(imp, THIEF,       IMP_SKILLS_SPECIAL_THIEF);
	AddSkillTraitText(imp, MARTIALARTS, IMP_SKILLS_SPECIAL_MARTIAL);


	// now the physical
	// imperial physical
	iOffSet = IMP_RESULTS_PHYSICAL;
	iEndOfSection = IMP_RESULTS_PHYSICAL_LENGTH;
	for (INT32 i = 0; i < iEndOfSection; ++i) AddIMPResultText(iOffSet + i);

	PhysicalBits Phys;

	// super physical
	Phys = PHYS_NONE;
	if (imp.bLife       >= SUPER_STAT_VALUE) Phys |= PHYS_HLTH;
	if (imp.bDexterity  >= SUPER_STAT_VALUE) Phys |= PHYS_DEX;
	if (imp.bAgility    >= SUPER_STAT_VALUE) Phys |= PHYS_AGI;
	if (imp.bStrength   >= SUPER_STAT_VALUE) Phys |= PHYS_STR;
	if (imp.bWisdom     >= SUPER_STAT_VALUE) Phys |= PHYS_WIS;
	if (imp.bLeadership >= SUPER_STAT_VALUE) Phys |= PHYS_LDR;

	if (Phys != PHYS_NONE) AddIMPResultText(IMP_PHYSICAL_SUPER);

	if (Phys & PHYS_HLTH) AddIMPResultText(IMP_PHYSICAL_SUPER_HEALTH);
	if (Phys & PHYS_DEX)  AddIMPResultText(IMP_PHYSICAL_SUPER_DEXTERITY);
	if (Phys & PHYS_STR)  AddIMPResultText(IMP_PHYSICAL_SUPER_STRENGTH);
	if (Phys & PHYS_AGI)  AddIMPResultText(IMP_PHYSICAL_SUPER_AGILITY);
	if (Phys & PHYS_WIS)  AddIMPResultText(IMP_PHYSICAL_SUPER_WISDOM);
	if (Phys & PHYS_LDR)  AddIMPResultText(IMP_PHYSICAL_SUPER_LEADERSHIP);


	// now the low attributes
	Phys = PHYS_NONE;
	if (imp.bLife       < NEEDS_TRAINING_STAT_VALUE && imp.bLife       > NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_HLTH;
	if (imp.bStrength   < NEEDS_TRAINING_STAT_VALUE && imp.bStrength   > NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_STR;
	if (imp.bAgility    < NEEDS_TRAINING_STAT_VALUE && imp.bAgility    > NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_AGI;
	if (imp.bWisdom     < NEEDS_TRAINING_STAT_VALUE && imp.bWisdom     > NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_WIS;
	if (imp.bLeadership < NEEDS_TRAINING_STAT_VALUE && imp.bLeadership > NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_LDR;
	if (imp.bDexterity  < NEEDS_TRAINING_STAT_VALUE && imp.bDexterity  > NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_DEX;

	if (Phys != PHYS_NONE) AddIMPResultText(IMP_PHYSICAL_LOW);

	if (Phys & PHYS_HLTH) AddIMPResultText(IMP_PHYSICAL_LOW_HEALTH);
	if (Phys & PHYS_DEX)  AddIMPResultText(IMP_PHYSICAL_LOW_DEXTERITY);
	if (Phys & PHYS_STR)  AddIMPResultText(IMP_PHYSICAL_LOW_STRENGTH);
	if (Phys & PHYS_AGI)  AddIMPResultText(IMP_PHYSICAL_LOW_AGILITY);
	if (Phys & PHYS_WIS)  AddIMPResultText(IMP_PHYSICAL_LOW_WISDOM);
	if (Phys & PHYS_LDR)  AddIMPResultText(IMP_PHYSICAL_LOW_LEADERSHIP);


	// very low physical
	Phys = PHYS_NONE;
	if (imp.bLife       <= NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_HLTH;
	if (imp.bDexterity  <= NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_DEX;
	if (imp.bStrength   <= NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_STR;
	if (imp.bAgility    <= NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_AGI;
	if (imp.bWisdom     <= NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_WIS;
	if (imp.bLeadership <= NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_LDR;

	if (Phys != PHYS_NONE) AddIMPResultText(IMP_PHYSICAL_VERY_LOW);

	if (Phys & PHYS_HLTH) AddIMPResultText(IMP_PHYSICAL_VERY_LOW_HEALTH);
	if (Phys & PHYS_DEX)  AddIMPResultText(IMP_PHYSICAL_VERY_LOW_DEXTERITY);
	if (Phys & PHYS_STR)  AddIMPResultText(IMP_PHYSICAL_VERY_LOW_STRENGTH);
	if (Phys & PHYS_AGI)  AddIMPResultText(IMP_PHYSICAL_VERY_LOW_AGILITY);
	if (Phys & PHYS_WIS)  AddIMPResultText(IMP_PHYSICAL_VERY_LOW_WISDOM);
	if (Phys & PHYS_LDR) AddIMPResultText(IMP_PHYSICAL_VERY_LOW_LEADERSHIP);


	iOffSet = IMP_RESULTS_PORTRAIT;
	iEndOfSection = IMP_RESULTS_PORTRAIT_LENGTH;
	for (INT32 i = 0; i < iEndOfSection; ++i) AddIMPResultText(iOffSet + i);

	switch (iPortraitNumber)
	{
		case  0: iOffSet = IMP_PORTRAIT_MALE_1;   break;
		case  1: iOffSet = IMP_PORTRAIT_MALE_2;   break;
		case  2: iOffSet = IMP_PORTRAIT_MALE_3;   break;
		case  3: iOffSet = IMP_PORTRAIT_MALE_4;   break;
		case  4:
		case  5: iOffSet = IMP_PORTRAIT_MALE_5;   break;
		case  6:
		case  7: iOffSet = IMP_PORTRAIT_MALE_6;   break;
		case  8: iOffSet = IMP_PORTRAIT_FEMALE_1; break;
		case  9: iOffSet = IMP_PORTRAIT_FEMALE_2; break;
		case 10: iOffSet = IMP_PORTRAIT_FEMALE_3; break;
		case 11:
		case 12: iOffSet = IMP_PORTRAIT_FEMALE_4; break;
		case 13:
		case 14: iOffSet = IMP_PORTRAIT_FEMALE_5; break;
	}

	if (iRand % 2 == 0) iOffSet += 2;

	iEndOfSection = 2;
	for (INT32 i = 0; i < iEndOfSection; ++i) AddIMPResultText(iOffSet + i);

	iOffSet = IMP_RESULTS_END;
	iEndOfSection = IMP_RESULTS_END_LENGTH;
	for (INT32 i = 0; i < iEndOfSection; ++i) AddIMPResultText(iOffSet + i);

	PreviousMail = CurrentMail;
}


static void HandleEmailViewerButtonStates(void)
{
	// handle state of email viewer buttons

	// leave, if not displaying message
	if (!fDisplayMessageFlag) return;

	if(  giNumberOfPagesToCurrentEmail <= 2 )
	{
		return;
	}

	// Turn on/off previous page button
	EnableButton(giMailMessageButtons[0], giMessagePage != 0);
	// Turn on/off next page button
	EnableButton(giMailMessageButtons[1], pEmailPageInfo[giMessagePage + 1].pFirstRecord);
}


static void CreateNextPreviousEmailPageButtons(void)
{
	// this function will create the buttons to advance and go back email pages
	giMailPageButtons[0] = MakeButtonNewMail(1, NEXT_PAGE_X,     NEXT_PAGE_Y, NextRegionButtonCallback);
	giMailPageButtons[1] = MakeButtonNewMail(0, PREVIOUS_PAGE_X, NEXT_PAGE_Y, PreviousRegionButtonCallback);
}


static void UpdateStatusOfNextPreviousButtons(void)
{
	// set the states of the page advance buttons
	EnableButton(giMailPageButtons[0], iCurrentPage < iLastPage);
	EnableButton(giMailPageButtons[1], iCurrentPage > 0);
}


static void DisplayWhichPageOfEmailProgramIsDisplayed(void)
{
	// will draw the number of the email program we are viewing right now
	SetFontAttributes(MESSAGE_FONT, FONT_BLACK, NO_SHADOW);

	// page number
	INT32 CPage;
	INT32 LPage;
	if (iLastPage < 0)
	{
		CPage = 1;
		LPage = 1;
	}
	else
	{
		CPage = iCurrentPage + 1;
		LPage = iLastPage + 1;
	}
	MPrint(PAGE_NUMBER_X, PAGE_NUMBER_Y, ST::format("{} / {}", CPage, LPage));

	// restore shadow
	SetFontShadow( DEFAULT_SHADOW );
}


static void OpenMostRecentUnreadEmail(void)
{
	// will open the most recent email the player has recieved and not read
	Email* MostRecentMail = NULL;
	Email* pB = pEmailList;
	UINT32 iLowestDate = 9999999;

	while( pB )
	{
		// if date is lesser and unread , swap
		if (pB->iDate < iLowestDate && !pB->fRead)
		{
			MostRecentMail = pB;
			iLowestDate = pB -> iDate;
		}

		// next in B's list
		pB=pB->Next;
	}

	CurrentMail = MostRecentMail;

	// valid message, show it
	if (MostRecentMail != NULL) fDisplayMessageFlag = TRUE;
}


static void DisplayNumberOfPagesToThisEmail(INT32 const iViewerY)
{
	// display the indent for the display of pages to this email..along with the current page/number of pages
	SetFontAttributes(FONT12ARIAL, FONT_BLACK, NO_SHADOW);

	MPrint(VIEWER_X + INDENT_X_OFFSET, VIEWER_Y + iViewerY + INDENT_Y_OFFSET - 2,
		ST::format("{} / {}", giMessagePage + 1, giNumberOfPagesToCurrentEmail - 1),
		CenterAlign(INDENT_X_WIDTH));

	SetFontShadow(DEFAULT_SHADOW);
}


static INT32 GetNumberOfPagesToEmail(void)
{
	Record* pTempRecord;
	INT32 iNumberOfPagesToEmail = 0;


	// set temp record to head of list
	pTempRecord=pMessageRecordList;

	// run through messages, and find out how many
	while( pTempRecord )
	{
		pTempRecord = GetFirstRecordOnThisPage(pMessageRecordList, iNumberOfPagesToEmail);
		iNumberOfPagesToEmail++;
	}


	return( iNumberOfPagesToEmail );
}


void ShutDownEmailList()
{
	// Loop through all the emails to delete them
	Email* i = pEmailList;
	pEmailList = 0;
	while (i)
	{
		Email* const del = i;
		i = i->Next;
		delete del;
	}
	ClearPages();
}


static void PreProcessEmail(Email* const m)
{
	// already processed?
	if (pEmailPageInfo[0].pFirstRecord) return;

	if (!pMessageRecordList)
	{ // List doesn't exist, reload
		INT32 const offset = m->usOffset;
		for (INT32 i = 0; i != m->usLength; ++i)
		{
			// read one record from email file
			ST::string str = LoadEMailText(offset + i);
			AddEmailRecordToList(str);
		}
		PreviousMail = CurrentMail;
	}

	Record* start = pMessageRecordList;
	if (start && m->usOffset != IMP_EMAIL_PROFILE_RESULTS)
	{ // pass the subject line
		start = start->Next;
	}

	// get number of pages to this email
	giNumberOfPagesToCurrentEmail = GetNumberOfPagesToEmail();

	INT32 h = 0;
	for (Record const* i = start; i; i = i->Next)
	{
		// get the height of the string, ONLY!...must redisplay ON TOP OF background graphic
		h += IanWrappedStringHeight(MESSAGE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, i->pRecord);
	}

	// set iViewerY so to center the viewer
	iViewerPositionY = (LAPTOP_SCREEN_LR_Y - 2 * VIEWER_Y - 2 * VIEWER_MESSAGE_BODY_START_Y  - h) / 2;
	if (iViewerPositionY < 0) iViewerPositionY = 0;

	UINT16 const line_h = GetFontHeight(MESSAGE_FONT);
	if (h < line_h * MIN_MESSAGE_HEIGHT_IN_LINES)
	{
		// Use minimum height
		h = line_h * MIN_MESSAGE_HEIGHT_IN_LINES;
	}
	else if (h > MAX_EMAIL_MESSAGE_PAGE_SIZE)
	{
		// Message to big to fit on page
		h = MAX_EMAIL_MESSAGE_PAGE_SIZE;
	}

	// set total height to height of records displayed
	iTotalHeight = h + 10;

	INT32 page = 0;
	if (iTotalHeight < MAX_EMAIL_MESSAGE_PAGE_SIZE)
	{
		EmailPageInfoStruct& info = pEmailPageInfo[page];
		info.pFirstRecord = start;
		info.iPageNumber  = 0;

		Record* last_record = 0;
		for (Record* i = start; i; i = i->Next)
		{
			last_record = i;
		}

		// only one record to this email?..then set next to null
		info.pLastRecord = last_record == info.pFirstRecord ? 0 : last_record;
		++page;
	}
	else
	{
		// more than one page
		for (Record* i; (i = GetFirstRecordOnThisPage(start, page)); ++page)
		{
			EmailPageInfoStruct& info = pEmailPageInfo[page];
			info.pFirstRecord = i;
			info.iPageNumber  = page;

			// go to the right record
			Record* last_record = 0;
			INT32   y           = 0;
			for (; i; i = last_record = i->Next)
			{
				UINT16 const h = IanWrappedStringHeight(MESSAGE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, i->pRecord);
				// Gonna get cut off?  End now
				if (y + h > MAX_EMAIL_MESSAGE_PAGE_SIZE) break;
				y += h;
			}

			info.pLastRecord = last_record == info.pFirstRecord ? 0 : last_record;
		}
	}

	EmailPageInfoStruct& info = pEmailPageInfo[page];
	info.pFirstRecord = 0;
	info.pLastRecord  = 0;
	info.iPageNumber  = page;
}


static void ModifyInsuranceEmails(UINT16 usMessageId, Email* pMail, UINT8 ubNumberOfRecords)
{
	UINT8	ubCnt;

	for( ubCnt=0; ubCnt<ubNumberOfRecords; ubCnt++)
	{
		// read one record from email file
		ST::string pString = LoadEMailText(usMessageId);

		//Replace the $MERCNAME$ and $AMOUNT$ with the mercs name and the amountm if the string contains the keywords.
		pString = ReplaceMercNameAndAmountWithProperData( pString, pMail );

		// add to list
		AddEmailRecordToList( pString );

		usMessageId++;
	}

	PreviousMail = CurrentMail;
}


static ST::string ReplaceMercNameAndAmountWithProperData(const ST::string& pFinishedString, const Email* pMail)
{
	const ST::string sMercName = "$MERCNAME$"; //Doesnt need to be translated, inside Email.txt and will be replaced by the mercs name
	const ST::string sAmount = "$AMOUN$"; //Doesnt need to be translated, inside Email.txt and will be replaced by a dollar amount

	ST::string mercName = gMercProfiles[ pMail->uiSecondData ].zName;
	ST::string amount = SPrintMoney(pMail->iFirstData);
	return pFinishedString.replace(sAmount, amount).replace(sMercName, mercName);
}

