#include "Font.h"
#include "Laptop.h"
#include "EMail.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Debug.h"
#include "WordWrap.h"
#include "Render_Dirty.h"
#include "Encrypted_File.h"
#include "Cursors.h"
#include "Soldier_Profile.h"
#include "IMP_Compile_Character.h"
#include "IMP_Portraits.h"
#include "Game_Clock.h"
#include "Environment.h"
#include "AIMMembers.h"
#include "Random.h"
#include "Text.h"
#include "LaptopSave.h"
#include "Finances.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "MemMan.h"
#include "Font_Control.h"


#define MAX_MESSAGES_PAGE 18 // max number of messages per page

#define VIEWER_X 155
#define VIEWER_Y 70 + 21
#define MAIL_STRING_SIZE 640


typedef enum EMailSortCriteria
{
	SENDER,
	RECEIVED,
	SUBJECT,
	READ
} EMailSortCriteria;


typedef struct Page Page;
struct Page
{
	Email* Mail[MAX_MESSAGES_PAGE];
	Page* Next;
};


typedef struct Record Record;
struct Record
{
	wchar_t pRecord[640];
	Record* Next;
};


typedef struct EmailPageInfoStruct
{
	Record* pFirstRecord;
	Record* pLastRecord;
	INT32 iPageNumber;
} EmailPageInfoStruct;


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
static BOOLEAN fOnLastPageFlag = FALSE;
BOOLEAN fOpenMostRecentUnReadFlag = FALSE;
static INT32 iViewerPositionY = 0;

static Email* CurrentMail;
static Email* PreviousMail;
static INT32 giMessagePage = -1;
static INT32 giNumberOfPagesToCurrentEmail = -1;
UINT32 guiEmailWarning;

#define EMAIL_TOP_BAR_HEIGHT 22

#define MIDDLE_X 0+LAPTOP_SCREEN_UL_X
#define MIDDLE_Y 72 + EMAIL_TOP_BAR_HEIGHT
#define MIDDLE_WIDTH 19


// new graphics
#define EMAIL_LIST_WINDOW_Y 22

// email columns
#define SENDER_X LAPTOP_SCREEN_UL_X+65

#define DATE_X LAPTOP_SCREEN_UL_X+428

#define SUBJECT_X LAPTOP_SCREEN_UL_X+175
#define SUBJECT_WIDTH					254	//526-245
#define INDIC_X 128

#define LINE_WIDTH 592-121

#define MESSAGE_X 5	//17
#define MESSAGE_WIDTH 528-125//150
#define MESSAGE_COLOR FONT_BLACK
#define MESSAGE_GAP 2


#define MESSAGE_HEADER_WIDTH 209-151
#define MESSAGE_HEADER_X VIEWER_X+4


#define MAX_BUTTON_COUNT 1


#define EMAIL_WARNING_X 210
#define EMAIL_WARNING_Y 140
#define EMAIL_WARNING_WIDTH 254
#define EMAIL_WARNING_HEIGHT 138


#define NEW_BTN_X EMAIL_WARNING_X +(338-245)
#define NEW_BTN_Y EMAIL_WARNING_Y +(278-195)

#define EMAIL_TEXT_FONT				FONT10ARIAL
#define MESSAGE_FONT					EMAIL_TEXT_FONT
#define EMAIL_HEADER_FONT			FONT14ARIAL
#define EMAIL_WARNING_FONT		FONT12ARIAL


// the max number of pages to an email
#define MAX_NUMBER_EMAIL_PAGES 100

#define NEXT_PAGE_X LAPTOP_UL_X + 562
#define NEXT_PAGE_Y 51

#define PREVIOUS_PAGE_X NEXT_PAGE_X - 21

#define ENVELOPE_BOX_X 116

#define FROM_BOX_X 166

#define SUBJECT_BOX_X 276

#define DATE_BOX_X 530

#define FROM_BOX_Y 51 + EMAIL_TOP_BAR_HEIGHT

#define EMAIL_TITLE_FONT FONT14ARIAL
#define EMAIL_TITLE_X 140
#define EMAIL_TITLE_Y 33
#define VIEWER_MESSAGE_BODY_START_Y VIEWER_Y+72
#define MIN_MESSAGE_HEIGHT_IN_LINES 5


#define INDENT_Y_OFFSET 310
#define INDENT_X_OFFSET 325
#define INDENT_X_WIDTH ( 544 - 481 )

// the position of the page number being displayed in the email program
#define PAGE_NUMBER_X 516
#define PAGE_NUMBER_Y 58

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
static INT32 giMessageButton[MAX_BUTTON_COUNT];
static INT32 giMessageButtonImage[MAX_BUTTON_COUNT];
static INT32 giDeleteMailButton[2];
static INT32 giDeleteMailButtonImage[2];
static INT32 giSortButton[4];
static INT32 giSortButtonImage[4];
static INT32 giNewMailButton[1];
static INT32 giNewMailButtonImage[1];
static INT32 giMailMessageButtons[3];
static INT32 giMailMessageButtonsImage[3];
static INT32 giMailPageButtons[2];
static INT32 giMailPageButtonsImage[2];


// the message record list, for the currently displayed message
static Record* pMessageRecordList = NULL;

// video handles
static UINT32 guiEmailTitle;
static UINT32 guiEmailBackground;
static UINT32 guiEmailIndicator;
static UINT32 guiEmailMessage;
static UINT32 guiMAILDIVIDER;


// the enumeration of headers
enum{
	FROM_HEADER=0,
	SUBJECT_HEADER,
	RECD_HEADER,
};


// current line in the email list that is highlighted, -1 is no line highlighted
static INT32 iHighLightLine = -1;

// whther or not we need to redraw the new mail box
BOOLEAN fReDrawNewMailFlag = FALSE;
static INT32 iTotalHeight = 0;


static void CreateDestroyNextPreviousRegions(void);
static void EmailBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void EmailMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);


static void InitializeMouseRegions(void)
{
	INT32 iCounter=0;

	// init mouseregions
	for(iCounter=0; iCounter <MAX_MESSAGES_PAGE; iCounter++)
	{
	 MSYS_DefineRegion(&pEmailRegions[iCounter],MIDDLE_X ,((INT16)(MIDDLE_Y+iCounter*MIDDLE_WIDTH)), MIDDLE_X+LINE_WIDTH ,(INT16)(MIDDLE_Y+iCounter*MIDDLE_WIDTH+MIDDLE_WIDTH),
			MSYS_PRIORITY_NORMAL+2,MSYS_NO_CURSOR, EmailMvtCallBack, EmailBtnCallBack );
		MSYS_SetRegionUserData(&pEmailRegions[iCounter],0,iCounter);
	}

	CreateDestroyNextPreviousRegions();
}


static void DeleteEmailMouseRegions(void)
{

	// this function will remove the mouse regions added
	INT32 iCounter=0;


	for(iCounter=0; iCounter <MAX_MESSAGES_PAGE; iCounter++)
	{
	 MSYS_RemoveRegion( &pEmailRegions[iCounter]);
	}
  CreateDestroyNextPreviousRegions();
}


void GameInitEmail()
{
  pEmailList=NULL;
	pPageList=NULL;

	iLastPage=-1;

	iCurrentPage=0;
	MailToDelete = NULL;

	// reset display message flag
	fDisplayMessageFlag=FALSE;

	 // reset page being displayed
   giMessagePage = 0;
}


static void CreateMailScreenButtons(void);


BOOLEAN EnterEmail()
{
  // load graphics

	iCurrentPage = LaptopSaveInfo.iCurrentEmailPage;

	// title bar
	guiEmailTitle = AddVideoObjectFromFile("LAPTOP/programtitlebar.sti");
	CHECKF(guiEmailTitle != NO_VOBJECT);

	// the list background
	guiEmailBackground = AddVideoObjectFromFile("LAPTOP/Mailwindow.sti");
	CHECKF(guiEmailBackground != NO_VOBJECT);

	// the indication/notification box
	guiEmailIndicator = AddVideoObjectFromFile("LAPTOP/MailIndicator.sti");
	CHECKF(guiEmailIndicator != NO_VOBJECT);

	// the message background
	guiEmailMessage = AddVideoObjectFromFile("LAPTOP/emailviewer.sti");
	CHECKF(guiEmailMessage != NO_VOBJECT);

  // the message background
	guiMAILDIVIDER = AddVideoObjectFromFile("LAPTOP/maillistdivider.sti");
	CHECKF(guiMAILDIVIDER != NO_VOBJECT);

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

	return( TRUE );
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
	DeleteVideoObjectFromIndex(guiEmailTitle);
  DeleteVideoObjectFromIndex(guiEmailBackground);
  DeleteVideoObjectFromIndex(guiMAILDIVIDER);
  DeleteVideoObjectFromIndex(guiEmailIndicator);
  DeleteVideoObjectFromIndex(guiEmailMessage);


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
	if( ( fDisplayMessageFlag == FALSE ) && ( fOldDisplayMessageFlag ) )
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

	if( fOpenMostRecentUnReadFlag == TRUE )
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
  BltVideoObjectFromIndex(FRAME_BUFFER, guiEmailBackground, 0, LAPTOP_SCREEN_UL_X, EMAIL_LIST_WINDOW_Y + LAPTOP_SCREEN_UL_Y);
  BltVideoObjectFromIndex(FRAME_BUFFER, guiEmailTitle,      0, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y - 2);

	// show text on titlebar
	DisplayTextOnTitleBar( );

  DisplayEmailList( );

  // redraw line dividers
  DrawLineDividers( );

	BltVideoObjectFromIndex(FRAME_BUFFER, guiLaptopBACKGROUND, 0, 108, 23);

	ReDisplayBoxes( );

	BlitTitleBarIcons(  );

	// show which page we are on
	DisplayWhichPageOfEmailProgramIsDisplayed( );

	InvalidateRegion(0,0,640,480);
	 // invalidate region to force update
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


static void AddMessageToPages(Email* Mail);
static BOOLEAN ReplaceMercNameAndAmountWithProperData(wchar_t* pFinishedString, const Email* pMail);


void AddEmailMessage(INT32 iMessageOffset, INT32 iMessageLength, INT32 iDate, UINT8 ubSender, BOOLEAN fAlreadyRead, INT32 iFirstData, UINT32 uiSecondData)
{
	// will add a message to the list of messages
	Email* pTempEmail = NULL;

	// add new element onto list
  pTempEmail=MemAlloc(sizeof(Email));

	// copy offset and length of the actual message in email.edt
	pTempEmail->usOffset =(UINT16)iMessageOffset;
	pTempEmail->usLength =(UINT16)iMessageLength;

	// copy date and sender id's
	pTempEmail->iDate=iDate;
  pTempEmail->ubSender=ubSender;

	// the special data
	pTempEmail->iFirstData = iFirstData;
  pTempEmail->uiSecondData = uiSecondData;

	wchar_t pSubject[320];
	LoadEncryptedDataFromFile("BINARYDATA/Email.edt", pSubject, 640 * iMessageOffset, 640);
	ReplaceMercNameAndAmountWithProperData(pSubject, pTempEmail);
	wcscpy(pTempEmail->pSubject, pSubject); // XXX potential buffer overflow

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
	MemFree(Mail);
}


static void AddEmailPage(void)
{
	// simple adds a page to the list
	Page* pPage = pPageList;
	if(pPage)
	{
	 while(pPage->Next)
		 pPage=pPage->Next;
	}


	if(pPage)
	{

		// there is a page, add current page after it
		pPage->Next=MemAlloc(sizeof(Page));
    pPage=pPage->Next;
		pPage->Next=NULL;
	}
	else
	{

		// page becomes head of page list
		pPageList=MemAlloc(sizeof(Page));
		pPage=pPageList;
		pPage->Next=NULL;
    pPageList=pPage;
	}
	for (size_t i = 0; i < lengthof(pPage->Mail); ++i) pPage->Mail[i] = NULL;
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
			INT Order;
			switch (Criterium)
			{
				case RECEIVED:
					Order = Mail->iDate - Other->iDate;
					if (fSortDateUpwards) Order = -Order;
					break;

				case SENDER:
					Order = wcscmp(pSenderNameList[Mail->ubSender], pSenderNameList[Other->ubSender]);
					if (fSortSenderUpwards) Order = -Order;
					break;

				case SUBJECT:
					Order = wcscmp(Mail->pSubject, Other->pSubject);
					if (fSortSubjectUpwards) Order = -Order;
					break;

				case READ:
					Order = Other->fRead - Mail->fRead;
					break;
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
	Page* pPage = pPageList;

	// error check
	if( pPageList == NULL )
	{
		return;
	}

	while(pPage->Next)
	{
		Page* Next = pPage->Next;
		MemFree(pPage);
		pPage = Next;
	}
	if(pPage)
   MemFree(pPage);
	pPageList=NULL;
	iLastPage=-1;
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


static void DrawLetterIcon(INT32 iCounter, BOOLEAN fRead)
{
  // will draw the icon for letter in mail list depending if the mail has been read or not
	BltVideoObjectFromIndex(FRAME_BUFFER, guiEmailIndicator, fRead ? 0 : 1, INDIC_X, MIDDLE_Y + iCounter * MIDDLE_WIDTH + 2);
}


static void DrawSubject(INT32 iCounter, STR16 pSubject, BOOLEAN fRead)
{
	wchar_t pTempSubject[320];


	// draw subject line of mail being viewed in viewer

	// lock buffer to prevent overwrite
	SetFontDestBuffer(FRAME_BUFFER, SUBJECT_X , MIDDLE_Y + iCounter * MIDDLE_WIDTH, SUBJECT_X + SUBJECT_WIDTH , MIDDLE_Y + iCounter * MIDDLE_WIDTH + MIDDLE_WIDTH);
	SetFontShadow(NO_SHADOW);
	SetFontForeground( FONT_BLACK );
	SetFontBackground( FONT_BLACK );


	wcscpy( pTempSubject, pSubject );

	UINT32 Font = fRead ? MESSAGE_FONT : FONT10ARIALBOLD;
	ReduceStringLength(pTempSubject, lengthof(pTempSubject), SUBJECT_WIDTH - 10, Font);
	IanDisplayWrappedString(SUBJECT_X, 4 + MIDDLE_Y + iCounter * MIDDLE_WIDTH, SUBJECT_WIDTH, MESSAGE_GAP, Font, MESSAGE_COLOR, pTempSubject, 0, LEFT_JUSTIFIED);

	SetFontShadow(DEFAULT_SHADOW);
	// reset font dest buffer
	SetFontDestBuffer(FRAME_BUFFER, 0, 0, 640, 480);
}


static void DrawSender(INT32 iCounter, UINT8 ubSender, BOOLEAN fRead)
{

	// draw name of sender in mail viewer
	SetFontShadow(NO_SHADOW);

	SetFontShadow(NO_SHADOW);
  SetFontForeground( FONT_BLACK );
	SetFontBackground( FONT_BLACK );

	if( fRead )
	{
		SetFont( MESSAGE_FONT );
	}
	else
	{
		SetFont( FONT10ARIALBOLD );
	}

  mprintf(SENDER_X,(( UINT16 )( 4 + MIDDLE_Y + iCounter * MIDDLE_WIDTH ) ) ,pSenderNameList[ubSender]);

	SetFont( MESSAGE_FONT );
	SetFontShadow(DEFAULT_SHADOW);
}


static void DrawDate(INT32 iCounter, INT32 iDate, BOOLEAN fRead)
{
	SetFontShadow(NO_SHADOW);
  SetFontForeground( FONT_BLACK );
	SetFontBackground( FONT_BLACK );

	if( fRead )
	{
		SetFont( MESSAGE_FONT );
	}
	else
	{
		SetFont( FONT10ARIALBOLD );
	}
	// draw date of message being displayed in mail viewer
	mprintf(DATE_X, 4 + MIDDLE_Y + iCounter * MIDDLE_WIDTH, L"%ls %d", pDayStrings[0], iDate / (24 * 60));

	SetFont( MESSAGE_FONT );
	SetFontShadow(DEFAULT_SHADOW);
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
	INT32 iCounter=0;
	// look at current page, and display
	Email* pEmail = NULL;

	// if current page ever ends up negative, reset to 0
	if(iCurrentPage==-1)
		iCurrentPage=0;

	Page* pPage = GetCurrentPage();
	if (pPage == NULL) return;

	// now we have current page, display it
	pEmail = pPage->Mail[iCounter];
	SetFontShadow(NO_SHADOW);
	SetFont(EMAIL_TEXT_FONT);


	// draw each line of the list for this page
	while(pEmail)
	{

		// highlighted message, set text of message in list to blue
		if(iCounter==iHighLightLine)
		{
			SetFontForeground(FONT_BLUE);
		}
		else if(pEmail->fRead)
		{
			// message has been read, reset color to black
      SetFontForeground(FONT_BLACK);
	    //SetFontBackground(FONT_BLACK);

		}
		else
		{
      // defualt, message is not read, set font red
      SetFontForeground(FONT_RED);
	    //SetFontBackground(FONT_BLACK);
		}
    SetFontBackground(FONT_BLACK);

		//draw the icon, sender, date, subject
		DrawLetterIcon(iCounter,pEmail->fRead );
		DrawSubject(iCounter, pEmail->pSubject, pEmail->fRead );
		DrawSender(iCounter, pEmail->ubSender, pEmail->fRead );
		DrawDate(iCounter, pEmail->iDate, pEmail->fRead );

		iCounter++;

		// too many messages onthis page, reset pEmail, so no more are drawn
    if(iCounter >=MAX_MESSAGES_PAGE)
     pEmail=NULL;
		else
			pEmail = pPage->Mail[iCounter];
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


static void EmailBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
 INT32 iCount;
 if(fDisplayMessageFlag)
	 return;
 if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
 {
		Page* pPage = GetCurrentPage();
		if (pPage == NULL) return;

	 // error check
	 iCount=MSYS_GetRegionUserData(pRegion, 0);

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
 else if(iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
 {
		Page* pPage = GetCurrentPage();
		if (pPage == NULL)
		{
			HandleRightButtonUpEvent();
			return;
		}

   iCount=MSYS_GetRegionUserData(pRegion, 0);

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
}


static void EmailMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
  if(fDisplayMessageFlag)
		return;
	if (iReason == MSYS_CALLBACK_REASON_MOVE)
	{

		// set highlight to current regions data, this is the message to display
	 iHighLightLine=MSYS_GetRegionUserData(pRegion, 0);
	}
  if (iReason == MSYS_CALLBACK_REASON_LOST_MOUSE )
	{

		// reset highlight line to invalid message
    iHighLightLine=-1;
	}
}


static void BtnMessageXCallback(GUI_BUTTON *btn, INT32 reason)
{
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP || reason & MSYS_CALLBACK_REASON_RBUTTON_UP)
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


static Record* GetFirstRecordOnThisPage(Record* RecordList, UINT32 uiFont, UINT16 usWidth, UINT8 ubGap, INT32 iPage, INT32 iPageSize)
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
	while( iCurrentPage < iPage )
	{
		// build record list to this point
		while (iCurrentPositionOnThisPage + IanWrappedStringHeight(usWidth, ubGap, uiFont, CurrentRecord->pRecord) <= iPageSize)
		{

			// still room on this page
			iCurrentPositionOnThisPage += IanWrappedStringHeight(usWidth, ubGap, uiFont, CurrentRecord->pRecord);

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
static BOOLEAN DisplayNumberOfPagesToThisEmail(INT32 iViewerY);
static void DrawEmailMessageDisplayTitleText(INT32 iViewerY);
static void HandleAnySpecialEmailMessageEvents(INT32 iMessageId);
static void HandleMailSpecialMessages(UINT16 usMessageId, Email* pMail);
static void PreProcessEmail(Email* pMail);


static INT32 DisplayEmailMessage(Email* pMail)
{
	INT32 iCounter=1;
	INT32 iOffSet=0;
	Record* pTempRecord;
	BOOLEAN fDonePrintingMessage = FALSE;



	if(!pMail)
		return 0;

  iOffSet=(INT32)pMail->usOffset;

  // reset redraw email message flag
	fReDrawMessageFlag = FALSE;

	// we KNOW the player is going to "read" this, so mark it as so
	pMail->fRead=TRUE;

	// is there any special event meant for this mail?..if so, handle it
	HandleAnySpecialEmailMessageEvents( iOffSet );

	HandleMailSpecialMessages(iOffSet, pMail);

	PreProcessEmail( pMail );


  pTempRecord = pMessageRecordList;



  // blt in top line of message as a blank graphic
	// get a handle to the bitmap of EMAIL VIEWER Background
	HVOBJECT hHandle = GetVideoObject(guiEmailMessage);

	// place the graphic on the frame buffer
	BltVideoObject( FRAME_BUFFER, hHandle, 1,VIEWER_X, VIEWER_MESSAGE_BODY_START_Y + iViewerPositionY);
  BltVideoObject( FRAME_BUFFER, hHandle, 1,VIEWER_X, VIEWER_MESSAGE_BODY_START_Y + GetFontHeight( MESSAGE_FONT ) + iViewerPositionY);

	// set shadow
	SetFontShadow(NO_SHADOW);

	BltVideoObjectFromIndex(FRAME_BUFFER, guiEmailMessage,  0, VIEWER_X,     VIEWER_Y + iViewerPositionY);
	BltVideoObjectFromIndex(FRAME_BUFFER, guiTITLEBARICONS, 0, VIEWER_X + 5, VIEWER_Y + iViewerPositionY + 2);

	// display header text
  DisplayEmailMessageSubjectDateFromLines( pMail, iViewerPositionY );

	// display title text
	DrawEmailMessageDisplayTitleText( iViewerPositionY );



  iCounter=0;
  // now blit the text background based on height
	for (iCounter=2; iCounter < ( ( iTotalHeight ) / ( GetFontHeight( MESSAGE_FONT ) ) ); iCounter++ )
	{
	  BltVideoObjectFromIndex(FRAME_BUFFER, guiEmailMessage, 1, VIEWER_X, iViewerPositionY + VIEWER_MESSAGE_BODY_START_Y + GetFontHeight(MESSAGE_FONT) * iCounter);
	}


	// now the bottom piece to the message viewer
	BltVideoObjectFromIndex(FRAME_BUFFER, guiEmailMessage, giNumberOfPagesToCurrentEmail <= 2 ? 2 : 3, VIEWER_X, iViewerPositionY + VIEWER_MESSAGE_BODY_START_Y + GetFontHeight(MESSAGE_FONT) * iCounter);

	// reset iCounter and iHeight
	iCounter = 1;
	INT32 iHeight = GetFontHeight(MESSAGE_FONT);

  // draw body of text. Any particular email can encompass more than one "record" in the
	// email file. Draw each record (length is number of records)

	// now place the text

	// reset ptemprecord to head of list
	pTempRecord = pMessageRecordList;
  // reset shadow
	SetFontShadow( NO_SHADOW );

	pTempRecord = pEmailPageInfo[ giMessagePage ].pFirstRecord;

	if( pTempRecord )
	{
		while( fDonePrintingMessage == FALSE )
		{
			// get the height of the string, ONLY!...must redisplay ON TOP OF background graphic
			iHeight += IanDisplayWrappedString(VIEWER_X + MESSAGE_X + 4, VIEWER_MESSAGE_BODY_START_Y + iHeight + iViewerPositionY, MESSAGE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, MESSAGE_COLOR, pTempRecord->pRecord, 0, IAN_WRAP_NO_SHADOW);

			// increment email record ptr
			pTempRecord = pTempRecord -> Next;



			if( pTempRecord == NULL )
			{
				fDonePrintingMessage = TRUE;
			}
			else if( ( pTempRecord == pEmailPageInfo[ giMessagePage ].pLastRecord ) && (  pEmailPageInfo[ giMessagePage + 1 ].pFirstRecord != NULL ) )
			{
				fDonePrintingMessage = TRUE;
			}
		}
	}

	// show number of pages to this email
	DisplayNumberOfPagesToThisEmail( iViewerPositionY );

	// mark this area dirty
	InvalidateRegion( LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_LR_Y );


	// reset shadow
	SetFontShadow( DEFAULT_SHADOW );


	return iViewerPositionY;
}


static void BtnNewOkback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fNewMailFlag=FALSE;
	}
}


static void BtnDeleteCallback(GUI_BUTTON* btn, INT32 iReason);
static void BtnNextEmailPageCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnPreviousEmailPageCallback(GUI_BUTTON* btn, INT32 reason);


static void AddDeleteRegionsToMessageRegion(INT32 iViewerY)
{
	// will create/destroy mouse region for message display

	if((fDisplayMessageFlag)&&(!fOldDisplayMessageFlag))
	{

		// set old flag
		fOldDisplayMessageFlag=TRUE;


		// add X button
    giMessageButtonImage[0]=  LoadButtonImage( "LAPTOP/X.sti" ,-1,0,-1,1,-1 );
	  giMessageButton[0] = QuickCreateButton( giMessageButtonImage[0], BUTTON_X + 2,(INT16) ( BUTTON_Y + ( INT16 )iViewerY + 1),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnMessageXCallback );
	  SetButtonCursor(giMessageButton[0], CURSOR_LAPTOP_SCREEN);

		if( giNumberOfPagesToCurrentEmail > 2 )
		{
			// add next and previous mail page buttons
			giMailMessageButtonsImage[0]=  LoadButtonImage( "LAPTOP/NewMailButtons.sti" ,-1,0,-1,3,-1 );
			giMailMessageButtons[0] = QuickCreateButton( giMailMessageButtonsImage[0], PREVIOUS_PAGE_BUTTON_X,(INT16) ( LOWER_BUTTON_Y + ( INT16 )iViewerY + 2),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnPreviousEmailPageCallback );
			SetButtonCursor(giMailMessageButtons[0], CURSOR_LAPTOP_SCREEN);

			giMailMessageButtonsImage[1]=  LoadButtonImage( "LAPTOP/NewMailButtons.sti" ,-1,1,-1,4,-1 );
			giMailMessageButtons[1] = QuickCreateButton( giMailMessageButtonsImage[1], NEXT_PAGE_BUTTON_X ,(INT16) ( LOWER_BUTTON_Y + ( INT16 )iViewerY + 2),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnNextEmailPageCallback );
			SetButtonCursor(giMailMessageButtons[1], CURSOR_LAPTOP_SCREEN);

			gfPageButtonsWereCreated = TRUE;

		}

    giMailMessageButtonsImage[2]=  LoadButtonImage( "LAPTOP/NewMailButtons.sti" ,-1,2,-1,5,-1 );
		giMailMessageButtons[2] = QuickCreateButton( giMailMessageButtonsImage[2], DELETE_BUTTON_X,(INT16) ( BUTTON_LOWER_Y + ( INT16 )iViewerY + 2),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnDeleteCallback );
    SetButtonCursor(giMailMessageButtons[2], CURSOR_LAPTOP_SCREEN);

		// force update of screen
	  fReDrawScreenFlag=TRUE;
	}
  else if((!fDisplayMessageFlag)&&(fOldDisplayMessageFlag))
  {
		// delete region
    fOldDisplayMessageFlag=FALSE;
	  RemoveButton(giMessageButton[0] );
	  UnloadButtonImage( giMessageButtonImage[0] );

		// net/previous email page buttons
    if( gfPageButtonsWereCreated )
		{
			RemoveButton(giMailMessageButtons[0] );
			UnloadButtonImage( giMailMessageButtonsImage[0] );
			RemoveButton(giMailMessageButtons[1] );
			UnloadButtonImage( giMailMessageButtonsImage[1] );
			gfPageButtonsWereCreated = FALSE;
		}
		RemoveButton(giMailMessageButtons[2] );
	  UnloadButtonImage( giMailMessageButtonsImage[2] );
    // force update of screen
		fReDrawScreenFlag=TRUE;
	}

}

void CreateDestroyNewMailButton()
{
 static BOOLEAN fOldNewMailFlag=FALSE;

 // check if we are video conferencing, if so, do nothing
 if( gubVideoConferencingMode != 0 )
 {
	 return ;
 }


 if((fNewMailFlag)&&(!fOldNewMailFlag))
 {
	 // create new mail dialog box button

	 // set old flag (stating button has been created)
  fOldNewMailFlag=TRUE;

	// load image and setup button
	giNewMailButtonImage[0]=LoadButtonImage( "LAPTOP/YesNoButtons.sti" ,-1,0,-1,1,-1 );
  giNewMailButton[0]= QuickCreateButton( giNewMailButtonImage[0], NEW_BTN_X+10, NEW_BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST-2,
										BtnGenericMouseMoveButtonCallback, BtnNewOkback);

	// set cursor
	SetButtonCursor(giNewMailButton[0], CURSOR_LAPTOP_SCREEN);

	// set up screen mask region
	MSYS_DefineRegion(&pScreenMask,0, 0,640,480,
		MSYS_PRIORITY_HIGHEST-3,CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, LapTopScreenCallBack);
	MarkAButtonDirty(  giNewMailButton[0] );
	fReDrawScreenFlag = TRUE;
 }
 else if((!fNewMailFlag)&&(fOldNewMailFlag))
 {


	 // reset old flag
   fOldNewMailFlag=FALSE;

	 // remove the button
   RemoveButton( giNewMailButton[0] );
   UnloadButtonImage( giNewMailButtonImage[0] );

	 // remove screen mask
	 MSYS_RemoveRegion( &pScreenMask );

	 // redraw screen
		 fPausedReDrawScreenFlag=TRUE;
 }
}


BOOLEAN DisplayNewMailBox( void )
{
	static BOOLEAN fOldNewMailFlag=FALSE;
	// will display a new mail box whenever new mail has arrived

	// check if we are video conferencing, if so, do nothing
  if( gubVideoConferencingMode != 0 )
  {
	  return( FALSE );
  }

	// just stopped displaying box, reset old flag
	if( ( !fNewMailFlag ) && ( fOldNewMailFlag ) )
	{
		fOldNewMailFlag=FALSE;
		return ( FALSE );
	}

	// not even set, leave NOW!
	if( !fNewMailFlag )
		return ( FALSE );

  BltVideoObjectFromIndex(FRAME_BUFFER, guiEmailWarning,  0, EMAIL_WARNING_X,     EMAIL_WARNING_Y);
  BltVideoObjectFromIndex(FRAME_BUFFER, guiTITLEBARICONS, 0, EMAIL_WARNING_X + 5, EMAIL_WARNING_Y + 2);

	// font stuff
	SetFont( EMAIL_HEADER_FONT );
	SetFontForeground( FONT_WHITE );
	SetFontBackground( FONT_BLACK );
	SetFontShadow( DEFAULT_SHADOW );

	// print warning
	mprintf(EMAIL_WARNING_X + 30, EMAIL_WARNING_Y + 8, pEmailTitleText[0] );

  // font stuff
	SetFontShadow( NO_SHADOW );
	SetFont( EMAIL_WARNING_FONT );
  SetFontForeground( FONT_BLACK );

	// printf warning string
	mprintf(EMAIL_WARNING_X + 60, EMAIL_WARNING_Y + 63, pNewMailStrings[0] );

	// invalidate region
	InvalidateRegion( EMAIL_WARNING_X, EMAIL_WARNING_Y, EMAIL_WARNING_X + 270, EMAIL_WARNING_Y + 200 );

	// mark button
	MarkAButtonDirty(  giNewMailButton[0] );

	// reset shadow
	SetFontShadow( DEFAULT_SHADOW );

	// redraw icons

	// set box as displayed
	fOldNewMailFlag=TRUE;



	// return
	return ( TRUE );
}

void ReDrawNewMailBox( void )
{

	// this function will check to see if the new mail region needs to be redrawn
	if( fReDrawNewMailFlag == TRUE )
	{
		if( fNewMailFlag )
		{
			// set display flag back to orginal
			fNewMailFlag = FALSE;

			// display new mail box
      DisplayNewMailBox( );

		  // dirty buttons
			MarkAButtonDirty(  giNewMailButton[0] );



			// set display flag back to orginal
			fNewMailFlag = TRUE;

		  // time to redraw
      DisplayNewMailBox( );
		}

		// reset flag for redraw
		  fReDrawNewMailFlag = FALSE;

		return;
	}
}


static void NextRegionButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// not on last page, move ahead one
		if (iCurrentPage < iLastPage)
		{
			iCurrentPage++;
			RenderEmail();
			MarkButtonsDirty();
		}
	}
}


static void BtnPreviousEmailPageCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (giMessagePage > 0) giMessagePage--;
		RenderEmail();
		MarkButtonsDirty();
  }
}


static void BtnNextEmailPageCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
    // not on last page, move ahead one
		if (giNumberOfPagesToCurrentEmail - 1 <= giMessagePage) return;

    if (!fOnLastPageFlag)
		{
			if (giNumberOfPagesToCurrentEmail - 1 > giMessagePage + 1)
				giMessagePage++;
		}

		MarkButtonsDirty();
		fReDrawScreenFlag = TRUE;
  }
}


static void PreviousRegionButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// if we are not on forst page, more back one
		if (iCurrentPage > 0)
		{
			iCurrentPage--;
			RenderEmail();
			MarkButtonsDirty();
		}
	}
}


static void BtnDeleteNoback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		MailToDelete = NULL;
		fReDrawScreenFlag = TRUE;
	}
}


static void DeleteEmail(void);


static void BtnDeleteYesback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fReDrawScreenFlag = TRUE;
		DeleteEmail();
	}
}


static void CreateNextPreviousEmailPageButtons(void);


static void CreateDestroyNextPreviousRegions(void)
{
	static BOOLEAN fCreated=FALSE;
	if(fCreated)
	{
		// destroy already create next, previous mouse regions
	  fCreated=FALSE;

		RemoveButton( giMailPageButtons[ 1 ] );
		UnloadButtonImage( giMailPageButtonsImage[ 1 ] );

		RemoveButton( giMailPageButtons[ 0 ] );
		UnloadButtonImage( giMailPageButtonsImage[ 0 ] );

	}
	else
	{
		// create uncreated mouse regions
		fCreated=TRUE;

		CreateNextPreviousEmailPageButtons( );
	}
}


void CreateDestroyDeleteNoticeMailButton()
{
 static BOOLEAN fOldDeleteMailFlag=FALSE;
	if (MailToDelete != NULL && !fOldDeleteMailFlag)
 {
	 // confirm delete email buttons

	 // YES button
  fOldDeleteMailFlag=TRUE;
	giDeleteMailButtonImage[0]=LoadButtonImage( "LAPTOP/YesNoButtons.sti" ,-1,0,-1,1,-1 );
  giDeleteMailButton[0]= QuickCreateButton( giDeleteMailButtonImage[0], NEW_BTN_X+1, NEW_BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 2,
										BtnGenericMouseMoveButtonCallback, BtnDeleteYesback);

	// NO button
	giDeleteMailButtonImage[1]=LoadButtonImage( "LAPTOP/YesNoButtons.sti" ,-1,2,-1,3,-1 );
  giDeleteMailButton[1]= QuickCreateButton( giDeleteMailButtonImage[1], NEW_BTN_X+40, NEW_BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 2,
										BtnGenericMouseMoveButtonCallback, BtnDeleteNoback);

	// set up cursors
	SetButtonCursor(giDeleteMailButton[0], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(giDeleteMailButton[1], CURSOR_LAPTOP_SCREEN);

	// set up screen mask to prevent other actions while delete mail box is destroyed
	MSYS_DefineRegion(&pDeleteScreenMask,0, 0,640,480,
		MSYS_PRIORITY_HIGHEST-3,CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, LapTopScreenCallBack);

	// force update
	fReDrawScreenFlag = TRUE;

 }
	else if (MailToDelete == NULL && fOldDeleteMailFlag)
 {

	 // clear out the buttons and screen mask
   fOldDeleteMailFlag=FALSE;
   RemoveButton( giDeleteMailButton[0] );
   UnloadButtonImage( giDeleteMailButtonImage[0] );
   RemoveButton( giDeleteMailButton[1] );
   UnloadButtonImage( giDeleteMailButtonImage[1] );

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

  BltVideoObjectFromIndex(FRAME_BUFFER, guiEmailWarning, 0, EMAIL_WARNING_X, EMAIL_WARNING_Y);

	// font stuff
	SetFont( EMAIL_HEADER_FONT );
	SetFontForeground( FONT_WHITE );
	SetFontBackground( FONT_BLACK );
	SetFontShadow( DEFAULT_SHADOW );

  BltVideoObjectFromIndex(FRAME_BUFFER, guiTITLEBARICONS, 0, EMAIL_WARNING_X + 5, EMAIL_WARNING_Y + 2);

  // title
	mprintf(EMAIL_WARNING_X + 30, EMAIL_WARNING_Y + 8, pEmailTitleText[0] );

	// shadow, font, and foreground
	SetFontShadow( NO_SHADOW );
	SetFont( EMAIL_WARNING_FONT );
  SetFontForeground( FONT_BLACK );

	// draw text based on mail being read or not
	if((pMail->fRead))
	 mprintf(EMAIL_WARNING_X + 95 , EMAIL_WARNING_Y + 65,pDeleteMailStrings[0]);
  else
	 mprintf(EMAIL_WARNING_X + 70, EMAIL_WARNING_Y + 65,pDeleteMailStrings[1]);


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

	 // invalidate
	 InvalidateRegion(0,0,640,480);
}


static void FromCallback(GUI_BUTTON *btn, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// sort messages based on sender name, then replace into pages of email
		fSortSenderUpwards = !fSortSenderUpwards;
		SortMessages(SENDER);
		PlaceMessagesinPages();
	}
}


static void SubjectCallback(GUI_BUTTON *btn, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// sort message on subject and reorder list
		fSortSubjectUpwards = !fSortSubjectUpwards;
		SortMessages(SUBJECT);
		PlaceMessagesinPages();
	}
}


static void BtnDeleteCallback(GUI_BUTTON *btn, INT32 iReason)
{
 if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
 {
		MailToDelete = CurrentMail;
 }
}


static void DateCallback(GUI_BUTTON *btn, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// sort messages based on date recieved and reorder lsit
		fSortDateUpwards = !fSortDateUpwards;
		SortMessages(RECEIVED);
		PlaceMessagesinPages();
	}
}


static void ReadCallback(GUI_BUTTON *btn, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// sort messages based on date recieved and reorder lsit
		SortMessages(READ);
		PlaceMessagesinPages();
	}
}


static void DisplayTextOnTitleBar(void)
{
	// draw email screen title text

	// font stuff
  SetFont( EMAIL_TITLE_FONT );
  SetFontForeground( FONT_WHITE );
	SetFontBackground( FONT_BLACK );

	// printf the title
	mprintf( EMAIL_TITLE_X, EMAIL_TITLE_Y, pEmailTitleText[0] );

	// reset the shadow

}


static void DestroyMailScreenButtons(void)
{
  // this function will destory the buttons used in the email screen

	// the sort email buttons
  RemoveButton( giSortButton[0] );
  UnloadButtonImage( giSortButtonImage[0] );
  RemoveButton( giSortButton[1] );
  UnloadButtonImage( giSortButtonImage[1] );
  RemoveButton( giSortButton[2] );
  UnloadButtonImage( giSortButtonImage[2] );
  RemoveButton( giSortButton[3] );
  UnloadButtonImage( giSortButtonImage[3] );
}


static void CreateMailScreenButtons(void)
{

	// create sort buttons, right now - not finished

	// read sort
	giSortButtonImage[0]=  LoadButtonImage( "LAPTOP/mailbuttons.sti" ,-1,0,-1,4,-1 );
	giSortButton[0] = QuickCreateButton( giSortButtonImage[0], ENVELOPE_BOX_X, FROM_BOX_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, ReadCallback);
	SetButtonCursor(giSortButton[0], CURSOR_LAPTOP_SCREEN);


	// subject sort
	giSortButtonImage[1]=  LoadButtonImage( "LAPTOP/mailbuttons.sti" ,-1,1,-1,5,-1 );
	giSortButton[1] = QuickCreateButton( giSortButtonImage[1], FROM_BOX_X, FROM_BOX_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, FromCallback);
	SetButtonCursor(giSortButton[1], CURSOR_LAPTOP_SCREEN);
   SpecifyFullButtonTextAttributes( giSortButton[1], pEmailHeaders[FROM_HEADER], EMAIL_WARNING_FONT,
																		  FONT_BLACK, FONT_BLACK,
																			FONT_BLACK, FONT_BLACK, TEXT_CJUSTIFIED );


	// sender sort
	giSortButtonImage[2]=  LoadButtonImage( "LAPTOP/mailbuttons.sti" ,-1,2,-1,6,-1 );
	giSortButton[2] = QuickCreateButton( giSortButtonImage[2], SUBJECT_BOX_X, FROM_BOX_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, SubjectCallback );
	SetButtonCursor(giSortButton[2], CURSOR_LAPTOP_SCREEN);
	SpecifyFullButtonTextAttributes( giSortButton[2], pEmailHeaders[SUBJECT_HEADER], EMAIL_WARNING_FONT,
																		  FONT_BLACK, FONT_BLACK,
																			FONT_BLACK, FONT_BLACK, TEXT_CJUSTIFIED );



	// date sort
  giSortButtonImage[3]=  LoadButtonImage( "LAPTOP/mailbuttons.sti" ,-1,3,-1,7,-1 );
	giSortButton[3] = QuickCreateButton( giSortButtonImage[3], DATE_BOX_X, FROM_BOX_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, DateCallback);
	SetButtonCursor(giSortButton[3], CURSOR_LAPTOP_SCREEN);
	SpecifyFullButtonTextAttributes( giSortButton[3], pEmailHeaders[RECD_HEADER], EMAIL_WARNING_FONT,
																		  FONT_BLACK, FONT_BLACK,
																			FONT_BLACK, FONT_BLACK, TEXT_CJUSTIFIED );
}


static void DisplayEmailMessageSubjectDateFromLines(Email* pMail, INT32 iViewerY)
{
	// this procedure will draw the title/headers to From, Subject, Date fields in the display
	// message box
  UINT16 usX, usY;

	// font stuff
	SetFont(MESSAGE_FONT);
	SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);



	// all headers, but not info are right justified

  // print from
	FindFontRightCoordinates(MESSAGE_HEADER_X - 20, MESSAGE_FROM_Y + iViewerY, MESSAGE_HEADER_WIDTH, MESSAGE_FROM_Y + GetFontHeight(MESSAGE_FONT), pEmailHeaders[0], MESSAGE_FONT, &usX, &usY);
	mprintf(usX, MESSAGE_FROM_Y + iViewerY, pEmailHeaders[0]);

	// the actual from info
	mprintf( MESSAGE_HEADER_X+MESSAGE_HEADER_WIDTH-13, MESSAGE_FROM_Y + iViewerY, pSenderNameList[pMail->ubSender]);


  // print date
	FindFontRightCoordinates(MESSAGE_HEADER_X + 168, MESSAGE_DATE_Y + iViewerY, MESSAGE_HEADER_WIDTH, MESSAGE_DATE_Y + GetFontHeight(MESSAGE_FONT), pEmailHeaders[2], MESSAGE_FONT, &usX, &usY);
	mprintf(usX, MESSAGE_DATE_Y + iViewerY, pEmailHeaders[2]);

	// the actual date info
	mprintf(MESSAGE_HEADER_X + 235, MESSAGE_DATE_Y + iViewerY, L"%d", pMail->iDate / (24 * 60));

	// print subject
	FindFontRightCoordinates(MESSAGE_HEADER_X - 20, MESSAGE_SUBJECT_Y, MESSAGE_HEADER_WIDTH, MESSAGE_SUBJECT_Y + GetFontHeight(MESSAGE_FONT), pEmailHeaders[1], MESSAGE_FONT, &usX, &usY);
	mprintf(usX, MESSAGE_SUBJECT_Y + iViewerY, pEmailHeaders[1]);

 	// the actual subject info
	IanDisplayWrappedString(SUBJECT_LINE_X + 2, SUBJECT_LINE_Y + 2 + iViewerY, SUBJECT_LINE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, MESSAGE_COLOR, pMail->pSubject, 0, 0);

	// reset shadow
	SetFontShadow(DEFAULT_SHADOW);
}


static void DrawEmailMessageDisplayTitleText(INT32 iViewerY)
{
  // this procedure will display the title of the email message display box

	// font stuff
  SetFont( EMAIL_HEADER_FONT );
	SetFontForeground( FONT_WHITE );
	SetFontBackground( FONT_BLACK );

	// dsiplay mail viewer title on message viewer
  mprintf( VIEWER_X + 30, VIEWER_Y + 8 + (UINT16) iViewerY, pEmailTitleText[0] );
}


static void DrawLineDividers(void)
{
  // this function draws divider lines between lines of text
	INT32 iCounter=0;

	for(iCounter=1; iCounter < 19; iCounter++)
	{
		BltVideoObjectFromIndex(FRAME_BUFFER, guiMAILDIVIDER, 0, INDIC_X - 10, MIDDLE_Y + iCounter * MIDDLE_WIDTH - 1);
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

		MemFree( pTempRecord );
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


static void AddEmailRecordToList(STR16 pString)
{
	Record* pTempRecord;

	// set to head of list
	pTempRecord=pMessageRecordList;

	if(!pTempRecord)
	{

		// list empty, set this node to head
		pTempRecord = MemAlloc( sizeof(Record));
		pMessageRecordList = pTempRecord;
	}
  else
	{
	  // run to end of list
	  while(pTempRecord -> Next)
		{
		  pTempRecord = pTempRecord -> Next;
		}

		// found, alloc
		pTempRecord -> Next = MemAlloc( sizeof(Record) );

		// move to node
		pTempRecord = pTempRecord -> Next;
  }

	// set next to null
	pTempRecord -> Next = NULL;

	// copy in string
  wcscpy( pTempRecord -> pRecord, pString );
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


typedef enum SkillBits
{
	SKILL_MECH = 1 << 0,
	SKILL_MARK = 1 << 1,
	SKILL_MED  = 1 << 2,
	SKILL_EXPL = 1 << 3
} SkillBits;


typedef enum PhysicalBits
{
	PHYS_HLTH = 1 << 0,
	PHYS_DEX  = 1 << 1,
	PHYS_STR  = 1 << 2,
	PHYS_AGI  = 1 << 3,
	PHYS_WIS  = 1 << 4,
	PHYS_LDR  = 1 << 5
} PhysicalBits;


static void LoadIMPResultText(wchar_t* Text, UINT32 Offset)
{
	LoadEncryptedDataFromFile("BINARYDATA/Impass.edt", Text, MAIL_STRING_SIZE * Offset, MAIL_STRING_SIZE);
}


static void AddIMPResultText(UINT32 Offset)
{
	wchar_t Text[MAIL_STRING_SIZE / 2];
	LoadIMPResultText(Text, Offset);
	AddEmailRecordToList(Text);
}


static void AddSkillTraitText(const MERCPROFILESTRUCT* Imp, SkillTrait Skill, UINT32 Offset)
{
	if (Imp->bSkillTrait == Skill || Imp->bSkillTrait2 == Skill)
	{
		AddIMPResultText(Offset);
	}
}


static void HandleIMPCharProfileResultsMessage(void)
{
  // special case, IMP profile return
	INT32 iOffSet;
  INT32 iEndOfSection;

	INT32 iRand = Random(32767);

	if (pMessageRecordList != NULL) return;
	// list doesn't exist, reload

	const MERCPROFILESTRUCT* Imp = &gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId];

	// load intro
	iEndOfSection = IMP_RESULTS_INTRO_LENGTH;
	for (INT32 i = 0; i < iEndOfSection; ++i)
	{
		wchar_t pString[MAIL_STRING_SIZE];
		LoadIMPResultText(pString, i);

		// have to place players name into string for first record
		if (i == 0)
		{
			wchar_t zTemp[512];
			swprintf(zTemp, lengthof(zTemp), L" %ls", Imp->zName);
			wcscat(pString, zTemp);
		}

		AddEmailRecordToList(pString);
	}

	// now the personality intro
	iOffSet = IMP_RESULTS_PERSONALITY_INTRO;
	iEndOfSection = IMP_RESULTS_PERSONALITY_INTRO_LENGTH + 1;
	for (INT32 i = 0; i < iEndOfSection; ++i) AddIMPResultText(iOffSet + i);

	// personality itself
	switch (Imp->bPersonalityTrait)
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
//		AddIMPResultText(iOffSet + Random(IMP_PERSONALITY_LENGTH - 1) + 1);
	AddIMPResultText(iOffSet + 1);

	// persoanlity paragraph
	AddIMPResultText(iOffSet + IMP_PERSONALITY_LENGTH);

	// extra paragraph for bugs
	if (Imp->bPersonalityTrait == FEAR_OF_INSECTS)
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
	switch (Imp->bAttitude)
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
//		AddIMPResultText(iOffSet + Random(IMP_ATTITUDE_LENGTH - 2) + 1);
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

	Skill = 0;
	if (Imp->bMarksmanship >= SUPER_SKILL_VALUE) Skill |= SKILL_MARK;
	if (Imp->bMedical      >= SUPER_SKILL_VALUE) Skill |= SKILL_MED;
	if (Imp->bMechanical   >= SUPER_SKILL_VALUE) Skill |= SKILL_MECH;
	if (Imp->bExplosive    >= SUPER_SKILL_VALUE) Skill |= SKILL_EXPL;

	if (Skill != 0) AddIMPResultText(IMP_SKILLS_IMPERIAL_SKILLS);

	if (Skill & SKILL_MARK) AddIMPResultText(IMP_SKILLS_IMPERIAL_MARK);
	if (Skill & SKILL_MED)  AddIMPResultText(IMP_SKILLS_IMPERIAL_MED);
	if (Skill & SKILL_MECH) AddIMPResultText(IMP_SKILLS_IMPERIAL_MECH);
	if (Skill & SKILL_EXPL) AddIMPResultText(IMP_SKILLS_IMPERIAL_EXPL);


	// now the needs training values
	Skill = 0;
	if (Imp->bMarksmanship > NO_CHANCE_IN_HELL_SKILL_VALUE && Imp->bMarksmanship <= NEEDS_TRAINING_SKILL_VALUE) Skill |= SKILL_MARK;
	if (Imp->bMedical      > NO_CHANCE_IN_HELL_SKILL_VALUE && Imp->bMedical      <= NEEDS_TRAINING_SKILL_VALUE) Skill |= SKILL_MED;
	if (Imp->bMechanical   > NO_CHANCE_IN_HELL_SKILL_VALUE && Imp->bMechanical   <= NEEDS_TRAINING_SKILL_VALUE) Skill |= SKILL_MECH;
	if (Imp->bExplosive    > NO_CHANCE_IN_HELL_SKILL_VALUE && Imp->bExplosive    <= NEEDS_TRAINING_SKILL_VALUE) Skill |= SKILL_EXPL;

	if (Skill != 0) AddIMPResultText(IMP_SKILLS_NEED_TRAIN_SKILLS);

	if (Skill & SKILL_MARK) AddIMPResultText(IMP_SKILLS_NEED_TRAIN_MARK);
	if (Skill & SKILL_MED)  AddIMPResultText(IMP_SKILLS_NEED_TRAIN_MED);
	if (Skill & SKILL_MECH) AddIMPResultText(IMP_SKILLS_NEED_TRAIN_MECH);
	if (Skill & SKILL_EXPL) AddIMPResultText(IMP_SKILLS_NEED_TRAIN_EXPL);


	// and the no chance in hell of doing anything useful values
	Skill = 0;
	if (Imp->bMarksmanship <= NO_CHANCE_IN_HELL_SKILL_VALUE) Skill |= SKILL_MARK;
	if (Imp->bMedical      <= NO_CHANCE_IN_HELL_SKILL_VALUE) Skill |= SKILL_MED;
	if (Imp->bMechanical   <= NO_CHANCE_IN_HELL_SKILL_VALUE) Skill |= SKILL_MECH;
	if (Imp->bExplosive    <= NO_CHANCE_IN_HELL_SKILL_VALUE) Skill |= SKILL_EXPL;

	if (Skill != 0) AddIMPResultText(IMP_SKILLS_NO_SKILL);

	if (Skill & SKILL_MECH) AddIMPResultText(IMP_SKILLS_NO_SKILL_MECH);
	if (Skill & SKILL_MARK) AddIMPResultText(IMP_SKILLS_NO_SKILL_MARK);
	if (Skill & SKILL_MED)  AddIMPResultText(IMP_SKILLS_NO_SKILL_MED);
	if (Skill & SKILL_EXPL) AddIMPResultText(IMP_SKILLS_NO_SKILL_EXPL);


	// now the specialized skills
	// imperial skills
	iOffSet = IMP_SKILLS_SPECIAL_INTRO;
	iEndOfSection = IMP_SKILLS_SPECIAL_INTRO_LENGTH;
	for (INT32 i = 0; i < iEndOfSection; ++i) AddIMPResultText(iOffSet + i);

	AddSkillTraitText(Imp, KNIFING,     IMP_SKILLS_SPECIAL_KNIFE);
	AddSkillTraitText(Imp, LOCKPICKING, IMP_SKILLS_SPECIAL_LOCK);
	AddSkillTraitText(Imp, HANDTOHAND,  IMP_SKILLS_SPECIAL_HAND);
	AddSkillTraitText(Imp, ELECTRONICS, IMP_SKILLS_SPECIAL_ELEC);
	AddSkillTraitText(Imp, NIGHTOPS,    IMP_SKILLS_SPECIAL_NIGHT);
	AddSkillTraitText(Imp, THROWING,    IMP_SKILLS_SPECIAL_THROW);
	AddSkillTraitText(Imp, TEACHING,    IMP_SKILLS_SPECIAL_TEACH);
	AddSkillTraitText(Imp, HEAVY_WEAPS, IMP_SKILLS_SPECIAL_HEAVY);
	AddSkillTraitText(Imp, AUTO_WEAPS,  IMP_SKILLS_SPECIAL_AUTO);
	AddSkillTraitText(Imp, STEALTHY,    IMP_SKILLS_SPECIAL_STEALTH);
	AddSkillTraitText(Imp, AMBIDEXT,    IMP_SKILLS_SPECIAL_AMBI);
	AddSkillTraitText(Imp, THIEF,       IMP_SKILLS_SPECIAL_THIEF);
	AddSkillTraitText(Imp, MARTIALARTS, IMP_SKILLS_SPECIAL_MARTIAL);


	// now the physical
	// imperial physical
	iOffSet = IMP_RESULTS_PHYSICAL;
	iEndOfSection = IMP_RESULTS_PHYSICAL_LENGTH;
	for (INT32 i = 0; i < iEndOfSection; ++i) AddIMPResultText(iOffSet + i);

	PhysicalBits Phys;

	// super physical
	Phys = 0;
	if (Imp->bLife       >= SUPER_STAT_VALUE) Phys |= PHYS_HLTH;
	if (Imp->bDexterity  >= SUPER_STAT_VALUE) Phys |= PHYS_DEX;
	if (Imp->bAgility    >= SUPER_STAT_VALUE) Phys |= PHYS_AGI;
	if (Imp->bStrength   >= SUPER_STAT_VALUE) Phys |= PHYS_STR;
	if (Imp->bWisdom     >= SUPER_STAT_VALUE) Phys |= PHYS_WIS;
	if (Imp->bLeadership >= SUPER_STAT_VALUE) Phys |= PHYS_LDR;

	if (Phys != 0) AddIMPResultText(IMP_PHYSICAL_SUPER);

	if (Phys & PHYS_HLTH) AddIMPResultText(IMP_PHYSICAL_SUPER_HEALTH);
	if (Phys & PHYS_DEX)  AddIMPResultText(IMP_PHYSICAL_SUPER_DEXTERITY);
	if (Phys & PHYS_STR)  AddIMPResultText(IMP_PHYSICAL_SUPER_STRENGTH);
	if (Phys & PHYS_AGI)  AddIMPResultText(IMP_PHYSICAL_SUPER_AGILITY);
	if (Phys & PHYS_WIS)  AddIMPResultText(IMP_PHYSICAL_SUPER_WISDOM);
	if (Phys & PHYS_LDR)  AddIMPResultText(IMP_PHYSICAL_SUPER_LEADERSHIP);


	// now the low attributes
	Phys = 0;
	if (Imp->bLife       < NEEDS_TRAINING_STAT_VALUE && Imp->bLife       > NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_HLTH;
	if (Imp->bStrength   < NEEDS_TRAINING_STAT_VALUE && Imp->bStrength   > NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_STR;
	if (Imp->bAgility    < NEEDS_TRAINING_STAT_VALUE && Imp->bAgility    > NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_AGI;
	if (Imp->bWisdom     < NEEDS_TRAINING_STAT_VALUE && Imp->bWisdom     > NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_WIS;
	if (Imp->bLeadership < NEEDS_TRAINING_STAT_VALUE && Imp->bLeadership > NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_LDR;
	if (Imp->bDexterity  < NEEDS_TRAINING_STAT_VALUE && Imp->bDexterity  > NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_DEX;

	if (Phys != 0) AddIMPResultText(IMP_PHYSICAL_LOW);

	if (Phys & PHYS_HLTH) AddIMPResultText(IMP_PHYSICAL_LOW_HEALTH);
	if (Phys & PHYS_DEX)  AddIMPResultText(IMP_PHYSICAL_LOW_DEXTERITY);
	if (Phys & PHYS_STR)  AddIMPResultText(IMP_PHYSICAL_LOW_STRENGTH);
	if (Phys & PHYS_AGI)  AddIMPResultText(IMP_PHYSICAL_LOW_AGILITY);
	if (Phys & PHYS_WIS)  AddIMPResultText(IMP_PHYSICAL_LOW_WISDOM);
	if (Phys & PHYS_LDR)  AddIMPResultText(IMP_PHYSICAL_LOW_LEADERSHIP);


	// very low physical
	Phys = 0;
	if (Imp->bLife       <= NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_HLTH;
	if (Imp->bDexterity  <= NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_DEX;
	if (Imp->bStrength   <= NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_STR;
	if (Imp->bAgility    <= NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_AGI;
	if (Imp->bWisdom     <= NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_WIS;
	if (Imp->bLeadership <= NO_CHANCE_IN_HELL_STAT_VALUE) Phys |= PHYS_LDR;

	if (Phys != 0) AddIMPResultText(IMP_PHYSICAL_VERY_LOW);

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

	if( fDisplayMessageFlag == FALSE )
	{
		// not displaying message, leave
		return;
	}



	if(  giNumberOfPagesToCurrentEmail <= 2 )
	{
		return;
	}

	// turn off previous page button
	if( giMessagePage == 0 )
	{
		DisableButton( giMailMessageButtons[ 0 ] );
	}
	else
	{
		EnableButton( giMailMessageButtons[ 0 ] );
	}


	// turn off next page button
	if( pEmailPageInfo[ giMessagePage + 1 ].pFirstRecord == NULL )
	{
		DisableButton( giMailMessageButtons[ 1 ] );
	}
	else
	{
		EnableButton( giMailMessageButtons[ 1 ] );
	}
}


static void CreateNextPreviousEmailPageButtons(void)
{

	// this function will create the buttons to advance and go back email pages

	// next button
	giMailPageButtonsImage[0]=  LoadButtonImage( "LAPTOP/NewMailButtons.sti" ,-1,1,-1,4,-1 );
	giMailPageButtons[0] = QuickCreateButton( giMailPageButtonsImage[0],NEXT_PAGE_X, NEXT_PAGE_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, NextRegionButtonCallback );
	SetButtonCursor(giMailPageButtons[0], CURSOR_LAPTOP_SCREEN);

	// previous button
	giMailPageButtonsImage[1]=  LoadButtonImage( "LAPTOP/NewMailButtons.sti" ,-1,0,-1,3,-1 );
	giMailPageButtons[1] = QuickCreateButton( giMailPageButtonsImage[1],PREVIOUS_PAGE_X, NEXT_PAGE_Y,
									BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
									BtnGenericMouseMoveButtonCallback, PreviousRegionButtonCallback );
	SetButtonCursor(giMailPageButtons[1], CURSOR_LAPTOP_SCREEN);
}


static void UpdateStatusOfNextPreviousButtons(void)
{

	// set the states of the page advance buttons

	DisableButton( giMailPageButtons[ 0 ]);
	DisableButton( giMailPageButtons[ 1 ]);

	if( iCurrentPage > 0 )
	{
		EnableButton( giMailPageButtons[ 1 ]);
	}

	if( iCurrentPage < iLastPage )
	{
		EnableButton( giMailPageButtons[ 0 ] );
	}
}


static void DisplayWhichPageOfEmailProgramIsDisplayed(void)
{
	// will draw the number of the email program we are viewing right now
	// font stuff
	SetFont(MESSAGE_FONT);
	SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);

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
	mprintf(PAGE_NUMBER_X, PAGE_NUMBER_Y, L"%d / %d", CPage, LPage);

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
		if( ( pB->iDate < iLowestDate )&&( pB->fRead == FALSE ) )
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


static BOOLEAN DisplayNumberOfPagesToThisEmail(INT32 iViewerY)
{
	// display the indent for the display of pages to this email..along with the current page/number of pages
	INT16 sX = 0, sY = 0;
	CHAR16 sString[ 32 ];

	giNumberOfPagesToCurrentEmail = ( giNumberOfPagesToCurrentEmail );

	// parse current page and max number of pages to email
	swprintf( sString, lengthof(sString), L"%d / %d", ( giMessagePage + 1 ), ( giNumberOfPagesToCurrentEmail - 1 ) );

	SetFont( FONT12ARIAL );
	SetFontForeground( FONT_BLACK );
	SetFontBackground( FONT_BLACK );

	// turn off the shadows
	SetFontShadow(NO_SHADOW);

	SetFontDestBuffer(FRAME_BUFFER, 0, 0, 640, 480);

	FindFontCenterCoordinates(VIEWER_X + INDENT_X_OFFSET, 0,INDENT_X_WIDTH, 0, sString, FONT12ARIAL, &sX, &sY);
	mprintf( sX, VIEWER_Y + iViewerY + INDENT_Y_OFFSET - 2, sString );


	// restore shadows
	SetFontShadow( DEFAULT_SHADOW );

	return ( TRUE );
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
		pTempRecord = GetFirstRecordOnThisPage( pMessageRecordList, MESSAGE_FONT, MESSAGE_WIDTH, MESSAGE_GAP, iNumberOfPagesToEmail, MAX_EMAIL_MESSAGE_PAGE_SIZE );
		iNumberOfPagesToEmail++;
	}


	return( iNumberOfPagesToEmail );
}


void ShutDownEmailList()
{
	Email* pEmail = pEmailList;
	Email* pTempEmail = NULL;

	//loop through all the emails to delete them
	while( pEmail )
	{
		pTempEmail = pEmail;

		pEmail = pEmail->Next;

		MemFree( pTempEmail );
		pTempEmail = NULL;
	}
	pEmailList = NULL;

	ClearPages();
}


static void PreProcessEmail(Email* pMail)
{
	Record* pTempRecord;
	Record* pCurrentRecord;
	Record* pLastRecord;
	Record* pTempList;
	INT32 iCounter = 0, iHeight = 0, iOffSet = 0;
	BOOLEAN fGoingOffCurrentPage = FALSE;
	INT32 iYPositionOnPage = 0;

	iOffSet=(INT32)pMail->usOffset;

	// set record ptr to head of list
	pTempRecord=pMessageRecordList;

	if( pEmailPageInfo[ 0 ].pFirstRecord != NULL )
	{
		// already processed
		return;
	}

	// list doesn't exist, reload
	if( !pTempRecord )
  {
	  while(pMail->usLength > iCounter)
		{
			wchar_t pString[512];
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA/Email.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );

      // increment email record counter
		  iCounter++;
    }
		PreviousMail = CurrentMail;
	}

	// set record ptr to head of list
	pTempRecord=pMessageRecordList;
//def removed
	// pass the subject line
	if( pTempRecord && pMail->usOffset != IMP_EMAIL_PROFILE_RESULTS)
	{
		pTempRecord = pTempRecord->Next;
	}

	// get number of pages to this email
	giNumberOfPagesToCurrentEmail = GetNumberOfPagesToEmail( );


	while( pTempRecord )
	{
	  // get the height of the string, ONLY!...must redisplay ON TOP OF background graphic
		iHeight += IanWrappedStringHeight(MESSAGE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, pTempRecord->pRecord);

		// next message record string
		pTempRecord = pTempRecord -> Next;

	}

	// set iViewerY so to center the viewer
  iViewerPositionY =  ( LAPTOP_SCREEN_LR_Y - 2 * VIEWER_Y - 2 * VIEWER_MESSAGE_BODY_START_Y  - iHeight ) /2;

	if ( iViewerPositionY < 0 )
	{
		iViewerPositionY = 0;
	}

	// set total height to height of records displayed
  iTotalHeight=iHeight;

		// if the message background is less than MIN_MESSAGE_HEIGHT_IN_LINES, set to that number
  if( ( iTotalHeight / GetFontHeight( MESSAGE_FONT ) ) < MIN_MESSAGE_HEIGHT_IN_LINES)
	{
		iTotalHeight=GetFontHeight( MESSAGE_FONT ) * MIN_MESSAGE_HEIGHT_IN_LINES;
	}

  if(iTotalHeight > MAX_EMAIL_MESSAGE_PAGE_SIZE)
	{
    // if message to big to fit on page
		iTotalHeight = MAX_EMAIL_MESSAGE_PAGE_SIZE + 10;
	}
	else
	{
		iTotalHeight += 10;
	}

	pTempRecord=pMessageRecordList;

	if( iTotalHeight < MAX_EMAIL_MESSAGE_PAGE_SIZE )
	{
		fOnLastPageFlag = TRUE;

		if( pTempRecord && pMail->usOffset != IMP_EMAIL_PROFILE_RESULTS)
		{
			pTempRecord = pTempRecord->Next;
		}

		pEmailPageInfo[ 0 ].pFirstRecord = pTempRecord ;
		pEmailPageInfo[ 0 ].iPageNumber = 0;


		Assert(pTempRecord);		// required, otherwise we're testing pCurrentRecord when undefined later

	  while( pTempRecord )
		{
			pCurrentRecord = pTempRecord;

			// increment email record ptr
		  pTempRecord = pTempRecord -> Next;

		}

		// only one record to this email?..then set next to null
		if( pCurrentRecord == pEmailPageInfo[ 0 ].pFirstRecord )
		{
			pCurrentRecord = NULL;
		}

		// set up the last record for the page
		pEmailPageInfo[ 0 ].pLastRecord = pCurrentRecord;

		// now set up the next page
		pEmailPageInfo[ 1 ].pFirstRecord = NULL;
		pEmailPageInfo[ 1 ].pLastRecord = NULL;
		pEmailPageInfo[ 1 ].iPageNumber = 1;
	}
	else
	{
		fOnLastPageFlag = FALSE;
		pTempList = pMessageRecordList;

		if( pTempList && pMail->usOffset != IMP_EMAIL_PROFILE_RESULTS)
		{
			pTempList = pTempList->Next;
		}

		iCounter = 0;

		// more than one page
		while( pTempRecord = GetFirstRecordOnThisPage( pTempList, MESSAGE_FONT, MESSAGE_WIDTH, MESSAGE_GAP, iCounter, MAX_EMAIL_MESSAGE_PAGE_SIZE ) )
		{
			iYPositionOnPage = 0;

			pEmailPageInfo[ iCounter ].pFirstRecord = pTempRecord;
			pEmailPageInfo[ iCounter ].iPageNumber = iCounter;
			pLastRecord = NULL;

			// go to the right record
			while( pTempRecord )
			{
				if (pTempRecord->pRecord[0] == L'\0')
				{
					// on last page
					fOnLastPageFlag = TRUE;
				}


				if (iYPositionOnPage + IanWrappedStringHeight(MESSAGE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, pTempRecord->pRecord) <= MAX_EMAIL_MESSAGE_PAGE_SIZE)
				{
     			// now print it
					iYPositionOnPage += IanWrappedStringHeight(MESSAGE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, pTempRecord->pRecord);
					fGoingOffCurrentPage = FALSE;
				}
				else
				{
					// gonna get cut off...end now
					fGoingOffCurrentPage = TRUE;
				}



				pCurrentRecord = pTempRecord;
				pTempRecord = pTempRecord ->Next;

				if( fGoingOffCurrentPage == FALSE )
				{
					pLastRecord = pTempRecord;
				}
				// record get cut off?...end now

				if( fGoingOffCurrentPage == TRUE )
				{
					pTempRecord = NULL;
				}
			}

			if( pLastRecord == pEmailPageInfo[ iCounter ].pFirstRecord )
			{
				pLastRecord = NULL;
			}

			pEmailPageInfo[ iCounter ].pLastRecord = pLastRecord;
			iCounter++;
		}

		pEmailPageInfo[ iCounter ].pFirstRecord = NULL;
		pEmailPageInfo[ iCounter ].pLastRecord = NULL;
		pEmailPageInfo[ iCounter ].iPageNumber = iCounter;
	}



}


static void ModifyInsuranceEmails(UINT16 usMessageId, Email* pMail, UINT8 ubNumberOfRecords)
{
	wchar_t pString[MAIL_STRING_SIZE];
	UINT8	ubCnt;

	for( ubCnt=0; ubCnt<ubNumberOfRecords; ubCnt++)
	{
		// read one record from email file
		LoadEncryptedDataFromFile( "BINARYDATA/Email.edt", pString, MAIL_STRING_SIZE * usMessageId, MAIL_STRING_SIZE );

		//Replace the $MERCNAME$ and $AMOUNT$ with the mercs name and the amountm if the string contains the keywords.
		ReplaceMercNameAndAmountWithProperData( pString, pMail );

		// add to list
		AddEmailRecordToList( pString );

		usMessageId++;
	}

	PreviousMail = CurrentMail;
}


static BOOLEAN ReplaceMercNameAndAmountWithProperData(wchar_t* pFinishedString, const Email* pMail)
{
	const wchar_t* const sMercName = L"$MERCNAME$"; //Doesnt need to be translated, inside Email.txt and will be replaced by the mercs name
	const wchar_t* const sAmount = L"$AMOUN$"; //Doesnt need to be translated, inside Email.txt and will be replaced by a dollar amount

	wchar_t		pTempString[MAIL_STRING_SIZE];
	INT32			iCurLocInSourceString=0;
	INT32			iLengthOfSourceString = wcslen( pFinishedString );		//Get the length of the source string
	BOOLEAN		fReplacingMercName = TRUE;

	//Copy the original string over to the temp string
	wcscpy( pTempString, pFinishedString );

	//Null out the string
	pFinishedString[0] = L'\0';


	//Keep looping through to replace all references to the keyword
	while( iCurLocInSourceString < iLengthOfSourceString )
	{
		//Find out if the $MERCNAME$ is in the string
		const wchar_t* pMercNameString = wcsstr(&pTempString[iCurLocInSourceString], sMercName);
		const wchar_t* pAmountString   = wcsstr(&pTempString[iCurLocInSourceString], sAmount);

		const wchar_t* pSubString;
		const wchar_t* sSearchString;
		if( pMercNameString != NULL && pAmountString != NULL )
		{
			if( pMercNameString < pAmountString )
			{
				fReplacingMercName = TRUE;
				pSubString = pMercNameString;
				sSearchString = sMercName;
			}
			else
			{
				fReplacingMercName = FALSE;
				pSubString = pAmountString;
				sSearchString = sAmount;
			}
		}
		else if( pMercNameString != NULL )
		{
			fReplacingMercName = TRUE;
			pSubString = pMercNameString;
			sSearchString = sMercName;
		}
		else if( pAmountString != NULL )
		{
			fReplacingMercName = FALSE;
			pSubString = pAmountString;
			sSearchString = sAmount;
		}
		else
		{
			pSubString = NULL;
		}


		// if there is a substring
		if( pSubString != NULL )
		{
			INT32 iLength = pSubString - &pTempString[iCurLocInSourceString];

			//Copy the part of the source string upto the keyword
			wcsncat( pFinishedString, &pTempString[ iCurLocInSourceString ], iLength );

			//increment the source string counter by how far in the keyword is and by the length of the keyword
			iCurLocInSourceString+= iLength + wcslen( sSearchString );

			if( fReplacingMercName )
			{
				//add the mercs name to the string
				wcscat( pFinishedString, gMercProfiles[ pMail->uiSecondData ].zName );
			}
			else
			{
				CHAR16	sDollarAmount[64];
				SPrintMoney(sDollarAmount, pMail->iFirstData);

				//add the mercs name to the string
				wcscat( pFinishedString, sDollarAmount );
			}
		}
		else
		{
			//add the rest of the string
			wcscat( pFinishedString, &pTempString[ iCurLocInSourceString ] );

			iCurLocInSourceString += wcslen( &pTempString[ iCurLocInSourceString ] );
		}
	}

	return( TRUE );
}

#ifdef JA2BETAVERSION
void AddAllEmails()
{
	UINT32 uiCnt;
	UINT32 uiOffset;

	AddEmail(IMP_EMAIL_INTRO,IMP_EMAIL_INTRO_LENGTH,CHAR_PROFILE_SITE,  GetWorldTotalMin() );
	AddEmail(ENRICO_CONGRATS,ENRICO_CONGRATS_LENGTH,MAIL_ENRICO, GetWorldTotalMin() );
	AddEmail(IMP_EMAIL_AGAIN,IMP_EMAIL_AGAIN_LENGTH,1, GetWorldTotalMin( ) );
	AddEmail(MERC_INTRO, MERC_INTRO_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin( ) );
	AddEmail( MERC_NEW_SITE_ADDRESS, MERC_NEW_SITE_ADDRESS_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin() );

	AddEmail(IMP_EMAIL_PROFILE_RESULTS, IMP_EMAIL_PROFILE_RESULTS_LENGTH, IMP_PROFILE_RESULTS, GetWorldTotalMin( ) );

	AddEmail( MERC_WARNING, MERC_WARNING_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin());
	AddEmail( MERC_INVALID, MERC_INVALID_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin());
	AddEmail( NEW_MERCS_AT_MERC, NEW_MERCS_AT_MERC_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin());
	AddEmail( MERC_FIRST_WARNING, MERC_FIRST_WARNING_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin());

	uiOffset = MERC_UP_LEVEL_BIFF;
	for( uiCnt=0; uiCnt<10; uiCnt++)
	{
		AddEmail( uiOffset, MERC_UP_LEVEL_LENGTH_BIFF, SPECK_FROM_MERC, GetWorldTotalMin() );
		uiOffset += MERC_UP_LEVEL_LENGTH_BIFF;
	}

	uiOffset = AIM_REPLY_BARRY;
	for( uiCnt=0; uiCnt<40; uiCnt++)
	{
		AddEmail( ( UINT8 )( uiOffset + ( uiCnt * AIM_REPLY_LENGTH_BARRY ) ), AIM_REPLY_LENGTH_BARRY, ( UINT8 )( 6 + uiCnt ), GetWorldTotalMin() );
	}

	AddEmail(OLD_ENRICO_1,OLD_ENRICO_1_LENGTH,MAIL_ENRICO,  GetWorldTotalMin() );
	AddEmail(OLD_ENRICO_2,OLD_ENRICO_2_LENGTH,MAIL_ENRICO,  GetWorldTotalMin() );
	AddEmail(OLD_ENRICO_3,OLD_ENRICO_3_LENGTH,MAIL_ENRICO,  GetWorldTotalMin() );
	AddEmail(RIS_REPORT,RIS_REPORT_LENGTH,RIS_EMAIL,  GetWorldTotalMin() );

	AddEmail( ENRICO_MIGUEL, ENRICO_MIGUEL_LENGTH, MAIL_ENRICO, GetWorldTotalMin() );
	AddEmail(ENRICO_PROG_20, ENRICO_PROG_20_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
	AddEmail(ENRICO_PROG_55, ENRICO_PROG_55_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
	AddEmail(ENRICO_PROG_80, ENRICO_PROG_80_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
	AddEmail(ENRICO_SETBACK, ENRICO_SETBACK_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
	AddEmail(ENRICO_SETBACK_2, ENRICO_SETBACK_2_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
	AddEmail(LACK_PLAYER_PROGRESS_1, LACK_PLAYER_PROGRESS_1_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
	AddEmail(LACK_PLAYER_PROGRESS_2, LACK_PLAYER_PROGRESS_2_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
	AddEmail(LACK_PLAYER_PROGRESS_3, LACK_PLAYER_PROGRESS_3_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
	AddEmail(ENRICO_CREATURES, ENRICO_CREATURES_LENGTH, MAIL_ENRICO, GetWorldTotalMin());


	//Add an email telling the user that he received an insurance payment
	AddEmailWithSpecialData( INSUR_PAYMENT, INSUR_PAYMENT_LENGTH, INSURANCE_COMPANY, GetWorldTotalMin(), 20, 0 );
	AddEmailWithSpecialData( INSUR_SUSPIC, INSUR_SUSPIC_LENGTH, INSURANCE_COMPANY, GetWorldTotalMin(), 20, 0 );
	AddEmailWithSpecialData( INSUR_SUSPIC_2, INSUR_SUSPIC_2_LENGTH, INSURANCE_COMPANY, GetWorldTotalMin(), 20, 0 );
	AddEmail( BOBBYR_NOW_OPEN, BOBBYR_NOW_OPEN_LENGTH, BOBBY_R, GetWorldTotalMin());
	AddEmail( KING_PIN_LETTER, KING_PIN_LETTER_LENGTH, KING_PIN, GetWorldTotalMin() );
	AddEmail( BOBBYR_SHIPMENT_ARRIVED, BOBBYR_SHIPMENT_ARRIVED_LENGTH, BOBBY_R, GetWorldTotalMin() );

	AddEmail( JOHN_KULBA_GIFT_IN_DRASSEN, JOHN_KULBA_GIFT_IN_DRASSEN_LENGTH, JOHN_KULBA, GetWorldTotalMin() );

	AddEmailWithSpecialData(MERC_DIED_ON_OTHER_ASSIGNMENT, MERC_DIED_ON_OTHER_ASSIGNMENT_LENGTH, AIM_SITE, GetWorldTotalMin(), 0, 0 );

	AddEmailWithSpecialData( INSUR_1HOUR_FRAUD, INSUR_1HOUR_FRAUD_LENGTH, INSURANCE_COMPANY, GetWorldTotalMin(), 20, 0 );

	//add an email
	AddEmailWithSpecialData( AIM_MEDICAL_DEPOSIT_REFUND, AIM_MEDICAL_DEPOSIT_REFUND_LENGTH, AIM_SITE, GetWorldTotalMin(), 20, 0 );

	AddEmailWithSpecialData( AIM_MEDICAL_DEPOSIT_PARTIAL_REFUND, AIM_MEDICAL_DEPOSIT_PARTIAL_REFUND_LENGTH, AIM_SITE, GetWorldTotalMin(), 20, 0 );

	AddEmailWithSpecialData( AIM_MEDICAL_DEPOSIT_NO_REFUND, AIM_MEDICAL_DEPOSIT_NO_REFUND_LENGTH, AIM_SITE, GetWorldTotalMin(), 20, 0 );
}
#endif
