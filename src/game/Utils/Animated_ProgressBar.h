#ifndef __ANIMATED_PROGRESSBAR_H
#define __ANIMATED_PROGRESSBAR_H

#include "Types.h"

void CreateLoadingScreenProgressBar(void);
void RemoveLoadingScreenProgressBar(void);


/* This creates a single progress bar given the coordinates without a panel
 * (containing a title and background).  A panel is automatically created if you
 * specify a title using SetProgressBarTitle() */
void CreateProgressBar(UINT8 ubProgressBarID, UINT16 x, UINT16 y, UINT16 w, UINT16 h);

//You may also define a panel to go in behind the progress bar.  You can now assign a title to go with
//the panel.
void DefineProgressBarPanel( UINT32 ubID, UINT8 r, UINT8 g, UINT8 b,
				UINT16 usLeft, UINT16 usTop, UINT16 usRight, UINT16 usBottom );

//Assigning a title for the panel will automatically position the text horizontally centered on the
//panel and vertically centered from the top of the panel, to the top of the progress bar.
void SetProgressBarTitle(UINT32 ubID, const ST::string& str, SGPFont font, UINT8 ubForeColor, UINT8 ubShadowColor);

//Unless you set up the attributes, any text you pass to SetRelativeStartAndEndPercentage will
//default to FONT12POINT1 in a black color.
void SetProgressBarMsgAttributes(UINT32 ubID, SGPFont, UINT8 ubForeColor, UINT8 ubShadowColor);


//When finished, the progress bar needs to be removed.
void RemoveProgressBar( UINT8 ubID );

//An important setup function.  The best explanation is through example.  The example being the loading
//of a file -- there are many stages of the map loading.  In JA2, the first step is to load the tileset.
//Because it is a large chunk of the total loading of the map, we may gauge that it takes up 30% of the
//total load.  Because it is also at the beginning, we would pass in the arguments ( 0, 30, "text" ).
//As the process animates using UpdateProgressBar( 0 to 100 ), the total progress bar will only reach 30%
//at the 100% mark within UpdateProgressBar.  At that time, you would go onto the next step, resetting the
//relative start and end percentage from 30 to whatever, until your done.
void SetRelativeStartAndEndPercentage(UINT8 id, UINT32 uiRelStartPerc, UINT32 uiRelEndPerc, ST::string const& unused);

//This part renders the progress bar at the percentage level that you specify.  If you have set relative
//percentage values in the above function, then the uiPercentage will be reflected based off of the relative
//percentages.
void RenderProgressBar( UINT8 ubID, UINT32 uiPercentage );


//Sets the color of the progress bars main color.
void SetProgressBarColor( UINT8 ubID, UINT8 ubColorFillRed, UINT8 ubColorFillGreen, UINT8 ubColorFillBlue );

#endif
