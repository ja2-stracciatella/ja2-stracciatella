#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "laptop.h"
	#include "email.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "Debug.h"
	#include "WordWrap.h"
	#include "Render Dirty.h"
	#include "Encrypted File.h"
	#include "cursors.h"
	#include "soldier profile.h"
	#include "IMP Compile Character.h"
	#include "IMP Voices.h"
	#include "IMP Portraits.h"
	#include "Game Clock.h"
	#include "environment.h"
	#include "AimMembers.h"
	#include "Random.h"
	#include "Text.h"
#endif



//static EmailPtr pEmailList;
EmailPtr pEmailList;
static PagePtr  pPageList;
static INT32 iLastPage=-1;
static INT32 iCurrentPage=0;
INT32 iDeleteId=0;
BOOLEAN fUnReadMailFlag=FALSE;
BOOLEAN fOldUnreadFlag=TRUE;
BOOLEAN fNewMailFlag=FALSE;
BOOLEAN fOldNewMailFlag=FALSE;
BOOLEAN fDisplayMessageFlag=FALSE;
BOOLEAN fOldDisplayMessageFlag=FALSE;
BOOLEAN fReDraw=FALSE;
BOOLEAN fDeleteMailFlag=FALSE;
BOOLEAN fReDrawMessageFlag = FALSE;
BOOLEAN fOnLastPageFlag=FALSE;
BOOLEAN fJustStartedEmail = FALSE;
BOOLEAN fDeleteInternal = FALSE;
BOOLEAN fOpenMostRecentUnReadFlag = FALSE;
INT32 iViewerPositionY=0;

INT32 giMessageId = -1;
INT32 giPrevMessageId = -1;
INT32 giMessagePage = -1;
INT32 giNumberOfPagesToCurrentEmail = -1;
UINT32 guiEmailWarning;

#define EMAIL_TOP_BAR_HEIGHT 22

#define LIST_MIDDLE_COUNT 18
// object positions
#define TITLE_X 0+LAPTOP_SCREEN_UL_X
#define TITLE_Y 0+LAPTOP_SCREEN_UL_Y

#define STAMP_X 344+LAPTOP_SCREEN_UL_X
#define STAMP_Y 0+LAPTOP_SCREEN_UL_Y
/*
#define TOP_X 0+LAPTOP_SCREEN_UL_X
#define TOP_Y 62+LAPTOP_SCREEN_UL_Y

#define BOTTOM_X 0+LAPTOP_SCREEN_UL_X
#define BOTTOM_Y 359+LAPTOP_SCREEN_UL_Y
*/
#define MIDDLE_X 0+LAPTOP_SCREEN_UL_X
#define MIDDLE_Y 72 + EMAIL_TOP_BAR_HEIGHT
#define MIDDLE_WIDTH 19


// new graphics
#define EMAIL_LIST_WINDOW_Y 22
#define EMAIL_TITLE_BAR_X 5

// email columns
#define SENDER_X LAPTOP_SCREEN_UL_X+65
#define SENDER_WIDTH 246-158

#define DATE_X LAPTOP_SCREEN_UL_X+428
#define DATE_WIDTH 592-527

#define SUBJECT_X LAPTOP_SCREEN_UL_X+175
#define SUBJECT_WIDTH					254	//526-245
#define INDIC_X 128
#define INDIC_WIDTH 155-123
#define INDIC_HEIGHT 145-128

#define LINE_WIDTH 592-121

#define MESSAGE_X 5	//17
#define MESSAGE_Y 35
#define MESSAGE_WIDTH 528-125//150
#define MESSAGE_COLOR FONT_BLACK
#define MESSAGE_GAP 2



#define MESSAGE_HEADER_WIDTH 209-151
#define MESSAGE_HEADER_X VIEWER_X+4


#define VIEWER_HEAD_X 140
#define VIEWER_HEAD_Y 9
#define VIEWER_HEAD_WIDTH 445-VIEWER_HEAD_X
#define MAX_BUTTON_COUNT 1
#define VIEWER_WIDTH 500
#define VIEWER_HEIGHT 195

#define MESSAGEX_X 425
#define MESSAGEX_Y 6


#define EMAIL_WARNING_X 210
#define EMAIL_WARNING_Y 140
#define EMAIL_WARNING_WIDTH 254
#define EMAIL_WARNING_HEIGHT 138


#define NEW_BTN_X EMAIL_WARNING_X +(338-245)
#define NEW_BTN_Y EMAIL_WARNING_Y +(278-195)

#define EMAIL_TEXT_FONT				FONT10ARIAL
#define TRAVERSE_EMAIL_FONT		FONT14ARIAL
#define EMAIL_BOX_FONT				FONT14ARIAL
#define MESSAGE_FONT					EMAIL_TEXT_FONT
#define EMAIL_HEADER_FONT			FONT14ARIAL
#define EMAIL_WARNING_FONT		FONT12ARIAL


// the max number of pages to an email
#define MAX_NUMBER_EMAIL_PAGES 100

#define PREVIOUS_PAGE 0
#define NEXT_PAGE     1

#define NEXT_PAGE_X LAPTOP_UL_X + 562
#define NEXT_PAGE_Y 51

#define PREVIOUS_PAGE_X NEXT_PAGE_X - 21
#define PREVIOUS_PAGE_Y NEXT_PAGE_Y

#define ENVELOPE_BOX_X 116

#define FROM_BOX_X 166
#define FROM_BOX_WIDTH 246-160

#define SUBJECT_BOX_X 276
#define SUBJECT_BOX_WIDTH 528-249

#define DATE_BOX_X 530
#define DATE_BOX_WIDTH 594-530

#define FROM_BOX_Y 51 + EMAIL_TOP_BAR_HEIGHT
#define TOP_HEIGHT 118-95

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
enum{
	PREVIOUS_BUTTON=0,
	NEXT_BUTTON,
};


// X button position
#define BUTTON_X VIEWER_X + 396
#define BUTTON_Y VIEWER_Y + 3 // was + 25
#define BUTTON_LOWER_Y BUTTON_Y + 22
#define PREVIOUS_PAGE_BUTTON_X VIEWER_X + 302
#define NEXT_PAGE_BUTTON_X VIEWER_X +395
#define DELETE_BUTTON_X NEXT_PAGE_BUTTON_X
#define LOWER_BUTTON_Y BUTTON_Y + 299


BOOLEAN fSortDateUpwards = FALSE;
BOOLEAN fSortSenderUpwards = FALSE;
BOOLEAN fSortSubjectUpwards = FALSE;
BOOLEAN gfPageButtonsWereCreated = FALSE;

// mouse regions
MOUSE_REGION pEmailRegions[MAX_MESSAGES_PAGE];
MOUSE_REGION pScreenMask;
MOUSE_REGION pDeleteScreenMask;

// the email info struct to speed up email
EmailPageInfoStruct pEmailPageInfo[ MAX_NUMBER_EMAIL_PAGES ];

//buttons
INT32 giMessageButton[MAX_BUTTON_COUNT];
INT32 giMessageButtonImage[MAX_BUTTON_COUNT];
INT32 giDeleteMailButton[2];
INT32 giDeleteMailButtonImage[2];
INT32 giSortButton[4];
INT32 giSortButtonImage[4];
INT32 giNewMailButton[1];
INT32 giNewMailButtonImage[1];
INT32 giMailMessageButtons[3];
INT32 giMailMessageButtonsImage[3];
INT32 giMailPageButtons[ 2 ];
INT32 giMailPageButtonsImage[ 2 ];


// mouse regions
MOUSE_REGION pEmailMoveRegions[NEXT_BUTTON+1];
MOUSE_REGION pSortMailRegions[3];

// the message record list, for the currently displayed message
RecordPtr pMessageRecordList=NULL;

// video handles
UINT32 guiEmailTitle;
UINT32 guiEmailStamp;
UINT32 guiEmailBackground;
UINT32 guiEmailIndicator;
UINT32 guiEmailMessage;
UINT32 guiMAILDIVIDER;



// the enumeration of headers
enum{
	FROM_HEADER=0,
	SUBJECT_HEADER,
	RECD_HEADER,
};


// position of header text on the email list
#define FROM_X 205
#define FROM_Y FROM_BOX_Y + 5
#define SUBJECTHEAD_X 368
#define RECD_X 550

// size of prev/next strings
#define PREVIOUS_WIDTH StringPixLength(pTraverseStrings[PREVIOUS_BUTTON], TRAVERSE_EMAIL_FONT)
#define NEXT_WIDTH StringPixLength(pTraverseStrings[NEXT_BUTTON], TRAVERSE_EMAIL_FONT)
#define PREVIOUS_HEIGHT GetFontHeight(TRAVERSE_EMAIL_FONT)
#define NEXT_HEIGHT GetFontHeight(TRAVERSE_EMAIL_FONT)


// current line in the email list that is highlighted, -1 is no line highlighted
INT32 iHighLightLine=-1;

// whther or not we need to redraw the new mail box
BOOLEAN fReDrawNewMailFlag = FALSE;
INT32 giNumberOfMessageToEmail = 0;
INT32 iTotalHeight = 0;

// function list
void SwapMessages(INT32 iIdA, INT32 iIdB);
void PlaceMessagesinPages();
BOOLEAN	fFirstTime=TRUE;
void EmailBtnCallBack(MOUSE_REGION * pRegion, INT32 iReason );
void EmailMvtCallBack(MOUSE_REGION * pRegion, INT32 iReason );
void PreviousRegionButtonCallback(GUI_BUTTON *btn,INT32 reason);
void NextRegionButtonCallback(GUI_BUTTON *btn,INT32 reason);
void SetUnNewMessages();
INT32 DisplayEmailMessage(EmailPtr pMail);
void AddDeleteRegionsToMessageRegion();
void DetermineNextPrevPageDisplay();
void CreateDestroyNextPreviousRegions();
void ReDraw();
void DeleteEmail();
BOOLEAN DisplayDeleteNotice(EmailPtr pMail);
void CreateDestroyDeleteNoticeMailButton();
void SetUpSortRegions();
void DeleteSortRegions();
void DisplayTextOnTitleBar( void );
void DisplayEmailMessageSubjectDateFromLines( EmailPtr pMail, INT32 iViewerY );
void DrawEmailMessageDisplayTitleText( INT32 iViewerY );
void CreateMailScreenButtons( void );
void DestroyMailScreenButtons( void );
void DrawLineDividers( void );
void FromCallback(GUI_BUTTON *btn, INT32 iReason );
void SubjectCallback(GUI_BUTTON *btn,  INT32 iReason );
void DateCallback(GUI_BUTTON *btn,  INT32 iReason );
void ReadCallback(GUI_BUTTON *btn,  INT32 iReason );
void BtnPreviousEmailPageCallback(GUI_BUTTON *btn,INT32 reason);
void BtnNextEmailPageCallback(GUI_BUTTON *btn,INT32 reason);
void DisplayEmailList();
void ClearOutEmailMessageRecordsList( void );
void AddEmailRecordToList( STR16 pString );
void UpDateMessageRecordList( void );
void HandleAnySpecialEmailMessageEvents(INT32 iMessageId );
BOOLEAN HandleMailSpecialMessages( UINT16 usMessageId, INT32 *iResults,  EmailPtr pMail );
void HandleIMPCharProfileResultsMessage(  void );
void HandleEmailViewerButtonStates( void );
void SetUpIconForButton( void );
void DeleteCurrentMessage( void );
void BtnDeleteCallback(GUI_BUTTON *btn, INT32 iReason );
void CreateNextPreviousEmailPageButtons( void );
void UpdateStatusOfNextPreviousButtons( void );
void DisplayWhichPageOfEmailProgramIsDisplayed( void );
void OpenMostRecentUnreadEmail( void );
BOOLEAN DisplayNumberOfPagesToThisEmail( INT32 iViewerY );
INT32 GetNumberOfPagesToEmail( );
void PreProcessEmail( EmailPtr pMail );
void ModifyInsuranceEmails( UINT16 usMessageId, INT32 *iResults, EmailPtr pMail, UINT8 ubNumberOfRecords );
BOOLEAN ReplaceMercNameAndAmountWithProperData( CHAR16 *pFinishedString, EmailPtr pMail );



void InitializeMouseRegions()
{
	INT32 iCounter=0;

	// init mouseregions
	for(iCounter=0; iCounter <MAX_MESSAGES_PAGE; iCounter++)
	{
	 MSYS_DefineRegion(&pEmailRegions[iCounter],MIDDLE_X ,((INT16)(MIDDLE_Y+iCounter*MIDDLE_WIDTH)), MIDDLE_X+LINE_WIDTH ,(INT16)(MIDDLE_Y+iCounter*MIDDLE_WIDTH+MIDDLE_WIDTH),
			MSYS_PRIORITY_NORMAL+2,MSYS_NO_CURSOR, EmailMvtCallBack, EmailBtnCallBack );
	  MSYS_AddRegion(&pEmailRegions[iCounter]);
		MSYS_SetRegionUserData(&pEmailRegions[iCounter],0,iCounter);
	}

	//SetUpSortRegions();

	CreateDestroyNextPreviousRegions();
}

void DeleteEmailMouseRegions()
{

	// this function will remove the mouse regions added
	INT32 iCounter=0;


	for(iCounter=0; iCounter <MAX_MESSAGES_PAGE; iCounter++)
	{
	 MSYS_RemoveRegion( &pEmailRegions[iCounter]);
	}
  //DeleteSortRegions();
  CreateDestroyNextPreviousRegions();

}
void GameInitEmail()
{
  pEmailList=NULL;
	pPageList=NULL;

	iLastPage=-1;

	iCurrentPage=0;
	iDeleteId=0;

	// reset display message flag
	fDisplayMessageFlag=FALSE;

	 // reset page being displayed
   giMessagePage = 0;
}

BOOLEAN EnterEmail()
{
  VOBJECT_DESC    VObjectDesc;
  // load graphics

	iCurrentPage = LaptopSaveInfo.iCurrentEmailPage;

	// title bar
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\programtitlebar.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiEmailTitle));

	// the list background
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\Mailwindow.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiEmailBackground));

	// the indication/notification box
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\MailIndicator.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiEmailIndicator));

	// the message background
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\emailviewer.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiEmailMessage));

  // the message background
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\maillistdivider.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiMAILDIVIDER));

  //AddEmail(IMP_EMAIL_PROFILE_RESULTS, IMP_EMAIL_PROFILE_RESULTS_LENGTH, IMP_PROFILE_RESULTS, GetWorldTotalMin( ) );
	// initialize mouse regions
	InitializeMouseRegions();

	// just started email
	fJustStartedEmail = TRUE;

	// create buttons
	CreateMailScreenButtons( );

	// marks these buttons dirty
	MarkButtonsDirty( );

	// no longer fitrst time in email
	fFirstTime = FALSE;

	// reset current page of the message being displayed
	giMessagePage = 0;

	// render email background and text
	RenderEmail();


	//AddEmail( MERC_REPLY_GRIZZLY, MERC_REPLY_LENGTH_GRIZZLY, GRIZZLY_MAIL, GetWorldTotalMin() );
	//RenderButtons( );


	return( TRUE );
}

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
		giMessageId = -1;
	}

	// delete mail notice?...get rid of it
	if(fDeleteMailFlag)
	{
   fDeleteMailFlag=FALSE;
	 CreateDestroyDeleteNoticeMailButton();
	}

	// remove all mouse regions in use in email
  DeleteEmailMouseRegions();

	// reset flags of new messages
	SetUnNewMessages();

	// remove video objects being used by email screen
	DeleteVideoObjectFromIndex(guiEmailTitle);
  DeleteVideoObjectFromIndex(guiEmailBackground);
  DeleteVideoObjectFromIndex(guiMAILDIVIDER);
  DeleteVideoObjectFromIndex(guiEmailIndicator);
  DeleteVideoObjectFromIndex(guiEmailMessage);


	// remove buttons
  DestroyMailScreenButtons( );


}

void HandleEmail( void )
{

	INT32 iViewerY = 0;
	static BOOLEAN fEmailListBeenDrawAlready = FALSE;
  //RenderButtonsFastHelp( );


	// check if email message record list needs to be updated
  UpDateMessageRecordList( );

	// does email list need to be draw, or can be drawn
	if( ( (!fDisplayMessageFlag)&&(!fNewMailFlag) && ( !fDeleteMailFlag ) )&&( fEmailListBeenDrawAlready == FALSE ) )
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
    iViewerY = DisplayEmailMessage(GetEmailMessage(giMessageId));
		fEmailListBeenDrawAlready = FALSE;

	}
	else if((fDisplayMessageFlag)&&(!fOldDisplayMessageFlag))
	{

		// redisplay list
		DisplayEmailList();

		// this simply redraws message with button manipulation
		iViewerY = DisplayEmailMessage(GetEmailMessage(giMessageId));
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
	if(fDeleteMailFlag)
   DisplayDeleteNotice(GetEmailMessage(iDeleteId));


	// update buttons
	HandleEmailViewerButtonStates( );

	// set up icons for buttons
	SetUpIconForButton( );

	// redraw screen
	//ReDraw();

	//redraw headers to sort buttons
  DisplayEmailHeaders( );


	// handle buttons states
	UpdateStatusOfNextPreviousButtons( );

	if( fOpenMostRecentUnReadFlag == TRUE )
	{
		// enter email due to email icon on program panel
		OpenMostRecentUnreadEmail( );
		fOpenMostRecentUnReadFlag = FALSE;

	}

	return;
}

void DisplayEmailHeaders( void )
{
  // draw the text at the top of the screen

	// font stuff
	SetFont(EMAIL_WARNING_FONT);
	SetFontShadow(NO_SHADOW);
	SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);


	// draw headers to the email list the player sees

	// sender text
	//mprintf(FROM_X, FROM_Y, pEmailHeaders[FROM_HEADER]);

	// subject text
	//mprintf(SUBJECTHEAD_X, FROM_Y, pEmailHeaders[SUBJECT_HEADER]);

	// date re'vd
	//mprintf(RECD_X, FROM_Y, pEmailHeaders[RECD_HEADER]);

	// reset shadow
	SetFontShadow(DEFAULT_SHADOW);


	return;
}

void RenderEmail( void )
{
  HVOBJECT hHandle;
  INT32 iCounter=0;

	// get and blt the email list background
  GetVideoObject( &hHandle, guiEmailBackground );
  BltVideoObject( FRAME_BUFFER, hHandle, 0,LAPTOP_SCREEN_UL_X, EMAIL_LIST_WINDOW_Y+LAPTOP_SCREEN_UL_Y, VO_BLT_SRCTRANSPARENCY,NULL);


	// get and blt the email title bar
  GetVideoObject( &hHandle, guiEmailTitle );
  BltVideoObject( FRAME_BUFFER, hHandle, 0,LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y - 2, VO_BLT_SRCTRANSPARENCY,NULL );

	// show text on titlebar
	DisplayTextOnTitleBar( );

	// redraw list if no graphics are being displayed on top of it
  //if((!fDisplayMessageFlag)&&(!fNewMailFlag))
	//{
  DisplayEmailList( );
  //}

  // redraw line dividers
  DrawLineDividers( );


	// show next/prev page buttons depending if there are next/prev page
  //DetermineNextPrevPageDisplay( );

  // draw headers for buttons
	DisplayEmailHeaders();

	// display border
	GetVideoObject(&hHandle, guiLaptopBACKGROUND);
	BltVideoObject(FRAME_BUFFER, hHandle, 0,108, 23, VO_BLT_SRCTRANSPARENCY,NULL);


	ReDisplayBoxes( );

	BlitTitleBarIcons(  );



	// show which page we are on
	DisplayWhichPageOfEmailProgramIsDisplayed( );


	InvalidateRegion(0,0,640,480);
	 // invalidate region to force update
	 return;
}

void AddEmailWithSpecialData(INT32 iMessageOffset, INT32 iMessageLength, UINT8 ubSender, INT32 iDate, INT32 iFirstData, UINT32 uiSecondData )
{
	wchar_t pSubject[320];
	//MessagePtr pMessageList;
	//MessagePtr pMessage;
	//wchar_t pMessageString[320];
	INT32 iPosition=0;
	INT32 iCounter=1;
	Email	FakeEmail;


	// starts at iSubjectOffset amd goes iSubjectLength, reading in string
	LoadEncryptedDataFromFile("BINARYDATA\\Email.edt", pSubject, 640*(iMessageOffset), 640);

	//Make a fake email that will contain the codes ( ie the merc ID )
	FakeEmail.iFirstData = iFirstData;
	FakeEmail.uiSecondData = uiSecondData;

	//Replace the $mercname$ with the actual mercname
	ReplaceMercNameAndAmountWithProperData( pSubject, &FakeEmail );

	// add message to list
	AddEmailMessage(iMessageOffset,iMessageLength, pSubject, iDate, ubSender, FALSE, iFirstData, uiSecondData );

	// if we are in fact int he laptop, redraw icons, might be change in mail status

	if( fCurrentlyInLaptop == TRUE )
	{
	  // redraw icons, might be new mail
	  DrawLapTopIcons();
	}

	return;
}

void AddEmail(INT32 iMessageOffset, INT32 iMessageLength, UINT8 ubSender, INT32 iDate)
{
	wchar_t pSubject[320];
	//MessagePtr pMessageList;
	//MessagePtr pMessage;
	//wchar_t pMessageString[320];
	INT32 iPosition=0;
	INT32 iCounter=1;


	// starts at iSubjectOffset amd goes iSubjectLength, reading in string
	LoadEncryptedDataFromFile("BINARYDATA\\Email.edt", pSubject, 640*(iMessageOffset), 640);

	// add message to list
	AddEmailMessage(iMessageOffset,iMessageLength, pSubject, iDate, ubSender, FALSE, 0, 0 );

	// if we are in fact int he laptop, redraw icons, might be change in mail status

	if( fCurrentlyInLaptop == TRUE )
	{
	  // redraw icons, might be new mail
	  DrawLapTopIcons();
	}

	return;
}

void AddPreReadEmail(INT32 iMessageOffset, INT32 iMessageLength, UINT8 ubSender, INT32 iDate)
{
	wchar_t pSubject[320];
	//MessagePtr pMessageList;
	//MessagePtr pMessage;
	//wchar_t pMessageString[320];
	INT32 iPosition=0;
	INT32 iCounter=1;


	// starts at iSubjectOffset amd goes iSubjectLength, reading in string
	LoadEncryptedDataFromFile("BINARYDATA\\Email.edt", pSubject, 640*(iMessageOffset), 640);

	// add message to list
	AddEmailMessage(iMessageOffset,iMessageLength, pSubject, iDate, ubSender, TRUE, 0, 0 );

	// if we are in fact int he laptop, redraw icons, might be change in mail status

	if( fCurrentlyInLaptop == TRUE )
	{
	  // redraw icons, might be new mail
	  DrawLapTopIcons();
	}

	return;
}

void AddEmailMessage(INT32 iMessageOffset, INT32 iMessageLength,STR16 pSubject, INT32 iDate, UINT8 ubSender, BOOLEAN fAlreadyRead, INT32 iFirstData, UINT32 uiSecondData )
{
	// will add a message to the list of messages
	EmailPtr pEmail=pEmailList;
	EmailPtr pTempEmail=NULL;
	UINT32 iCounter=0;
	INT32 iId=0;

	// run through list of messages, get id of oldest message
	if(pEmail)
	{
	  while(pEmail)
		{
		  if(pEmail->iId >iId)
			  iId=pEmail->iId;
		  pEmail=pEmail->Next;
		}
	}

	// reset pEmail
	pEmail=pEmailList;

	// move to end of list
	if( pEmail )
	{
	  while( pEmail -> Next)
      pEmail = pEmail -> Next;
  }

	// add new element onto list
  pTempEmail=MemAlloc(sizeof(Email));
	// add in strings
	//while(pMessage !=NULL)
	//{
	 //pTempEmail->pText[iCounter]=MemAlloc((wcslen(pMessage->pString)+2)*2);
   //wcscpy(pTempEmail->pText[iCounter],pMessage->pString);
	 //pMessage=pMessage->Next;
	 //iCounter++;
	//}
  //pTempEmail->pText[iCounter]=NULL;

	// copy subject
	pTempEmail->pSubject=MemAlloc( 128 * 2 );
	memset( pTempEmail->pSubject, 0, sizeof( CHAR16 ) * 128 );
	wcscpy(pTempEmail->pSubject,pSubject);

	// copy offset and length of the actual message in email.edt
	pTempEmail->usOffset =(UINT16)iMessageOffset;
	pTempEmail->usLength =(UINT16)iMessageLength;

	// null out last byte of subject
  pTempEmail->pSubject[wcslen(pSubject)+1]=0;


	// set date and sender, Id
	if(pEmail)
	  pTempEmail->iId=iId+1;
	else
		pTempEmail->iId=0;

	// copy date and sender id's
	pTempEmail->iDate=iDate;
  pTempEmail->ubSender=ubSender;

	// the special data
	pTempEmail->iFirstData = iFirstData;
  pTempEmail->uiSecondData = uiSecondData;

	// place into list
	if(pEmail)
	{
		// list exists, place at end
	  pEmail->Next=pTempEmail;
	  pTempEmail->Prev=pEmail;
	}
	else
	{
		// no list, becomes head of a new list
		pEmail=pTempEmail;
		pTempEmail->Prev=NULL;
		pEmailList=pEmail;
	}

	// reset Next ptr
	pTempEmail->Next=NULL;

	// set flag that new mail has arrived
  fNewMailFlag=TRUE;

	// add this message to the pages of email
  AddMessageToPages(pTempEmail->iId);

  // reset read flag of this particular message
  pTempEmail->fRead=fAlreadyRead;

	 // set fact this message is new
	pTempEmail->fNew=TRUE;
	return;
}


void RemoveEmailMessage(INT32 iId)
{
	// run through list and remove message, update everyone afterwards
	EmailPtr pEmail=pEmailList;
	EmailPtr pTempEmail=NULL;
	INT32 iCounter=0;


	// error check
	if(!pEmail)
		return;

	// look for message
	pEmail = GetEmailMessage( iId );
	//while((pEmail->iId !=iId)&&(pEmail -> Next))
	//	pEmail=pEmail->Next;

	// end of list, no mail found, leave
	if(!pEmail)
		return;
  // found

	// set tempt o current
	pTempEmail=pEmail;

  // check position of message in list
  if((pEmail->Prev)&&(pTempEmail->Next))
	{
		// in the middle of the list
	 pEmail=pEmail->Prev;
	 pTempEmail=pTempEmail->Next;
	 MemFree(pEmail->Next->pSubject);
   //while(pEmail->Next->pText[iCounter])
	 //{
   //MemFree(pEmail->Next->pText[iCounter]);
	 //iCounter++;
	 //}
	 MemFree(pEmail->Next);
	 pEmail->Next=pTempEmail;
	 pTempEmail->Prev=pEmail;
	}
	else if(pEmail->Prev)
	{
		// end of the list
		pEmail=pEmail->Prev;
	  MemFree(pEmail->Next->pSubject);
		//while(pEmail->Next->pText[iCounter])
		//{
     //MemFree(pEmail->Next->pText[iCounter]);
		 //iCounter++;
		//}
	  MemFree(pEmail->Next);
		pEmail->Next=NULL;
	}
	else if(pTempEmail->Next)
	{
		// beginning of the list
		pEmail=pTempEmail;
		pTempEmail=pTempEmail->Next;
	  MemFree(pEmail->pSubject);
    //while(pEmail->pText[iCounter])
		//{
    //MemFree(pEmail->pText[iCounter]);
		//iCounter++;
		//}
	  MemFree(pEmail);
		pTempEmail->Prev=NULL;
		pEmailList=pTempEmail;
	}
	else
	{
		// all alone
    MemFree(pEmail->pSubject);
	//	while(pEmail->pText[iCounter])
		//{
     //MemFree(pEmail->pText[iCounter]);
		 //iCounter++;
		//}
	  MemFree(pEmail);
    pEmailList=NULL;
	}
}

EmailPtr GetEmailMessage(INT32 iId)
{
	EmailPtr pEmail=pEmailList;
	// return pointer to message with iId

	// invalid id
	if(iId==-1)
		return NULL;

	// invalid list
	if( pEmail == NULL )
	{
		return NULL;
	}

  // look for message
	while( (pEmail->iId !=iId)&&(pEmail->Next) )
		pEmail=pEmail->Next;

	if( ( pEmail ->iId != iId ) && ( pEmail->Next == NULL ) )
	{
		pEmail = NULL;
	}

	// no message, or is there?
	if(!pEmail)
		return NULL;
	else
		return pEmail;
}


void AddEmailPage()
{
	// simple adds a page to the list
	PagePtr pPage=pPageList;
	if(pPage)
	{
	 while(pPage->Next)
		 pPage=pPage->Next;
	}


	if(pPage)
	{

		// there is a page, add current page after it
		pPage->Next=MemAlloc(sizeof(Page));
		pPage->Next->Prev=pPage;
    pPage=pPage->Next;
		pPage->Next=NULL;
		pPage->iPageId=pPage->Prev->iPageId+1;
	  memset(pPage->iIds, -1, sizeof(INT32) * MAX_MESSAGES_PAGE );
	}
	else
	{

		// page becomes head of page list
		pPageList=MemAlloc(sizeof(Page));
		pPage=pPageList;
		pPage->Prev=NULL;
		pPage->Next=NULL;
		pPage->iPageId=0;
    memset(pPage->iIds, -1, sizeof(INT32) * MAX_MESSAGES_PAGE );
    pPageList=pPage;
	}
	iLastPage++;
	return;

}


void RemoveEmailPage(INT32 iPageId)
{
  PagePtr pPage=pPageList;
	PagePtr pTempPage=NULL;

	// run through list until page is matched, or out of pages
	while((pPage->iPageId !=iPageId)&&(pPage))
		pPage=pPage->Next;

	// error check
	if(!pPage)
		return;


  // found
	pTempPage=pPage;
  if((pPage->Prev)&&(pTempPage->Next))
	{
		// in the middle of the list
	 pPage=pPage->Prev;
	 pTempPage=pTempPage->Next;
	 MemFree(pPage->Next);
	 pPage->Next=pTempPage;
	 pTempPage->Prev=pPage;
	}
	else if(pPage->Prev)
	{
		// end of the list
		pPage=pPage->Prev;
	  MemFree(pPage->Next);
		pPage->Next=NULL;
	}
	else if(pTempPage->Next)
	{
		// beginning of the list
		pPage=pTempPage;
		pTempPage=pTempPage->Next;
	  MemFree(pPage);
		pTempPage->Prev=NULL;
	}
	else
	{
		// all alone

	  MemFree(pPage);
    pPageList=NULL;
	}
  if(iLastPage !=0)
	 iLastPage--;
}

void AddMessageToPages(INT32 iMessageId)
{
	// go to end of page list
	PagePtr pPage=pPageList;
	INT32 iCounter=0;
	if(!pPage)
   AddEmailPage();
	pPage=pPageList;
	while((pPage->Next)&&(pPage->iIds[MAX_MESSAGES_PAGE-1]!=-1))
		pPage=pPage->Next;
	// if list is full, add new page
  while(iCounter <MAX_MESSAGES_PAGE)
	{
		if(pPage->iIds[iCounter]==-1)
			break;
		iCounter++;
	}
	if(iCounter==MAX_MESSAGES_PAGE)
  {
		AddEmailPage();
		AddMessageToPages(iMessageId);
	  return;
	}
  else
	{
		pPage->iIds[iCounter]=iMessageId;
	}
	return;
}

void SortMessages(INT32 iCriteria)
{
  EmailPtr pA=pEmailList;
	EmailPtr pB=pEmailList;
	CHAR16 pSubjectA[256];
	CHAR16 pSubjectB[256];
  INT32 iId=0;

	// no messages to sort?
	if( ( pA == NULL) ||( pB == NULL ) )
	{
		return;
	}

  // nothing here either?
	if(!pA->Next)
		return;

	pB=pA->Next;
	switch(iCriteria)
	{
		case RECEIVED:
      while(pA)
			{

				// set B to next in A
				pB=pA -> Next;
				while(pB)
				{

					if( fSortDateUpwards )
					{
							// if date is lesser, swap
					  if(pA->iDate > pB->iDate)
						  SwapMessages(pA->iId, pB->iId);
					}
					else
					{
						// if date is lesser, swap
					  if(pA->iDate < pB->iDate)
						  SwapMessages(pA->iId, pB->iId);
					}


					// next in B's list
					pB=pB->Next;
				}

				// next in A's List
	      pA=pA->Next;
			}
			break;
		case SENDER:
			 while(pA)
			{

				 pB = pA ->Next;
				while(pB)
				{
          // lesser string?...need sorting
					if( fSortSenderUpwards )
					{
					   if(( wcscmp( pSenderNameList[pA->ubSender] , pSenderNameList[pB->ubSender] ) ) < 0 )
						   SwapMessages(pA->iId, pB->iId);
					}
					else
					{
						if(( wcscmp( pSenderNameList[pA->ubSender] , pSenderNameList[pB->ubSender] ) ) > 0 )
						  SwapMessages(pA->iId, pB->iId);
					}
					// next in B's list
					pB=pB->Next;
				}
			  // next in A's List
				pA=pA->Next;
			}
			break;
    case SUBJECT:
			 while(pA)
			{

				pB = pA ->Next;
				while(pB)
				{
					// clear out control codes
					CleanOutControlCodesFromString( pA->pSubject,  pSubjectA );
          CleanOutControlCodesFromString( pB->pSubject,  pSubjectB );

					// lesser string?...need sorting
					if( fSortSubjectUpwards )
					{
					  if( ( wcscmp( pA->pSubject ,pB->pSubject ) ) < 0)
						  SwapMessages(pA->iId, pB->iId);
          }
					else
					{
						if( ( wcscmp( pA->pSubject ,pB->pSubject ) ) > 0)
						  SwapMessages(pA->iId, pB->iId);
					}
					// next in B's list
					pB=pB->Next;
				}
				// next in A's List
	      pA=pA->Next;
			}
			break;

    case READ:
			 while(pA)
			{

				pB = pA ->Next;
				while(pB)
				{
					// one read and another not?...need sorting
					if( ( pA->fRead ) && ( ! ( pB -> fRead ) ) )
						SwapMessages(pA->iId, pB->iId);

					// next in B's list
					pB=pB->Next;
				}
				// next in A's List
	      pA=pA->Next;
			}
			break;
	}


	// place new list into pages of email
  //PlaceMessagesinPages();

	// redraw the screen
	fReDrawScreenFlag=TRUE;
}

void SwapMessages(INT32 iIdA, INT32 iIdB)
{
 // swaps locations of messages in the linked list
 EmailPtr pA=pEmailList;
 EmailPtr pB=pEmailList;
 EmailPtr pTemp=MemAlloc(sizeof(Email) );
 pTemp->pSubject= MemAlloc( 128 * 2 );

 memset( pTemp->pSubject, 0, sizeof( CHAR16 ) * 128 );

 if(!pA->Next)
	 return;
 //find pA
 while(pA->iId!=iIdA)
	 pA=pA->Next;
 // find pB
 while(pB->iId!=iIdB)
	 pB=pB->Next;

 // swap

 // pTemp becomes pA
 pTemp->iId=pA->iId;
 pTemp->fRead=pA->fRead;
 pTemp->fNew=pA->fNew;
 pTemp->usOffset=pA->usOffset;
 pTemp->usLength=pA->usLength;
 pTemp->iDate=pA->iDate;
 pTemp->ubSender=pA->ubSender;
 wcscpy(pTemp->pSubject,pA->pSubject);

 // pA becomes pB
 pA->iId=pB->iId;
 pA->fRead=pB->fRead;
 pA->fNew=pB->fNew;
 pA->usOffset=pB->usOffset;
 pA->usLength=pB->usLength;
 pA->iDate=pB->iDate;
 pA->ubSender=pB->ubSender;
 wcscpy(pA->pSubject, pB->pSubject);

// pB becomes pTemp
 pB->iId=pTemp->iId;
 pB->fRead=pTemp->fRead;
 pB->fNew=pTemp->fNew;
 pB->usOffset=pTemp->usOffset;
 pB->usLength=pTemp->usLength;
 pB->iDate=pTemp->iDate;
 pB->ubSender=pTemp->ubSender;
 wcscpy(pB->pSubject, pTemp->pSubject);

 // free up memory
 MemFree(pTemp -> pSubject);
 MemFree( pTemp );
 return;
}

void ClearPages()
{
	// run through list of message pages and set to -1
	PagePtr pPage=pPageList;

	// error check
	if( pPageList == NULL )
	{
		return;
	}

	while(pPage->Next)
	{
		pPage=pPage->Next;
		MemFree(pPage->Prev);
	}
	if(pPage)
   MemFree(pPage);
	pPageList=NULL;
	iLastPage=-1;

	return;
}

void PlaceMessagesinPages()
{
	EmailPtr pEmail=pEmailList;
	// run through the list of messages and add to pages
	ClearPages();
	while(pEmail)
	{
		AddMessageToPages(pEmail->iId);
		pEmail=pEmail->Next;

	}
	if(iCurrentPage >iLastPage)
		iCurrentPage=iLastPage;
	return;
}

void DisplayMessageList(INT32 iPageNum)
{
	// will display page with idNumber iPageNum
	PagePtr pPage=pPageList;
	while(pPage->iPageId!=iPageNum)
	{
		pPage=pPage->Next;
		if(!pPage)
			return;
	}
	// found page show it
	return;
}

void DrawLetterIcon(INT32 iCounter, BOOLEAN fRead)
{
  HVOBJECT hHandle;
  // will draw the icon for letter in mail list depending if the mail has been read or not

	// grab video object
	GetVideoObject(&hHandle, guiEmailIndicator);

	// is it read or not?
	if(fRead)
	 BltVideoObject(FRAME_BUFFER, hHandle, 0,INDIC_X, (MIDDLE_Y+iCounter*MIDDLE_WIDTH+2), VO_BLT_SRCTRANSPARENCY,NULL);
	else
   BltVideoObject(FRAME_BUFFER, hHandle, 1,INDIC_X, (MIDDLE_Y+iCounter*MIDDLE_WIDTH+2), VO_BLT_SRCTRANSPARENCY,NULL);
	return;
}

void DrawSubject(INT32 iCounter, STR16 pSubject, BOOLEAN fRead)
{
	wchar_t pTempSubject[320];


	// draw subject line of mail being viewed in viewer

	// lock buffer to prevent overwrite
  SetFontDestBuffer(FRAME_BUFFER, SUBJECT_X , ((UINT16)(MIDDLE_Y+iCounter*MIDDLE_WIDTH)) , SUBJECT_X  + SUBJECT_WIDTH , ( ( UINT16 ) ( MIDDLE_Y + iCounter * MIDDLE_WIDTH ) ) + MIDDLE_WIDTH,  FALSE  );
	SetFontShadow(NO_SHADOW);
	SetFontForeground( FONT_BLACK );
	SetFontBackground( FONT_BLACK );


	wcscpy( pTempSubject, pSubject );

	if( fRead )
	{
		//if the subject will be too long, cap it, and add the '...'
		if( StringPixLength( pTempSubject, MESSAGE_FONT ) >= SUBJECT_WIDTH - 10 )
		{
			ReduceStringLength( pTempSubject, SUBJECT_WIDTH - 10, MESSAGE_FONT );
		}

	  // display string subject
	  IanDisplayWrappedString(SUBJECT_X, (( UINT16 )( 4 + MIDDLE_Y + iCounter * MIDDLE_WIDTH ) ) , SUBJECT_WIDTH, MESSAGE_GAP, MESSAGE_FONT, MESSAGE_COLOR ,pTempSubject,0 ,FALSE ,LEFT_JUSTIFIED );
  }
	else
	{
		//if the subject will be too long, cap it, and add the '...'
		if( StringPixLength( pTempSubject, FONT10ARIALBOLD ) >= SUBJECT_WIDTH - 10 )
		{
			ReduceStringLength( pTempSubject, SUBJECT_WIDTH - 10, FONT10ARIALBOLD );
		}

		// display string subject
	  IanDisplayWrappedString(SUBJECT_X, (( UINT16 )( 4 + MIDDLE_Y + iCounter * MIDDLE_WIDTH ) ) , SUBJECT_WIDTH, MESSAGE_GAP, FONT10ARIALBOLD, MESSAGE_COLOR ,pTempSubject,0 ,FALSE ,LEFT_JUSTIFIED );

	}
	SetFontShadow(DEFAULT_SHADOW);
	// reset font dest buffer
	SetFontDestBuffer(FRAME_BUFFER, 0, 0, 640, 480, FALSE  );

	return;
}

void DrawSender(INT32 iCounter, UINT8 ubSender, BOOLEAN fRead)
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
	return;
}

void DrawDate(INT32 iCounter, INT32 iDate, BOOLEAN fRead)
{
	wchar_t sString[20];

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
  swprintf(sString, L"%s %d", pDayStrings[ 0 ], iDate/ ( 24 * 60 ) );
  mprintf(DATE_X,(( UINT16 )( 4 + MIDDLE_Y + iCounter * MIDDLE_WIDTH ) ),sString);

	SetFont( MESSAGE_FONT );
	SetFontShadow(DEFAULT_SHADOW);
	return;
}

void DisplayEmailList()
{
	INT32 iCounter=0;
	// look at current page, and display
  PagePtr pPage=pPageList;
  EmailPtr pEmail=NULL;


	// error check, if no page, return
	if(!pPage)
		return;

	// if current page ever ends up negative, reset to 0
	if(iCurrentPage==-1)
		iCurrentPage=0;

	// loop until we get to the current page
	while((pPage->iPageId!=iCurrentPage)&&(iCurrentPage <=iLastPage))
		pPage=pPage->Next;

	// now we have current page, display it
	pEmail=GetEmailMessage(pPage->iIds[iCounter]);
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
		 pEmail=GetEmailMessage(pPage->iIds[iCounter]);

	}




  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_LR_Y);

	SetFontShadow(DEFAULT_SHADOW);
	return;
}


void LookForUnread()
{
	BOOLEAN fStatusOfNewEmailFlag = fUnReadMailFlag;

	// simply runrs through list of messages, if any unread, set unread flag

  EmailPtr pA=pEmailList;

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

	return;
}

void EmailBtnCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
 INT32 iCount;
 PagePtr pPage=pPageList;
 INT32 iId=0;
 EmailPtr pEmail=NULL;
 if (iReason & MSYS_CALLBACK_REASON_INIT)
 {
	return;
 }
 if(fDisplayMessageFlag)
	 return;
 if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
 {

	 // error check
	 iCount=MSYS_GetRegionUserData(pRegion, 0);
   // check for valid email
   // find surrent page
	 if(!pPage)
		 return;
	 while((pPage->Next)&&(pPage->iPageId!=iCurrentPage))
		 pPage=pPage->Next;
	 if(!pPage)
		 return;
	 // found page

	 // get id for element iCount
	 iId=pPage->iIds[iCount];

	 // invalid message
	 if(iId==-1)
	 {
     fDisplayMessageFlag=FALSE;
		 return;
	 }
	 // Get email and display
	 fDisplayMessageFlag=TRUE;
   giMessagePage = 0;
   giPrevMessageId = giMessageId;
	 giMessageId=iId;


 }
 else if(iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
 {
   iCount=MSYS_GetRegionUserData(pRegion, 0);

	 // error check
	 if(!pPage)
	 {
		 HandleRightButtonUpEvent( );
		 return;
   }

 	 giMessagePage = 0;

   while((pPage->Next)&&(pPage->iPageId!=iCurrentPage))
		 pPage=pPage->Next;
	 if(!pPage)
	 {
		 HandleRightButtonUpEvent( );
		 return;
	 }
	 // found page
   // get id for element iCount
	 iId=pPage->iIds[iCount];
	 if(!GetEmailMessage(iId))
	 {
		 // no mail here, handle right button up event
		 HandleRightButtonUpEvent( );
		 return;
	 }
	 else
	 {
		 fDeleteMailFlag=TRUE;
		 iDeleteId=iId;
		 //DisplayDeleteNotice(GetEmailMessage(iDeleteId));
		 //DeleteEmail();
	 }
 }
}
void EmailMvtCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
		return;
	}
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

void BtnMessageXCallback(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if((reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )||(reason & MSYS_CALLBACK_REASON_RBUTTON_DWN))
	{

		btn->uiFlags |= BUTTON_CLICKED_ON;

	}
  else if((reason & MSYS_CALLBACK_REASON_LBUTTON_UP )||(reason & MSYS_CALLBACK_REASON_RBUTTON_UP))
	{

		if(btn->uiFlags& BUTTON_CLICKED_ON)
		{
		 // X button has been pressed and let up, this means to stop displaying the currently displayed message

     // reset display message flag
     fDisplayMessageFlag=FALSE;

		 // reset button flag
		 btn->uiFlags &= ~BUTTON_CLICKED_ON;

		 // reset page being displayed
     giMessagePage = 0;

		 // redraw icons
		 DrawLapTopIcons();

		 // force update of entire screen
		 fPausedReDrawScreenFlag=TRUE;

		 // rerender email
		 //RenderEmail();
		}
	}

}
void
SetUnNewMessages()
{
	// on exit from the mailer, set all new messages as 'un'new
  	EmailPtr pEmail=pEmailList;
	// run through the list of messages and add to pages

	while(pEmail)
	{
		pEmail->fNew=FALSE;
		pEmail=pEmail->Next;
	}
	return;
}

INT32 DisplayEmailMessage(EmailPtr pMail)
{
  HVOBJECT hHandle;
	INT32 iCnt=0;
	INT32 iHeight=0;
	INT32 iCounter=1;
//	wchar_t pString[MAIL_STRING_SIZE/2 + 1];
	wchar_t pString[MAIL_STRING_SIZE];
	INT32 iOffSet=0;
	INT32 iHeightTemp=0;
  INT32 iHeightSoFar = 0;
	RecordPtr pTempRecord;
	INT32 iPageSize = 0;
  INT32 iPastHeight=0;
	INT32 iYPositionOnPage = 0;
	INT32 iTotalYPosition = 0;
	BOOLEAN fGoingOffCurrentPage = FALSE;
	BOOLEAN fDonePrintingMessage = FALSE;



	if(!pMail)
		return 0;

  iOffSet=(INT32)pMail->usOffset;

  // reset redraw email message flag
	fReDrawMessageFlag = FALSE;

	// we KNOW the player is going to "read" this, so mark it as so
	pMail->fRead=TRUE;

	// draw text for title bar
  //swprintf(pString, L"%s / %s", pSenderNameList[pMail->ubSender],pMail->pSubject);
	//DisplayWrappedString(VIEWER_X+VIEWER_HEAD_X+4, VIEWER_Y+VIEWER_HEAD_Y+4, VIEWER_HEAD_WIDTH, MESSAGE_GAP, MESSAGE_FONT, MESSAGE_COLOR, pString, 0,FALSE,0);

	// increment height for size of one line
	iHeight+=GetFontHeight( MESSAGE_FONT );

	// is there any special event meant for this mail?..if so, handle it
	HandleAnySpecialEmailMessageEvents( iOffSet );

	HandleMailSpecialMessages( ( UINT16 ) ( iOffSet ), &iViewerPositionY, pMail);

	PreProcessEmail( pMail );


  pTempRecord = pMessageRecordList;



  // blt in top line of message as a blank graphic
	// get a handle to the bitmap of EMAIL VIEWER Background
	GetVideoObject( &hHandle, guiEmailMessage );

	// place the graphic on the frame buffer
	BltVideoObject( FRAME_BUFFER, hHandle, 1,VIEWER_X, VIEWER_MESSAGE_BODY_START_Y + iViewerPositionY, VO_BLT_SRCTRANSPARENCY,NULL );
  BltVideoObject( FRAME_BUFFER, hHandle, 1,VIEWER_X, VIEWER_MESSAGE_BODY_START_Y + GetFontHeight( MESSAGE_FONT ) + iViewerPositionY, VO_BLT_SRCTRANSPARENCY,NULL );

	// set shadow
	SetFontShadow(NO_SHADOW);

	// get a handle to the bitmap of EMAIL VIEWER
	GetVideoObject(&hHandle, guiEmailMessage);

	// place the graphic on the frame buffer
	BltVideoObject(FRAME_BUFFER, hHandle, 0,VIEWER_X, VIEWER_Y + iViewerPositionY, VO_BLT_SRCTRANSPARENCY,NULL);


	// the icon for the title of this box
	GetVideoObject( &hHandle, guiTITLEBARICONS );
  BltVideoObject( FRAME_BUFFER, hHandle, 0,VIEWER_X + 5, VIEWER_Y + iViewerPositionY + 2, VO_BLT_SRCTRANSPARENCY,NULL );

	// display header text
  DisplayEmailMessageSubjectDateFromLines( pMail, iViewerPositionY );

	// display title text
	DrawEmailMessageDisplayTitleText( iViewerPositionY );



  iCounter=0;
  // now blit the text background based on height
	for (iCounter=2; iCounter < ( ( iTotalHeight ) / ( GetFontHeight( MESSAGE_FONT ) ) ); iCounter++ )
	{
    // get a handle to the bitmap of EMAIL VIEWER Background
	  GetVideoObject( &hHandle, guiEmailMessage );

	  // place the graphic on the frame buffer
	  BltVideoObject( FRAME_BUFFER, hHandle, 1,VIEWER_X, iViewerPositionY + VIEWER_MESSAGE_BODY_START_Y+( (GetFontHeight( MESSAGE_FONT ) ) * ( iCounter )), VO_BLT_SRCTRANSPARENCY,NULL );

	}


	// now the bottom piece to the message viewer
  GetVideoObject( &hHandle, guiEmailMessage );

	if( giNumberOfPagesToCurrentEmail <= 2 )
	{
		// place the graphic on the frame buffer
		BltVideoObject( FRAME_BUFFER, hHandle, 2,VIEWER_X, iViewerPositionY + VIEWER_MESSAGE_BODY_START_Y+( ( GetFontHeight( MESSAGE_FONT ) ) * ( iCounter )), VO_BLT_SRCTRANSPARENCY,NULL );
	}
	else
	{
		// place the graphic on the frame buffer
		BltVideoObject( FRAME_BUFFER, hHandle, 3,VIEWER_X, iViewerPositionY + VIEWER_MESSAGE_BODY_START_Y+( ( GetFontHeight( MESSAGE_FONT ) ) * ( iCounter )), VO_BLT_SRCTRANSPARENCY,NULL );
	}

	// reset iCounter and iHeight
	iCounter = 1;
  iHeight = GetFontHeight(MESSAGE_FONT);

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


			// copy over string
			wcscpy( pString, pTempRecord -> pRecord );

			// get the height of the string, ONLY!...must redisplay ON TOP OF background graphic
			iHeight += IanDisplayWrappedString(VIEWER_X + MESSAGE_X + 4, ( UINT16 )( VIEWER_MESSAGE_BODY_START_Y + iHeight + iViewerPositionY), MESSAGE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, MESSAGE_COLOR,pString,0,FALSE, IAN_WRAP_NO_SHADOW);

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

	/*
	if(iTotalHeight < MAX_EMAIL_MESSAGE_PAGE_SIZE)
	{
		fOnLastPageFlag = TRUE;
	  while( pTempRecord )
		{
      // copy over string
		  wcscpy( pString, pTempRecord -> pRecord );

	    // get the height of the string, ONLY!...must redisplay ON TOP OF background graphic
	    iHeight += IanDisplayWrappedString(VIEWER_X + MESSAGE_X + 4, ( UINT16 )( VIEWER_MESSAGE_BODY_START_Y + iHeight + iViewerPositionY), MESSAGE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, MESSAGE_COLOR,pString,0,FALSE, IAN_WRAP_NO_SHADOW);

			// increment email record ptr
		  pTempRecord = pTempRecord -> Next;
		}


	}
	else
	{

		iYPositionOnPage = 0;
		// go to the right record
		pTempRecord = GetFirstRecordOnThisPage( pMessageRecordList, MESSAGE_FONT, MESSAGE_WIDTH, MESSAGE_GAP, giMessagePage, MAX_EMAIL_MESSAGE_PAGE_SIZE );
    while( pTempRecord )
		{
			// copy over string
		  wcscpy( pString, pTempRecord -> pRecord );

			if( pString[ 0 ] == 0 )
			{
				// on last page
				fOnLastPageFlag = TRUE;
			}


			if( ( iYPositionOnPage + IanWrappedStringHeight(0, 0, MESSAGE_WIDTH, MESSAGE_GAP,
															  MESSAGE_FONT, 0, pTempRecord->pRecord,
															 0, 0, 0 ) )  <= MAX_EMAIL_MESSAGE_PAGE_SIZE  )
			{
     	  // now print it
		    iYPositionOnPage += IanDisplayWrappedString(VIEWER_X + MESSAGE_X + 4, ( UINT16 )( VIEWER_MESSAGE_BODY_START_Y + 10 +iYPositionOnPage + iViewerPositionY), MESSAGE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, MESSAGE_COLOR,pString,0,FALSE, IAN_WRAP_NO_SHADOW);
				fGoingOffCurrentPage = FALSE;
			}
			else
			{
				// gonna get cut off...end now
				fGoingOffCurrentPage = TRUE;
			}


			pTempRecord = pTempRecord ->Next;


			if( ( pTempRecord == NULL ) && ( fGoingOffCurrentPage == FALSE ) )
			{
				// on last page
				fOnLastPageFlag = TRUE;
			}
			else
			{
				fOnLastPageFlag = FALSE;
			}

			// record get cut off?...end now

			if( fGoingOffCurrentPage == TRUE )
			{
				pTempRecord = NULL;
			}
		}

	}

	*/
	// show number of pages to this email
	DisplayNumberOfPagesToThisEmail( iViewerPositionY );

	// mark this area dirty
	InvalidateRegion( LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_LR_Y );


	// reset shadow
	SetFontShadow( DEFAULT_SHADOW );


	return iViewerPositionY;
}



void BtnNewOkback(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
		 btn->uiFlags&=~(BUTTON_CLICKED_ON);
		 fNewMailFlag=FALSE;

		}
	}
}

void AddDeleteRegionsToMessageRegion(INT32 iViewerY)
{
	// will create/destroy mouse region for message display

	if((fDisplayMessageFlag)&&(!fOldDisplayMessageFlag))
	{

		// set old flag
		fOldDisplayMessageFlag=TRUE;


		// add X button
    giMessageButtonImage[0]=  LoadButtonImage( "LAPTOP\\X.sti" ,-1,0,-1,1,-1 );
	  giMessageButton[0] = QuickCreateButton( giMessageButtonImage[0], BUTTON_X + 2,(INT16) ( BUTTON_Y + ( INT16 )iViewerY + 1),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnMessageXCallback );
	  SetButtonCursor(giMessageButton[0], CURSOR_LAPTOP_SCREEN);

		if( giNumberOfPagesToCurrentEmail > 2 )
		{
			// add next and previous mail page buttons
			giMailMessageButtonsImage[0]=  LoadButtonImage( "LAPTOP\\NewMailButtons.sti" ,-1,0,-1,3,-1 );
			giMailMessageButtons[0] = QuickCreateButton( giMailMessageButtonsImage[0], PREVIOUS_PAGE_BUTTON_X,(INT16) ( LOWER_BUTTON_Y + ( INT16 )iViewerY + 2),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnPreviousEmailPageCallback );

			giMailMessageButtonsImage[1]=  LoadButtonImage( "LAPTOP\\NewMailButtons.sti" ,-1,1,-1,4,-1 );
			giMailMessageButtons[1] = QuickCreateButton( giMailMessageButtonsImage[1], NEXT_PAGE_BUTTON_X ,(INT16) ( LOWER_BUTTON_Y + ( INT16 )iViewerY + 2),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnNextEmailPageCallback );

			gfPageButtonsWereCreated = TRUE;

		}

    giMailMessageButtonsImage[2]=  LoadButtonImage( "LAPTOP\\NewMailButtons.sti" ,-1,2,-1,5,-1 );
		giMailMessageButtons[2] = QuickCreateButton( giMailMessageButtonsImage[2], DELETE_BUTTON_X,(INT16) ( BUTTON_LOWER_Y + ( INT16 )iViewerY + 2),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnDeleteCallback );
/*
		// set up disable methods
		SpecifyDisabledButtonStyle( giMailMessageButtons[1], DISABLED_STYLE_SHADED );
    SpecifyDisabledButtonStyle( giMailMessageButtons[0], DISABLED_STYLE_SHADED );
*/
		// set cursors
		SetButtonCursor(giMailMessageButtons[0], CURSOR_LAPTOP_SCREEN);
		SetButtonCursor(giMailMessageButtons[1], CURSOR_LAPTOP_SCREEN);
    SetButtonCursor(giMailMessageButtons[2], CURSOR_LAPTOP_SCREEN);
		SetButtonCursor(giMessageButton[0], CURSOR_LAPTOP_SCREEN);

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
	giNewMailButtonImage[0]=LoadButtonImage( "LAPTOP\\YesNoButtons.sti" ,-1,0,-1,1,-1 );
  giNewMailButton[0]= QuickCreateButton( giNewMailButtonImage[0], NEW_BTN_X+10, NEW_BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST-2,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnNewOkback);

	// set cursor
	SetButtonCursor(giNewMailButton[0], CURSOR_LAPTOP_SCREEN);

	// set up screen mask region
	MSYS_DefineRegion(&pScreenMask,0, 0,640,480,
		MSYS_PRIORITY_HIGHEST-3,CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, LapTopScreenCallBack);
	MSYS_AddRegion(&pScreenMask);
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


	 //re draw screen
	 fReDraw=TRUE;

	 // redraw screen
		 fPausedReDrawScreenFlag=TRUE;
 }
}


BOOLEAN DisplayNewMailBox( void )
{



	HVOBJECT hHandle;
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



	// is set but already drawn, LEAVE NOW!
  //if( ( fNewMailFlag ) && ( fOldNewMailFlag ) )
	//	return ( FALSE );



  GetVideoObject( &hHandle, guiEmailWarning );
  BltVideoObject( FRAME_BUFFER, hHandle, 0,EMAIL_WARNING_X, EMAIL_WARNING_Y, VO_BLT_SRCTRANSPARENCY,NULL );


	// the icon for the title of this box
	GetVideoObject( &hHandle, guiTITLEBARICONS );
  BltVideoObject( FRAME_BUFFER, hHandle, 0,EMAIL_WARNING_X + 5, EMAIL_WARNING_Y + 2, VO_BLT_SRCTRANSPARENCY,NULL );

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
	DrawLapTopIcons( );

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

		// return;

		// reset flag for redraw
		  fReDrawNewMailFlag = FALSE;

		return;
	}
}

void SwitchEmailPages( void )
{
	// this function will switch current page

	// gone too far, reset page to start
	if( iCurrentPage >iLastPage )
		iCurrentPage=0;

	// set to last page
	if( iCurrentPage < 0 )
		iCurrentPage=iLastPage;
	return;

}


void DetermineNextPrevPageDisplay( void )
{
	// will determine which of previous and next page graphics to display



	if( iCurrentPage > 0 )
	{
		// display Previous graphic

		// font stuff
	  SetFont( TRAVERSE_EMAIL_FONT );
		SetFontForeground( FONT_RED );
		SetFontBackground( FONT_BLACK );

		// print previous string
    mprintf( PREVIOUS_PAGE_X, PREVIOUS_PAGE_Y, pTraverseStrings[PREVIOUS_PAGE] );
	}

	// less than last page, so there is a next page
	if(iCurrentPage <iLastPage)
	{
		// display Next graphic

		// font stuff
	  SetFont( TRAVERSE_EMAIL_FONT );
		SetFontForeground( FONT_RED );
		SetFontBackground( FONT_BLACK );

		// next string
	  mprintf( NEXT_PAGE_X, NEXT_PAGE_Y, pTraverseStrings[NEXT_PAGE] );
	}
}

void NextRegionButtonCallback(GUI_BUTTON *btn,INT32 reason )
{

  if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
		  btn->uiFlags&=~(BUTTON_CLICKED_ON);

		  // not on last page, move ahead one
			if(iCurrentPage <iLastPage)
			{
				iCurrentPage++;
				fReDraw=TRUE;
				RenderEmail();
				MarkButtonsDirty( );
			}
		}
 }
 else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP)
 {
  // nothing yet
 }

}

void BtnPreviousEmailPageCallback(GUI_BUTTON *btn,INT32 reason)
{
  if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
			if( giMessagePage > 0 )
			{
				giMessagePage--;
			}

			btn->uiFlags&=~(BUTTON_CLICKED_ON);

			fReDraw=TRUE;
			RenderEmail();
			MarkButtonsDirty( );
		}
  }
  else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP)
  {
    // nothing yet
  }

}

void BtnNextEmailPageCallback(GUI_BUTTON *btn,INT32 reason)
{
  if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
    // not on last page, move ahead one
		btn->uiFlags&=~(BUTTON_CLICKED_ON);

		if( ( giNumberOfPagesToCurrentEmail - 1 ) <= giMessagePage )
		{
			return;
		}

    if( ! ( fOnLastPageFlag ) )
		{
			if( ( giNumberOfPagesToCurrentEmail - 1 ) > ( giMessagePage + 1 ) )
				giMessagePage++;
		}

		MarkButtonsDirty( );
		fReDrawScreenFlag = TRUE;
  }
  else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP)
  {
    // nothing yet
  }

}

void PreviousRegionButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
 if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
		  btn->uiFlags&=~(BUTTON_CLICKED_ON);
			// if we are not on forst page, more back one
			if(iCurrentPage>0)
			{
				iCurrentPage--;
				fReDraw=TRUE;
				RenderEmail();
				MarkButtonsDirty( );
			}
		}
 }
 else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP)
 {
  // nothing yet
 }

}


void BtnDeleteNoback(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
		 btn->uiFlags&=~(BUTTON_CLICKED_ON);
		 fDeleteMailFlag=FALSE;
		 fReDrawScreenFlag=TRUE;
		}
	}
}


void BtnDeleteYesback(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
		 btn->uiFlags&=~(BUTTON_CLICKED_ON);
		 fReDrawScreenFlag=TRUE;
		 DeleteEmail();

		}
	}
}


void CreateDestroyNextPreviousRegions()
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

		/*
		// ' next' region
    MSYS_DefineRegion(&pEmailMoveRegions[NEXT_BUTTON],NEXT_PAGE_X, NEXT_PAGE_Y,(INT16) (NEXT_PAGE_X+NEXT_WIDTH), (INT16)(NEXT_PAGE_Y+NEXT_HEIGHT),
			MSYS_PRIORITY_NORMAL+2,MSYS_NO_CURSOR, MSYS_NO_CALLBACK, NextRegionButtonCallback);

		// ' previous ' region
	  MSYS_DefineRegion(&pEmailMoveRegions[PREVIOUS_BUTTON],PREVIOUS_PAGE_X,PREVIOUS_PAGE_Y, (INT16)(PREVIOUS_PAGE_X+PREVIOUS_WIDTH),(INT16)(PREVIOUS_PAGE_Y+PREVIOUS_HEIGHT),
			MSYS_PRIORITY_NORMAL+2,MSYS_NO_CURSOR, MSYS_NO_CALLBACK, PreviousRegionButtonCallback );

		// add regions
		MSYS_AddRegion(&pEmailMoveRegions[PREVIOUS_BUTTON]);
	  MSYS_AddRegion(&pEmailMoveRegions[NEXT_BUTTON]);
		*/
	}
}

void ReDraw()
{
	// forces update of entire laptop screen
	if(fReDraw)
	{
   RenderLaptop( );
	 //EnterNewLaptopMode();
   DrawLapTopText();
	 ReDrawHighLight();
	 MarkButtonsDirty( );
	 fReDraw=FALSE;
	}

}

void CreateDestroyDeleteNoticeMailButton()
{
 static BOOLEAN fOldDeleteMailFlag=FALSE;
 if((fDeleteMailFlag)&&(!fOldDeleteMailFlag))
 {
	 // confirm delete email buttons

	 // YES button
  fOldDeleteMailFlag=TRUE;
	giDeleteMailButtonImage[0]=LoadButtonImage( "LAPTOP\\YesNoButtons.sti" ,-1,0,-1,1,-1 );
  giDeleteMailButton[0]= QuickCreateButton( giDeleteMailButtonImage[0], NEW_BTN_X+1, NEW_BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 2,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnDeleteYesback);

	// NO button
	giDeleteMailButtonImage[1]=LoadButtonImage( "LAPTOP\\YesNoButtons.sti" ,-1,2,-1,3,-1 );
  giDeleteMailButton[1]= QuickCreateButton( giDeleteMailButtonImage[1], NEW_BTN_X+40, NEW_BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 2,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnDeleteNoback);

	// set up cursors
	SetButtonCursor(giDeleteMailButton[0], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(giDeleteMailButton[1], CURSOR_LAPTOP_SCREEN);

	// set up screen mask to prevent other actions while delete mail box is destroyed
	MSYS_DefineRegion(&pDeleteScreenMask,0, 0,640,480,
		MSYS_PRIORITY_HIGHEST-3,CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, LapTopScreenCallBack);
	MSYS_AddRegion(&pDeleteScreenMask);

	// force update
	fReDrawScreenFlag = TRUE;

 }
 else if((!fDeleteMailFlag)&&(fOldDeleteMailFlag))
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
	return;
}
BOOLEAN DisplayDeleteNotice(EmailPtr pMail)
{


	HVOBJECT hHandle;
	// will display a delete mail box whenever delete mail has arrived
	if(!fDeleteMailFlag)
		return(FALSE);

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


  // load graphics

  GetVideoObject(&hHandle, guiEmailWarning);
  BltVideoObject(FRAME_BUFFER, hHandle, 0,EMAIL_WARNING_X, EMAIL_WARNING_Y, VO_BLT_SRCTRANSPARENCY,NULL);


	// font stuff
	SetFont( EMAIL_HEADER_FONT );
	SetFontForeground( FONT_WHITE );
	SetFontBackground( FONT_BLACK );
	SetFontShadow( DEFAULT_SHADOW );

	// the icon for the title of this box
	GetVideoObject( &hHandle, guiTITLEBARICONS );
  BltVideoObject( FRAME_BUFFER, hHandle, 0,EMAIL_WARNING_X + 5, EMAIL_WARNING_Y + 2, VO_BLT_SRCTRANSPARENCY,NULL );

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

void DeleteEmail()
{

	// error check, invalid mail, or not time to delete mail
	if( fDeleteInternal != TRUE )
	{
	  if((iDeleteId==-1)||(!fDeleteMailFlag))
		  return;
	}
   // remove the message
   RemoveEmailMessage(iDeleteId);

	 // stop displaying message, if so
	 fDisplayMessageFlag = FALSE;

	 // upadte list
   PlaceMessagesinPages();

	 // redraw icons (if deleted message was last unread, remove checkmark)
	 DrawLapTopIcons();

	 // if all of a sudden we are beyond last page, move back one
	 if(iCurrentPage > iLastPage)
		 iCurrentPage=iLastPage;

	 // rerender mail list
	 RenderEmail();

	 // nolong time to delete mail
	 fDeleteMailFlag=FALSE;
   fReDrawScreenFlag=TRUE;
	 // refresh screen (get rid of dialog box image)
	 //ReDraw();

	 // invalidate
	 InvalidateRegion(0,0,640,480);
}



void FromCallback(GUI_BUTTON *btn, INT32 iReason )
{
 if (iReason & MSYS_CALLBACK_REASON_INIT)
 {
	return;
 }
 if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
 {

  // sort messages based on sender name, then replace into pages of email
	fSortSenderUpwards = !fSortSenderUpwards;

  SortMessages(SENDER);

	//SpecifyButtonIcon( giSortButton[1] , giArrowsForEmail, UINT16 usVideoObjectIndex,  INT8 bXOffset, INT8 bYOffset, TRUE );

	fJustStartedEmail = FALSE;

	PlaceMessagesinPages();
  btn->uiFlags&= ~(BUTTON_CLICKED_ON);
 }

 else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
 {
  // nothing yet
 }

}

void SubjectCallback(GUI_BUTTON *btn, INT32 iReason )
{
 if (iReason & MSYS_CALLBACK_REASON_INIT)
 {
	return;
 }
 if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
 {
  // sort message on subject and reorder list
	fSortSubjectUpwards = !fSortSubjectUpwards;

  SortMessages(SUBJECT);
	fJustStartedEmail = FALSE;
	PlaceMessagesinPages();



	btn->uiFlags&= ~(BUTTON_CLICKED_ON);
 }
 else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
 {
  // nothing yet
 }

}


void BtnDeleteCallback(GUI_BUTTON *btn, INT32 iReason )
{
 if (iReason & MSYS_CALLBACK_REASON_INIT)
 {
	return;
 }
 if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
 {

	 btn->uiFlags&= ~(BUTTON_CLICKED_ON);
	 iDeleteId = giMessageId;
	 fDeleteMailFlag = TRUE;

 }
 else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
 {
  // nothing yet
 }

}

void DateCallback(GUI_BUTTON *btn, INT32 iReason )
{
 if (iReason & MSYS_CALLBACK_REASON_INIT)
 {
	return;
 }
 if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
 {
  // sort messages based on date recieved and reorder lsit
	fSortDateUpwards = !fSortDateUpwards;
  SortMessages(RECEIVED);
	PlaceMessagesinPages();

	fJustStartedEmail = FALSE;

	btn->uiFlags&= ~(BUTTON_CLICKED_ON);
 }
 else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
 {
  // nothing yet
 }

}


void ReadCallback(GUI_BUTTON *btn, INT32 iReason )
{
 if (iReason & MSYS_CALLBACK_REASON_INIT)
 {
	return;
 }
 if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
 {
  // sort messages based on date recieved and reorder lsit
  SortMessages(READ);
	PlaceMessagesinPages();

	fJustStartedEmail = FALSE;

	btn->uiFlags&= ~(BUTTON_CLICKED_ON);
 }
 else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
 {
  // nothing yet
 }

}


void SetUpSortRegions()
{

	// have been replaced by buttons
	return;

	// will set up sort mail regions

	// from region
  /*
	MSYS_DefineRegion(&pSortMailRegions[0],FROM_BOX_X ,FROM_BOX_Y, FROM_BOX_X+FROM_BOX_WIDTH ,FROM_BOX_Y+TOP_HEIGHT,
			MSYS_PRIORITY_NORMAL+2,MSYS_NO_CURSOR,MSYS_NO_CALLBACK, FromCallback );

	// subject region
	MSYS_DefineRegion(&pSortMailRegions[1],SUBJECT_X ,FROM_BOX_Y, SUBJECT_BOX_X+SUBJECT_WIDTH ,FROM_BOX_Y+TOP_HEIGHT,
			MSYS_PRIORITY_NORMAL+2,MSYS_NO_CURSOR,MSYS_NO_CALLBACK, SubjectCallback );

	// date region
	MSYS_DefineRegion(&pSortMailRegions[2],DATE_X ,FROM_BOX_Y, DATE_BOX_X+DATE_WIDTH ,FROM_BOX_Y+TOP_HEIGHT,
			MSYS_PRIORITY_NORMAL+2,MSYS_NO_CURSOR,MSYS_NO_CALLBACK, DateCallback );

	//add regions
	MSYS_AddRegion(&pSortMailRegions[0]);
  MSYS_AddRegion(&pSortMailRegions[1]);
  MSYS_AddRegion(&pSortMailRegions[2]);

	return;
	*/
}

void DeleteSortRegions()
{

	// have been replaced by buttons
 return;
 /*
 MSYS_RemoveRegion(&pSortMailRegions[0]);
 MSYS_RemoveRegion(&pSortMailRegions[1]);
 MSYS_RemoveRegion(&pSortMailRegions[2]);
 */
}



void DisplayTextOnTitleBar( void )
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

void DestroyMailScreenButtons( void )
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

	return;
}

void CreateMailScreenButtons( void )
{

	// create sort buttons, right now - not finished

	// read sort
	giSortButtonImage[0]=  LoadButtonImage( "LAPTOP\\mailbuttons.sti" ,-1,0,-1,4,-1 );
	giSortButton[0] = QuickCreateButton( giSortButtonImage[0], ENVELOPE_BOX_X, FROM_BOX_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)ReadCallback);
	SetButtonCursor(giSortButton[0], CURSOR_LAPTOP_SCREEN);


	// subject sort
	giSortButtonImage[1]=  LoadButtonImage( "LAPTOP\\mailbuttons.sti" ,-1,1,-1,5,-1 );
	giSortButton[1] = QuickCreateButton( giSortButtonImage[1], FROM_BOX_X, FROM_BOX_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)FromCallback);
	SetButtonCursor(giSortButton[1], CURSOR_LAPTOP_SCREEN);
   SpecifyFullButtonTextAttributes( giSortButton[1], pEmailHeaders[FROM_HEADER], EMAIL_WARNING_FONT,
																		  FONT_BLACK, FONT_BLACK,
																			FONT_BLACK, FONT_BLACK, TEXT_CJUSTIFIED );


	// sender sort
	giSortButtonImage[2]=  LoadButtonImage( "LAPTOP\\mailbuttons.sti" ,-1,2,-1,6,-1 );
	giSortButton[2] = QuickCreateButton( giSortButtonImage[2], SUBJECT_BOX_X, FROM_BOX_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)SubjectCallback );
	SetButtonCursor(giSortButton[2], CURSOR_LAPTOP_SCREEN);
	SpecifyFullButtonTextAttributes( giSortButton[2], pEmailHeaders[SUBJECT_HEADER], EMAIL_WARNING_FONT,
																		  FONT_BLACK, FONT_BLACK,
																			FONT_BLACK, FONT_BLACK, TEXT_CJUSTIFIED );



	// date sort
  giSortButtonImage[3]=  LoadButtonImage( "LAPTOP\\mailbuttons.sti" ,-1,3,-1,7,-1 );
	giSortButton[3] = QuickCreateButton( giSortButtonImage[3], DATE_BOX_X, FROM_BOX_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)DateCallback);
	SetButtonCursor(giSortButton[3], CURSOR_LAPTOP_SCREEN);
	SpecifyFullButtonTextAttributes( giSortButton[3], pEmailHeaders[RECD_HEADER], EMAIL_WARNING_FONT,
																		  FONT_BLACK, FONT_BLACK,
																			FONT_BLACK, FONT_BLACK, TEXT_CJUSTIFIED );


	return;
}


void DisplayEmailMessageSubjectDateFromLines( EmailPtr pMail , INT32 iViewerY)
{
	// this procedure will draw the title/headers to From, Subject, Date fields in the display
	// message box
  UINT16 usX, usY;
  wchar_t sString[100];

	// font stuff
	SetFont(MESSAGE_FONT);
	SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);



	// all headers, but not info are right justified

  // print from
  FindFontRightCoordinates( MESSAGE_HEADER_X-20, ( INT16 ) ( MESSAGE_FROM_Y + (INT16)iViewerY ) ,  MESSAGE_HEADER_WIDTH, ( INT16 ) ( MESSAGE_FROM_Y + GetFontHeight ( MESSAGE_FONT ) ) ,pEmailHeaders[0] ,MESSAGE_FONT, &usX, &usY);
	mprintf( usX, MESSAGE_FROM_Y + (UINT16)iViewerY, pEmailHeaders[0]);

	// the actual from info
	mprintf( MESSAGE_HEADER_X+MESSAGE_HEADER_WIDTH-13, MESSAGE_FROM_Y + iViewerY, pSenderNameList[pMail->ubSender]);


  // print date
	FindFontRightCoordinates( MESSAGE_HEADER_X+168, ( INT16 ) ( MESSAGE_DATE_Y + (UINT16)iViewerY ),  MESSAGE_HEADER_WIDTH, ( INT16 ) ( MESSAGE_DATE_Y + GetFontHeight ( MESSAGE_FONT ) ) ,pEmailHeaders[2] ,MESSAGE_FONT, &usX, &usY);
	mprintf( usX, MESSAGE_DATE_Y+ (UINT16)iViewerY , pEmailHeaders[2]);

	// the actual date info
	swprintf(sString, L"%d", ( ( pMail->iDate ) / ( 24 * 60) ) );
	mprintf( MESSAGE_HEADER_X+235, MESSAGE_DATE_Y + (UINT16)iViewerY, sString);



	// print subject
	FindFontRightCoordinates( MESSAGE_HEADER_X-20, MESSAGE_SUBJECT_Y ,  MESSAGE_HEADER_WIDTH, ( INT16 ) (MESSAGE_SUBJECT_Y + GetFontHeight ( MESSAGE_FONT )),pEmailHeaders[1] ,MESSAGE_FONT, &usX, &usY);
	mprintf( usX, MESSAGE_SUBJECT_Y + (UINT16)iViewerY, pEmailHeaders[1]);

 	// the actual subject info
	//mprintf( , MESSAGE_SUBJECT_Y, pMail->pSubject);
	IanDisplayWrappedString(SUBJECT_LINE_X+2, (INT16) ( SUBJECT_LINE_Y+2 + (UINT16)iViewerY ), SUBJECT_LINE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, MESSAGE_COLOR,pMail->pSubject,0,FALSE,0);


	// reset shadow
	SetFontShadow(DEFAULT_SHADOW);
	return;
}


void DrawEmailMessageDisplayTitleText( INT32 iViewerY )
{
  // this procedure will display the title of the email message display box

	// font stuff
  SetFont( EMAIL_HEADER_FONT );
	SetFontForeground( FONT_WHITE );
	SetFontBackground( FONT_BLACK );

	// dsiplay mail viewer title on message viewer
  mprintf( VIEWER_X + 30, VIEWER_Y + 8 + (UINT16) iViewerY, pEmailTitleText[0] );

  return;
}

void DrawLineDividers( void )
{
  // this function draws divider lines between lines of text
	INT32 iCounter=0;
  HVOBJECT hHandle;

	for(iCounter=1; iCounter < 19; iCounter++)
	{
   GetVideoObject( &hHandle, guiMAILDIVIDER );
	 BltVideoObject(FRAME_BUFFER, hHandle, 0,INDIC_X-10, (MIDDLE_Y+iCounter*MIDDLE_WIDTH - 1), VO_BLT_SRCTRANSPARENCY,NULL);
  }


	return;
}


void ClearOutEmailMessageRecordsList( void )
{
  RecordPtr pTempRecord;
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

	return;
}

void AddEmailRecordToList( STR16 pString )
{
  RecordPtr pTempRecord;

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

	// done return

	return;

}


void UpDateMessageRecordList( void )
{

	// simply checks to see if old and new message ids are the same, if so, do nothing
	// otherwise clear list

  if( giMessageId != giPrevMessageId )
	{
		// if chenged, clear list
    ClearOutEmailMessageRecordsList( );

		// set prev to current
		giPrevMessageId = giMessageId;
	}

}

void HandleAnySpecialEmailMessageEvents(INT32 iMessageId )
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

void ReDisplayBoxes( void )
{




	// the email message itself
	if(fDisplayMessageFlag)
	{
		// this simply redraws message with button manipulation
		DisplayEmailMessage(GetEmailMessage(giMessageId));
	}

	if(fDeleteMailFlag)
	{
		// delete message, redisplay
    DisplayDeleteNotice(GetEmailMessage(iDeleteId));
	}

	if(fNewMailFlag)
	{
		// if new mail, redisplay box
    DisplayNewMailBox( );
	}
}


BOOLEAN HandleMailSpecialMessages( UINT16 usMessageId, INT32 *iResults, EmailPtr pMail )
{
  BOOLEAN fSpecialCase = FALSE;

	// this procedure will handle special cases of email messages that are not stored in email.edt, or need special processing
	switch( usMessageId )
	{
		case( IMP_EMAIL_PROFILE_RESULTS ):

			HandleIMPCharProfileResultsMessage( );
		  fSpecialCase = TRUE;

		break;
		case( MERC_INTRO ):
			SetBookMark( MERC_BOOKMARK );
			fReDrawScreenFlag = TRUE;
		break;


		case INSUR_PAYMENT:
		case INSUR_SUSPIC:
		case INSUR_SUSPIC_2:
		case INSUR_INVEST_OVER:
			ModifyInsuranceEmails( usMessageId, iResults, pMail, INSUR_PAYMENT_LENGTH );
			break;

		case INSUR_1HOUR_FRAUD:
			ModifyInsuranceEmails( usMessageId, iResults, pMail, INSUR_1HOUR_FRAUD_LENGTH );
			break;

		case MERC_NEW_SITE_ADDRESS:
			//Set the book mark so the player can access the site
			SetBookMark( MERC_BOOKMARK );
			break;

		case MERC_DIED_ON_OTHER_ASSIGNMENT:
			ModifyInsuranceEmails( usMessageId, iResults, pMail, MERC_DIED_ON_OTHER_ASSIGNMENT_LENGTH );
			break;

		case AIM_MEDICAL_DEPOSIT_REFUND:
		case AIM_MEDICAL_DEPOSIT_NO_REFUND:
		case AIM_MEDICAL_DEPOSIT_PARTIAL_REFUND:
			ModifyInsuranceEmails( usMessageId, iResults, pMail, AIM_MEDICAL_DEPOSIT_REFUND_LENGTH );
			break;
	}

	return fSpecialCase;
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




void HandleIMPCharProfileResultsMessage( void)
{
  // special case, IMP profile return
	INT32 iTotalHeight=0;
	INT32 iCnt=0;
	INT32 iHeight=0;
	INT32 iCounter=0;
//	wchar_t pString[MAIL_STRING_SIZE/2 + 1];
	wchar_t pString[MAIL_STRING_SIZE];
	INT32 iOffSet=0;
	INT32 iViewerY=0;
	INT32 iHeightTemp=0;
  INT32 iHeightSoFar = 0;
	RecordPtr pTempRecord;
  INT32 iEndOfSection =0;
	INT32 iRand = 0;
	BOOLEAN fSufficientMechSkill = FALSE, fSufficientMarkSkill = FALSE, fSufficientMedSkill = FALSE, fSufficientExplSkill = FALSE;
	BOOLEAN fSufficientHlth = FALSE, fSufficientStr = FALSE, fSufficientWis = FALSE, fSufficientAgi = FALSE, fSufficientDex = FALSE, fSufficientLdr = FALSE;

	iRand = Random( 32767 );

	// set record ptr to head of list
	pTempRecord=pMessageRecordList;

	// increment height for size of one line
	iHeight+=GetFontHeight( MESSAGE_FONT );

	// load intro
  iEndOfSection = IMP_RESULTS_INTRO_LENGTH;

	// list doesn't exist, reload
	if( !pTempRecord )
  {

	  while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

			// have to place players name into string for first record
			if( iCounter == 0)
			{
				wchar_t	zTemp[512];

				swprintf( zTemp, L" %s", gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].zName );
				wcscat( pString, zTemp );
			}

		  // add to list
		  AddEmailRecordToList( pString );

       // increment email record counter
		   iCounter++;
		}

		// now the personality intro
		iOffSet = IMP_RESULTS_PERSONALITY_INTRO;
		iEndOfSection = IMP_RESULTS_PERSONALITY_INTRO_LENGTH + 1;
    iCounter = 0;

		while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

       // increment email record counter
		   iCounter++;
		}

		// personality itself
		switch( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bPersonalityTrait)
		{
			// normal as can be
		  case( NO_PERSONALITYTRAIT ):
        iOffSet = IMP_PERSONALITY_NORMAL;
				break;
			case( HEAT_INTOLERANT ):
				iOffSet = IMP_PERSONALITY_HEAT;
				break;
      case( NERVOUS ):
				iOffSet = IMP_PERSONALITY_NERVOUS;
				break;
			case( CLAUSTROPHOBIC ):
				iOffSet = IMP_PERSONALITY_CLAUSTROPHOBIC;
				break;
			case( NONSWIMMER ):
				iOffSet = IMP_PERSONALITY_NONSWIMMER;
				break;
			case( FEAR_OF_INSECTS ):
				iOffSet = IMP_PERSONALITY_FEAR_OF_INSECTS;
				break;
			case( FORGETFUL ):
				iOffSet = IMP_PERSONALITY_FORGETFUL;
				break;
			case( PSYCHO ):
				iOffSet = IMP_PERSONALITY_PSYCHO;
				break;
		}

		// personality tick
//  DEF: removed 1/12/99, cause it was changing the length of email that were already calculated
//		LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + Random( IMP_PERSONALITY_LENGTH - 1 ) + 1 ), MAIL_STRING_SIZE );
		LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + 1 ), MAIL_STRING_SIZE );
		// add to list
		AddEmailRecordToList( pString );

		// persoanlity paragraph
		LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + IMP_PERSONALITY_LENGTH ), MAIL_STRING_SIZE );
    // add to list
		AddEmailRecordToList( pString );

		// extra paragraph for bugs
		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bPersonalityTrait == FEAR_OF_INSECTS )
		{
      // persoanlity paragraph
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + IMP_PERSONALITY_LENGTH + 1 ), MAIL_STRING_SIZE );
			// add to list
			AddEmailRecordToList( pString );
		}

    // attitude intro
    // now the personality intro
		iOffSet = IMP_RESULTS_ATTITUDE_INTRO;
		iEndOfSection = IMP_RESULTS_ATTITUDE_LENGTH;
    iCounter = 0;

		while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

      // increment email record counter
		  iCounter++;
		}

			// personality itself
		switch( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bAttitude )
		{
			// normal as can be
		  case( ATT_NORMAL ):
        iOffSet = IMP_ATTITUDE_NORMAL;
				break;
			case( ATT_FRIENDLY ):
				iOffSet = IMP_ATTITUDE_FRIENDLY;
				break;
      case( ATT_LONER ):
				iOffSet = IMP_ATTITUDE_LONER;
				break;
			case( ATT_OPTIMIST ):
				iOffSet = IMP_ATTITUDE_OPTIMIST;
				break;
			case( ATT_PESSIMIST ):
				iOffSet = IMP_ATTITUDE_PESSIMIST;
				break;
			case( ATT_AGGRESSIVE ):
				iOffSet = IMP_ATTITUDE_AGGRESSIVE;
				break;
			case( ATT_ARROGANT ):
				iOffSet = IMP_ATTITUDE_ARROGANT;
				break;
      case( ATT_ASSHOLE ):
				iOffSet = IMP_ATTITUDE_ASSHOLE;
				break;
			case( ATT_COWARD ):
				iOffSet = IMP_ATTITUDE_COWARD;
				break;

		}

		// attitude title
		LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet ), MAIL_STRING_SIZE );
		// add to list
		AddEmailRecordToList( pString );


		// attitude tick
//  DEF: removed 1/12/99, cause it was changing the length of email that were already calculated
//		LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + Random( IMP_ATTITUDE_LENGTH - 2 ) + 1 ), MAIL_STRING_SIZE );
		LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + 1 ), MAIL_STRING_SIZE );
		// add to list
		AddEmailRecordToList( pString );

		// attitude paragraph
		LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + IMP_ATTITUDE_LENGTH - 1 ), MAIL_STRING_SIZE );
    // add to list
		AddEmailRecordToList( pString );

		//check for second paragraph
		if( iOffSet != IMP_ATTITUDE_NORMAL )
		{
      // attitude paragraph
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + IMP_ATTITUDE_LENGTH ), MAIL_STRING_SIZE );
      // add to list
		  AddEmailRecordToList( pString );
		}


		// skills
    // now the skills intro
		iOffSet = IMP_RESULTS_SKILLS;
		iEndOfSection = IMP_RESULTS_SKILLS_LENGTH;
    iCounter = 0;

	  while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

      // increment email record counter
		  iCounter++;
		}

		// imperial skills
    iOffSet = IMP_SKILLS_IMPERIAL_SKILLS;
		iEndOfSection = 0;
    iCounter = 0;

		// marksmanship
		if ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bMarksmanship >= SUPER_SKILL_VALUE )
    {
			fSufficientMarkSkill = TRUE;
			iEndOfSection = 1;
		}

		// medical
		if ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bMedical >= SUPER_SKILL_VALUE )
		{
			fSufficientMedSkill = TRUE;
			iEndOfSection = 1;
		}

		// mechanical
		if ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bMechanical >= SUPER_SKILL_VALUE )
		{
			fSufficientMechSkill = TRUE;
			iEndOfSection = 1;
		}

		if ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bExplosive >= SUPER_SKILL_VALUE )
		{
			fSufficientExplSkill = TRUE;
			iEndOfSection = 1;
		}

		while (iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

      // increment email record counter
		  iCounter++;
		}

		// now handle skills
    if ( fSufficientMarkSkill )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_IMPERIAL_MARK  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}


		if ( fSufficientMedSkill )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_IMPERIAL_MED  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if ( fSufficientMechSkill )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_IMPERIAL_MECH  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		// explosives
		if ( fSufficientExplSkill )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_IMPERIAL_EXPL  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		fSufficientMechSkill = FALSE;
		fSufficientMarkSkill = FALSE;
		fSufficientExplSkill = FALSE;
		fSufficientMedSkill = FALSE;

		// imperial skills
    iOffSet = IMP_SKILLS_NEED_TRAIN_SKILLS;
		iEndOfSection = 0;
    iCounter = 0;



		// now the needs training values
		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bMarksmanship > NO_CHANCE_IN_HELL_SKILL_VALUE ) &&( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bMarksmanship <= NEEDS_TRAINING_SKILL_VALUE ) )
    {
			fSufficientMarkSkill = TRUE;
			iEndOfSection = 1;
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bMedical > NO_CHANCE_IN_HELL_SKILL_VALUE ) &&( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bMedical <= NEEDS_TRAINING_SKILL_VALUE ) )
    {
			fSufficientMedSkill = TRUE;
			iEndOfSection = 1;
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bMechanical > NO_CHANCE_IN_HELL_SKILL_VALUE ) &&( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bMechanical <= NEEDS_TRAINING_SKILL_VALUE ) )
    {
			fSufficientMechSkill = TRUE;
			iEndOfSection = 1;
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bExplosive > NO_CHANCE_IN_HELL_SKILL_VALUE ) &&( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bExplosive <= NEEDS_TRAINING_SKILL_VALUE ) )
    {
			fSufficientExplSkill = TRUE;
			iEndOfSection = 1;
		}

		while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

       // increment email record counter
		   iCounter++;
		}

		if( fSufficientMarkSkill )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_NEED_TRAIN_MARK  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientMedSkill )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_NEED_TRAIN_MED  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientMechSkill )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_NEED_TRAIN_MECH ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientExplSkill )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_NEED_TRAIN_EXPL ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		fSufficientMechSkill = FALSE;
		fSufficientMarkSkill = FALSE;
		fSufficientExplSkill = FALSE;
		fSufficientMedSkill = FALSE;

		// and the no chance in hell of doing anything useful values

		// no skill
    iOffSet = IMP_SKILLS_NO_SKILL;
		iEndOfSection = 0;
    iCounter = 0;

		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bMarksmanship <= NO_CHANCE_IN_HELL_SKILL_VALUE )
		{
			fSufficientMarkSkill = TRUE;
			iEndOfSection = 1;
		}

		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bMedical <= NO_CHANCE_IN_HELL_SKILL_VALUE )
		{
			fSufficientMedSkill = TRUE;
			iEndOfSection = 1;
		}

		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bMechanical <= NO_CHANCE_IN_HELL_SKILL_VALUE )
		{
			fSufficientMechSkill = TRUE;
			iEndOfSection = 1;
		}

		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bExplosive <= NO_CHANCE_IN_HELL_SKILL_VALUE )
		{
			fSufficientExplSkill = TRUE;
			iEndOfSection = 1;
		}

		while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

       // increment email record counter
		   iCounter++;
		}

		if( fSufficientMechSkill )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_NO_SKILL_MECH ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientMarkSkill )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_NO_SKILL_MARK ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientMedSkill )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_NO_SKILL_MED ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}
		if( fSufficientExplSkill )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_NO_SKILL_EXPL ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		// now the specialized skills
		// imperial skills
    iOffSet = IMP_SKILLS_SPECIAL_INTRO;
		iEndOfSection = IMP_SKILLS_SPECIAL_INTRO_LENGTH;
    iCounter = 0;

		while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

       // increment email record counter
		   iCounter++;
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == KNIFING )||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == KNIFING ) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_KNIFE ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}

    // lockpick
		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == LOCKPICKING)||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == LOCKPICKING) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_LOCK ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}

		// hand to hand
		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == HANDTOHAND )||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == HANDTOHAND ) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_HAND ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}

		// electronics
		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == ELECTRONICS )||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == ELECTRONICS ) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_ELEC ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == NIGHTOPS )||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == NIGHTOPS ) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_NIGHT ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == THROWING)||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == THROWING) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_THROW ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == TEACHING )||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == TEACHING ) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_TEACH ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == HEAVY_WEAPS )||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == HEAVY_WEAPS ) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_HEAVY ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == AUTO_WEAPS )||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == AUTO_WEAPS ) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_AUTO ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == STEALTHY )||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == STEALTHY ) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_STEALTH ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == AMBIDEXT)||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == AMBIDEXT) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_AMBI ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == THIEF )||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == THIEF ) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_THIEF ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}

		if( ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait == MARTIALARTS )||( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bSkillTrait2 == MARTIALARTS ) )
    {
      	// read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_SKILLS_SPECIAL_MARTIAL ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );
		}


		// now the physical
		// imperial physical
    iOffSet = IMP_RESULTS_PHYSICAL;
		iEndOfSection = IMP_RESULTS_PHYSICAL_LENGTH;
    iCounter = 0;

		while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

       // increment email record counter
		   iCounter++;
		}

		// super physical
		iOffSet = IMP_PHYSICAL_SUPER;
		iEndOfSection = 0;
    iCounter = 0;


		// health
		if(  gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bLife >= SUPER_STAT_VALUE )
    {
			fSufficientHlth = TRUE;
			iEndOfSection = 1;
		}

		// dex
		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bDexterity >= SUPER_STAT_VALUE )
		{
			fSufficientDex = TRUE;
			iEndOfSection = 1;
		}

		// agility
		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bAgility >= SUPER_STAT_VALUE )
		{
			fSufficientAgi  = TRUE;
			iEndOfSection = 1;
		}

		// strength
		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bStrength >= SUPER_STAT_VALUE )
		{
			fSufficientStr = TRUE;
			iEndOfSection = 1;
		}

		// wisdom
		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bWisdom >= SUPER_STAT_VALUE )
		{
			fSufficientWis = TRUE;
			iEndOfSection =1;
		}

		// leadership
		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bLeadership >= SUPER_STAT_VALUE )
		{
			fSufficientLdr = TRUE;
			iEndOfSection = 1;
		}

		while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

       // increment email record counter
		   iCounter++;
		}

		if( fSufficientHlth )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_SUPER_HEALTH  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}


		if( fSufficientDex )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_SUPER_DEXTERITY  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientStr )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_SUPER_STRENGTH  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientAgi )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_SUPER_AGILITY  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientWis )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_SUPER_WISDOM  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientLdr )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_SUPER_LEADERSHIP  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		fSufficientHlth = FALSE;
		fSufficientStr = FALSE;
		fSufficientWis = FALSE;
		fSufficientAgi = FALSE;
		fSufficientDex = FALSE;
		fSufficientLdr = FALSE;

    // now the low attributes
    // super physical
		iOffSet = IMP_PHYSICAL_LOW;
		iEndOfSection = 0;
    iCounter = 0;

		// health
		if(  ( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bLife < NEEDS_TRAINING_STAT_VALUE ) &&( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bLife > NO_CHANCE_IN_HELL_STAT_VALUE ) )
    {
			fSufficientHlth = TRUE;
			iEndOfSection = 1;
		}

		// strength
		if( (gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bStrength < NEEDS_TRAINING_STAT_VALUE )&&( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bStrength > NO_CHANCE_IN_HELL_STAT_VALUE ) )
		{
			fSufficientStr = TRUE;
			iEndOfSection = 1;
		}

		// agility
		if( (gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bAgility < NEEDS_TRAINING_STAT_VALUE )&&( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bAgility <= NO_CHANCE_IN_HELL_STAT_VALUE ) )
		{
			fSufficientAgi = TRUE;
			iEndOfSection = 1;
		}

		// wisdom
		if( (gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bWisdom < NEEDS_TRAINING_STAT_VALUE)&&( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bWisdom > NO_CHANCE_IN_HELL_STAT_VALUE ) )
		{
			fSufficientWis = TRUE;
			iEndOfSection = 1;
		}

		// leadership
		if( (gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bLeadership < NEEDS_TRAINING_STAT_VALUE)&&( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bLeadership > NO_CHANCE_IN_HELL_STAT_VALUE ) )
		{
			fSufficientLdr = TRUE;
			iEndOfSection = 1;
		}

		// dex
		if( (gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bDexterity < NEEDS_TRAINING_STAT_VALUE )&&( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bDexterity > NO_CHANCE_IN_HELL_STAT_VALUE ) )
		{
			fSufficientDex = TRUE;
			iEndOfSection = 1;
		}

		while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

       // increment email record counter
		   iCounter++;
		}

		if( fSufficientHlth )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_LOW_HEALTH  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}


		if( fSufficientDex )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_LOW_DEXTERITY  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientStr )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_LOW_STRENGTH  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}



		if( fSufficientAgi )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_LOW_AGILITY  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientWis )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_LOW_WISDOM  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientLdr )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_LOW_LEADERSHIP  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}









		// very low physical
		iOffSet = IMP_PHYSICAL_VERY_LOW;
		iEndOfSection = 0;
    iCounter = 0;

		fSufficientHlth = FALSE;
		fSufficientStr = FALSE;
		fSufficientWis = FALSE;
		fSufficientAgi = FALSE;
		fSufficientDex = FALSE;
		fSufficientLdr = FALSE;

		// health
		if(  gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bLife <= NO_CHANCE_IN_HELL_STAT_VALUE )
    {
			fSufficientHlth = TRUE;
			iEndOfSection =1;
		}

		// dex
		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bDexterity <= NO_CHANCE_IN_HELL_STAT_VALUE )
		{
			fSufficientDex = TRUE;
			iEndOfSection =1;
		}

		// strength
		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bStrength <= NO_CHANCE_IN_HELL_STAT_VALUE )
		{
			fSufficientStr = TRUE;
			iEndOfSection = 1;
		}

		// agility
		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bAgility <= NO_CHANCE_IN_HELL_STAT_VALUE )
		{
			fSufficientAgi = TRUE;
			iEndOfSection = 1;
		}

		// wisdom
		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bWisdom <= NO_CHANCE_IN_HELL_STAT_VALUE )
		{
			fSufficientWis = TRUE;
			iEndOfSection =1;
		}

		while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

       // increment email record counter
		   iCounter++;
		}

		if( fSufficientHlth )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_VERY_LOW_HEALTH  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}



		if( fSufficientDex )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_VERY_LOW_DEXTERITY  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

		if( fSufficientStr )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_VERY_LOW_STRENGTH  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}



		if( fSufficientAgi )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_VERY_LOW_AGILITY  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}



		if( fSufficientWis )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_VERY_LOW_WISDOM  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}


		// leadership
		if( gMercProfiles[ PLAYER_GENERATED_CHARACTER_ID + LaptopSaveInfo.iVoiceId ].bLeadership <= NO_CHANCE_IN_HELL_STAT_VALUE )
		{
			fSufficientLdr = TRUE;
		}

    if( fSufficientLdr )
		{
			// read one record from email file
			LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( IMP_PHYSICAL_VERY_LOW_LEADERSHIP  ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );
		}

    	// very low physical
		iOffSet = IMP_RESULTS_PORTRAIT;
		iEndOfSection = IMP_RESULTS_PORTRAIT_LENGTH;
    iCounter = 0;

		while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

       // increment email record counter
		   iCounter++;
		}


		// portraits

		switch( iPortraitNumber )
		{
		  case( 0 ):
		    iOffSet = IMP_PORTRAIT_MALE_1;
				break;
			case( 1 ):
		    iOffSet = IMP_PORTRAIT_MALE_2;
				break;
			case( 2 ):
		    iOffSet = IMP_PORTRAIT_MALE_3;
				break;
			case( 3 ):
		    iOffSet = IMP_PORTRAIT_MALE_4;
				break;
			case( 4 ):
			case( 5 ):
		    iOffSet = IMP_PORTRAIT_MALE_5;
				break;
			case( 6 ):
			case( 7 ):
		    iOffSet = IMP_PORTRAIT_MALE_6;
				break;
			case( 8 ):
		    iOffSet = IMP_PORTRAIT_FEMALE_1;
				break;
			case( 9 ):
		    iOffSet = IMP_PORTRAIT_FEMALE_2;
				break;
      case( 10 ):
		    iOffSet = IMP_PORTRAIT_FEMALE_3;
				break;
			case( 11 ):
			case( 12 ):
		    iOffSet = IMP_PORTRAIT_FEMALE_4;
				break;
			case( 13 ):
			case( 14 ):
		    iOffSet = IMP_PORTRAIT_FEMALE_5;
				break;
		}

		if( ( iRand % 2 ) == 0 )
		{
			iOffSet += 2;
		}

    iEndOfSection = 2;
    iCounter = 0;

    while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

       // increment email record counter
		   iCounter++;
		}

    iOffSet = IMP_RESULTS_END;
		iEndOfSection = IMP_RESULTS_END_LENGTH;
    iCounter = 0;

		while(iEndOfSection > iCounter)
		{
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Impass.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

		  // add to list
		  AddEmailRecordToList( pString );

       // increment email record counter
		   iCounter++;
		}

		giPrevMessageId = giMessageId;

	}

  pTempRecord = pMessageRecordList;


}

void HandleEmailViewerButtonStates( void )
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

	return;

}


void SetUpIconForButton( )
{
	// if we just got in, return, don't set any

	if( fJustStartedEmail == TRUE )
	{
		return;
	}




	return;
}


void DeleteCurrentMessage( void )
{
	// will delete the currently displayed message

	// set current message to be deleted
	iDeleteId = giMessageId;

	// set the currently displayed message to none
	giMessageId = -1;

	// reset display message flag
  fDisplayMessageFlag=FALSE;

	// reset page being displayed
  giMessagePage = 0;

	fDeleteInternal = TRUE;

	// delete message
	DeleteEmail( );

	fDeleteInternal = FALSE;

	// force update of entire screen
	fReDrawScreenFlag=TRUE;

  // rerender email
	RenderEmail();

	return;
}


void CreateNextPreviousEmailPageButtons( void )
{

	// this function will create the buttons to advance and go back email pages

	// next button
	giMailPageButtonsImage[0]=  LoadButtonImage( "LAPTOP\\NewMailButtons.sti" ,-1,1,-1,4,-1 );
	giMailPageButtons[0] = QuickCreateButton( giMailPageButtonsImage[0],NEXT_PAGE_X, NEXT_PAGE_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)NextRegionButtonCallback );
	SetButtonCursor(giMailPageButtons[0], CURSOR_LAPTOP_SCREEN);

	// previous button
	giMailPageButtonsImage[1]=  LoadButtonImage( "LAPTOP\\NewMailButtons.sti" ,-1,0,-1,3,-1 );
	giMailPageButtons[1] = QuickCreateButton( giMailPageButtonsImage[1],PREVIOUS_PAGE_X, NEXT_PAGE_Y,
									BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
									(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)PreviousRegionButtonCallback );
	SetButtonCursor(giMailPageButtons[1], CURSOR_LAPTOP_SCREEN);

	/*
	// set up disable methods
  SpecifyDisabledButtonStyle( giMailPageButtons[1], DISABLED_STYLE_SHADED );
  SpecifyDisabledButtonStyle( giMailPageButtons[0], DISABLED_STYLE_SHADED );
*/

	return;
}


void UpdateStatusOfNextPreviousButtons( void )
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


void DisplayWhichPageOfEmailProgramIsDisplayed( void )
{
	// will draw the number of the email program we are viewing right now
	CHAR16 sString[ 10 ];

	// font stuff
	SetFont(MESSAGE_FONT);
	SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);

	// page number
	if( iLastPage < 0 )
		swprintf( sString, L"%d / %d", 1, 1);
	else
		swprintf( sString, L"%d / %d", iCurrentPage + 1, iLastPage + 1);

	// print it
	mprintf( PAGE_NUMBER_X ,PAGE_NUMBER_Y, sString );

	// restore shadow
	SetFontShadow( DEFAULT_SHADOW );

	return;
}

void OpenMostRecentUnreadEmail( void )
{
	// will open the most recent email the player has recieved and not read
	INT32 iMostRecentMailId = -1;
	EmailPtr pB=pEmailList;
	UINT32 iLowestDate = 9999999;

	while( pB )
	{
		// if date is lesser and unread , swap
		if( ( pB->iDate < iLowestDate )&&( pB->fRead == FALSE ) )
		{
			iMostRecentMailId = pB -> iId;
			iLowestDate = pB -> iDate;
		}

		// next in B's list
	  pB=pB->Next;
	}

	// set up id
	giMessageId = iMostRecentMailId;

	// valid message, show it
	if( giMessageId != -1 )
	{
		fDisplayMessageFlag = TRUE;
	}

	return;
}


BOOLEAN DisplayNumberOfPagesToThisEmail( INT32 iViewerY )
{
	// display the indent for the display of pages to this email..along with the current page/number of pages


  INT32 iCounter=0;
	INT16 sX = 0, sY = 0;
	CHAR16 sString[ 32 ];


	// get and blt the email list background
   // load, blt and delete graphics
	//VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
//	FilenameForBPP( "LAPTOP\\mailindent.sti", VObjectDesc.ImageFile );
	//CHECKF( AddVideoObject( &VObjectDesc, &uiMailIndent ) );
 // GetVideoObject( &hHandle, uiMailIndent );
 // BltVideoObject( FRAME_BUFFER, hHandle, 0,VIEWER_X + INDENT_X_OFFSET, VIEWER_Y + iViewerY + INDENT_Y_OFFSET - 10, VO_BLT_SRCTRANSPARENCY,NULL );
 // DeleteVideoObjectFromIndex( uiMailIndent );

	giNumberOfPagesToCurrentEmail = ( giNumberOfPagesToCurrentEmail );

	// parse current page and max number of pages to email
	swprintf( sString,L"%d / %d", ( giMessagePage + 1 ), ( giNumberOfPagesToCurrentEmail - 1 ) );

	SetFont( FONT12ARIAL );
	SetFontForeground( FONT_BLACK );
	SetFontBackground( FONT_BLACK );

	// turn off the shadows
	SetFontShadow(NO_SHADOW);

	SetFontDestBuffer(FRAME_BUFFER, 0 , 0 , 640 , 480,  FALSE  );

	FindFontCenterCoordinates(VIEWER_X + INDENT_X_OFFSET, 0,INDENT_X_WIDTH, 0, sString, FONT12ARIAL, &sX, &sY);
	mprintf( sX, VIEWER_Y + iViewerY + INDENT_Y_OFFSET - 2, sString );


	// restore shadows
	SetFontShadow( DEFAULT_SHADOW );

	return ( TRUE );
}


INT32 GetNumberOfPagesToEmail( )
{
	RecordPtr pTempRecord;
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
	EmailPtr pEmail = pEmailList;
	EmailPtr pTempEmail = NULL;

	//loop through all the emails to delete them
	while( pEmail )
	{
		pTempEmail = pEmail;

		pEmail = pEmail->Next;

		MemFree( pTempEmail->pSubject );
		pTempEmail->pSubject = NULL;

		MemFree( pTempEmail );
		pTempEmail = NULL;
	}
	pEmailList = NULL;

	ClearPages();
}

void PreProcessEmail( EmailPtr pMail )
{
	RecordPtr pTempRecord, pCurrentRecord, pLastRecord , pTempList;
	CHAR16 pString[ 512 ];
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
      // read one record from email file
		  LoadEncryptedDataFromFile( "BINARYDATA\\Email.edt", pString, MAIL_STRING_SIZE * ( iOffSet + iCounter ), MAIL_STRING_SIZE );

			// add to list
			AddEmailRecordToList( pString );

      // increment email record counter
		  iCounter++;
    }
    giPrevMessageId = giMessageId;

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

		// copy over string
		wcscpy(pString, pTempRecord -> pRecord);

	  // get the height of the string, ONLY!...must redisplay ON TOP OF background graphic
		iHeight += IanWrappedStringHeight(VIEWER_X + MESSAGE_X + 4, ( UINT16 )( VIEWER_MESSAGE_BODY_START_Y + iHeight + GetFontHeight(MESSAGE_FONT)), MESSAGE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, MESSAGE_COLOR,pString,0,FALSE,0);

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

/*
//Def removed
		if( pTempRecord )
		{
			pTempRecord = pTempRecord->Next;
		}
*/

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

/*
//def removed
		// skip the subject
		if( pTempList )
		{
			pTempList = pTempList->Next;
		}

*/
		iCounter = 0;

		// more than one page
		//for( iCounter = 0; iCounter < giNumberOfPagesToCurrentEmail; iCounter++ )
		while( pTempRecord = GetFirstRecordOnThisPage( pTempList, MESSAGE_FONT, MESSAGE_WIDTH, MESSAGE_GAP, iCounter, MAX_EMAIL_MESSAGE_PAGE_SIZE ) )
		{
			iYPositionOnPage = 0;

			pEmailPageInfo[ iCounter ].pFirstRecord = pTempRecord;
			pEmailPageInfo[ iCounter ].iPageNumber = iCounter;
			pLastRecord = NULL;

			// go to the right record
			while( pTempRecord )
			{
				// copy over string
				wcscpy( pString, pTempRecord -> pRecord );

				if( pString[ 0 ] == 0 )
				{
					// on last page
					fOnLastPageFlag = TRUE;
				}


				if( ( iYPositionOnPage + IanWrappedStringHeight(0, 0, MESSAGE_WIDTH, MESSAGE_GAP,
																	MESSAGE_FONT, 0, pTempRecord->pRecord,
																 0, 0, 0 ) )  <= MAX_EMAIL_MESSAGE_PAGE_SIZE  )
				{
     			// now print it
					iYPositionOnPage += IanWrappedStringHeight(VIEWER_X + MESSAGE_X + 4, ( UINT16 )( VIEWER_MESSAGE_BODY_START_Y + 10 +iYPositionOnPage + iViewerPositionY), MESSAGE_WIDTH, MESSAGE_GAP, MESSAGE_FONT, MESSAGE_COLOR,pString,0,FALSE, IAN_WRAP_NO_SHADOW);
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


void ModifyInsuranceEmails( UINT16 usMessageId, INT32 *iResults, EmailPtr pMail, UINT8 ubNumberOfRecords )
{
	INT32 iHeight=0;
	RecordPtr pTempRecord;
//	wchar_t pString[MAIL_STRING_SIZE/2 + 1];
	wchar_t pString[MAIL_STRING_SIZE];
	UINT8	ubCnt;


	// Replace the name in the subject line
//	swprintf( pMail->pSubject, gMercProfiles[ pMail->ubFirstData ].zNickname );

	// set record ptr to head of list
	pTempRecord=pMessageRecordList;

	// increment height for size of one line
	iHeight+=GetFontHeight( MESSAGE_FONT );

	for( ubCnt=0; ubCnt<ubNumberOfRecords; ubCnt++)
	{
		// read one record from email file
		LoadEncryptedDataFromFile( "BINARYDATA\\Email.edt", pString, MAIL_STRING_SIZE * usMessageId, MAIL_STRING_SIZE );

		//Replace the $MERCNAME$ and $AMOUNT$ with the mercs name and the amountm if the string contains the keywords.
		ReplaceMercNameAndAmountWithProperData( pString, pMail );

		// add to list
		AddEmailRecordToList( pString );

		usMessageId++;
	}


//
	giPrevMessageId = giMessageId;
}

BOOLEAN ReplaceMercNameAndAmountWithProperData( CHAR16 *pFinishedString, EmailPtr pMail )
{
//	wchar_t		pTempString[MAIL_STRING_SIZE/2 + 1];
	wchar_t		pTempString[MAIL_STRING_SIZE];
	INT32			iLength=0;
	INT32			iCurLocInSourceString=0;
	INT32			iLengthOfSourceString = wcslen( pFinishedString );		//Get the length of the source string
	CHAR16		*pMercNameString=NULL;
	CHAR16		*pAmountString=NULL;
	CHAR16		*pSubString=NULL;
	BOOLEAN		fReplacingMercName = TRUE;

	CHAR16	sMercName[ 32 ] = L"$MERCNAME$";	//Doesnt need to be translated, inside Email.txt and will be replaced by the mercs name
	CHAR16	sAmount[32] = L"$AMOUN$";		//Doesnt need to be translated, inside Email.txt and will be replaced by a dollar amount
	CHAR16	sSearchString[32];

	//Copy the original string over to the temp string
	wcscpy( pTempString, pFinishedString );

	//Null out the string
	pFinishedString[0] = L'\0';


	//Keep looping through to replace all references to the keyword
	while( iCurLocInSourceString < iLengthOfSourceString )
	{
		iLength = 0;
		pSubString = NULL;

		//Find out if the $MERCNAME$ is in the string
		pMercNameString = wcsstr( &pTempString[ iCurLocInSourceString ], sMercName );

		pAmountString = wcsstr( &pTempString[ iCurLocInSourceString ], sAmount );

		if( pMercNameString != NULL && pAmountString != NULL )
		{
			if( pMercNameString < pAmountString )
			{
				fReplacingMercName = TRUE;
				pSubString = pMercNameString;
				wcscpy( sSearchString, sMercName);
			}
			else
			{
				fReplacingMercName = FALSE;
				pSubString = pAmountString;
				wcscpy( sSearchString, sAmount);
			}
		}
		else if( pMercNameString != NULL )
		{
			fReplacingMercName = TRUE;
			pSubString = pMercNameString;
			wcscpy( sSearchString, sMercName);
		}
		else if( pAmountString != NULL )
		{
			fReplacingMercName = FALSE;
			pSubString = pAmountString;
			wcscpy( sSearchString, sAmount);
		}
		else
		{
			pSubString = NULL;
		}


		// if there is a substring
		if( pSubString != NULL )
		{
			iLength = pSubString - &pTempString[ iCurLocInSourceString ];

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

				swprintf( sDollarAmount, L"%d", pMail->iFirstData );

				InsertCommasForDollarFigure( sDollarAmount );
				InsertDollarSignInToString( sDollarAmount );

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

//	AddEmail( ( UINT8 )( AIM_REPLY_BARRY + ( AIM_REPLY_LENGTH_BARRY ) ), AIM_REPLY_LENGTH_BARRY, AIM_REPLY_BARRY, GetWorldTotalMin() );

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
