#ifdef PRECOMPILEDHEADERS
	#include "Utils All.h"
#else
	#include <stdio.h>
	#include <stdarg.h>
	#include <time.h>
	#include "sgp.h"
	#include "container.h"
	#include "wcheck.h"
	#include "Event Pump.h"
	#include "Timer.h"
	#include "Soldier Control.h"
	#include "Sound Control.h"
	#include "Overhead.h"
	#include "weapons.h"
	#include "Animation Control.h"
	#include "opplist.h"
#endif

#ifdef NETWORKED
#include "Networking.h"
#include "NetworkEvent.h"
#endif

UINT8 gubEncryptionArray4[ BASE_NUMBER_OF_ROTATION_ARRAYS * 3 ][ NEW_ROTATION_ARRAY_SIZE ] =
{
    {
    177,131,58,218,175,130,210,
    59,25,190,170,189,227,245,
    104,118,7,168,136,178,184,
    4,27,64,199,101,160,24,
    83,177,178,232,185,40,122,
    109,38,253,160,14,133,106,
    190,206,58,102,244,229,124
    },

    {
    201,183,24,153,17,111,47,
    19,116,248,160,215,143,180,
    195,122,74,29,158,193,73,
    159,193,93,140,172,31,38,
    129,181,96,183,56,29,172,
    191,252,183,91,214,254,247,
    135,66,76,87,1,112,214
    },

    {
    28,138,132,173,9,230,220,
    57,32,166,202,153,246,49,
    119,246,212,1,50,230,19,
    135,20,173,235,182,61,143,
    76,162,155,224,182,54,123,
    29,223,119,200,175,10,66,
    87,171,204,12,1,44,68
    },

    {
    142,1,145,102,227,81,84,
    108,1,120,168,9,14,135,
    165,118,141,183,47,235,212,
    109,85,167,185,255,89,18,
    12,36,41,24,181,66,107,
    96,5,160,235,87,33,173,
    140,207,20,70,135,44,102
    },

    {
    85,38,191,199,157,183,122,
    186,2,236,106,181,53,100,
    69,189,22,137,117,230,153,
    150,211,184,157,220,165,234,
    113,19,238,122,120,233,181,
    130,182,140,76,219,162,90,
    66,242,19,125,8,215,162
    },

    {
    167,227,240,92,158,80,82,
    18,222,92,51,44,252,34,
    112,74,12,237,158,156,179,
    48,154,47,61,204,24,36,
    62,228,29,23,43,176,239,
    144,117,131,97,219,129,138,
    243,31,33,108,211,255,196
    },

    {
    83,29,187,49,130,95,252,
    214,6,223,87,149,168,140,
    172,33,170,24,103,180,107,
    63,212,92,143,155,126,129,
    190,25,211,244,151,103,7,
    51,227,119,104,18,177,110,
    33,155,104,15,159,62,103
    },

    {
    9,151,31,80,46,220,174,
    147,4,3,76,83,157,9,
    39,52,79,252,16,63,25,
    215,131,90,79,128,143,128,
    117,57,224,68,198,98,158,
    191,90,222,94,135,106,238,
    212,168,164,233,138,106,14
    },

    {
    159,70,200,99,183,101,85,
    117,251,218,149,119,199,12,
    102,190,144,68,145,212,202,
    250,32,118,13,149,136,91,
    203,247,68,69,124,122,248,
    92,125,154,189,60,160,12,
    148,182,162,108,187,198,113
    },

    {
    75,226,151,153,186,223,56,
    174,53,4,212,8,241,147,
    253,166,123,148,241,166,172,
    118,103,6,219,178,187,41,
    89,250,28,220,20,27,134,
    77,125,14,40,175,125,114,
    15,2,160,240,169,186,63
    },

    {
    165,251,182,208,45,34,74,
    41,106,244,193,205,160,100,
    51,41,232,106,237,201,184,
    35,112,98,118,238,38,211,
    191,33,221,226,126,55,61,
    224,82,255,19,252,131,188,
    220,111,242,172,93,61,67
    },

    {
    246,142,205,184,114,219,1,
    246,63,229,6,41,167,187,
    173,185,75,205,96,37,252,
    142,135,8,82,240,138,254,
    107,202,42,104,198,71,98,
    173,197,100,156,37,180,190,
    88,30,229,135,12,58,198
    },

    {
    7,121,166,123,114,32,78,
    223,125,21,155,124,116,240,
    222,91,57,5,254,51,4,
    161,122,236,49,146,88,235,
    253,149,42,31,140,254,163,
    71,253,70,242,115,10,171,
    101,38,217,187,117,31,141
    },

    {
    220,220,58,61,163,182,106,
    201,38,91,53,99,7,253,
    179,12,108,175,148,246,162,
    217,7,36,146,193,22,3,
    220,68,101,117,184,62,195,
    25,94,226,155,28,43,110,
    161,132,70,110,201,58,228
    },

    {
    39,253,170,4,47,122,100,
    182,223,98,128,205,167,103,
    42,16,227,30,43,181,80,
    212,194,100,164,123,181,97,
    126,145,213,51,44,135,240,
    100,105,151,106,174,180,134,
    106,49,72,73,237,2,84
    },

    {
    1,140,181,150,80,96,57,
    214,115,209,143,122,31,162,
    201,171,155,38,225,68,12,
    219,180,253,105,97,208,19,
    20,8,84,223,139,223,146,
    150,53,161,187,167,163,61,
    45,242,115,110,195,89,15
    },

    {
    50,197,196,115,105,176,64,
    87,141,157,64,185,202,118,
    158,70,79,168,121,141,57,
    163,128,141,228,192,195,115,
    15,227,176,28,130,126,54,
    75,200,45,202,7,158,179,
    77,23,142,127,110,31,141
    },

    {
    123,175,80,224,82,146,27,
    61,247,16,236,96,150,244,
    102,13,165,47,253,185,96,
    178,149,204,82,2,235,182,
    47,249,110,211,181,241,87,
    93,208,215,155,65,168,65,
    152,71,236,50,249,80,249
    },

    {
    139,219,5,39,213,136,215,
    228,108,228,169,234,173,243,
    229,45,65,105,121,208,18,
    202,118,209,11,19,178,162,
    59,74,82,99,111,28,119,
    6,217,203,9,252,227,146,
    217,194,195,213,12,221,229
    },

    {
    49,9,169,202,201,231,152,
    102,226,150,26,173,50,161,
    241,73,224,232,42,44,182,
    48,85,6,112,192,109,58,
    164,25,233,113,68,229,93,
    83,32,42,74,152,119,240,
    95,234,245,83,222,203,49
    },

    {
    58,104,2,6,164,206,186,
    224,222,73,218,87,103,158,
    186,30,242,149,198,193,89,
    94,43,38,197,36,33,64,
    7,136,243,253,80,61,90,
    223,72,116,47,46,190,94,
    50,77,217,111,227,35,30
    },

    {
    161,5,6,26,113,239,46,
    35,195,65,36,225,119,8,
    31,27,206,249,207,129,119,
    218,138,239,90,154,78,217,
    247,85,161,87,123,185,175,
    152,74,2,181,30,66,9,
    30,147,91,147,86,146,232
    },

    {
    111,204,65,49,124,16,12,
    149,51,137,89,252,190,203,
    155,194,84,231,136,213,197,
    202,212,73,251,17,80,12,
    106,75,180,1,15,52,1,
    36,27,194,180,13,226,151,
    210,106,27,190,237,194,16
    },

    {
    63,38,43,226,100,106,44,
    100,72,214,138,170,142,137,
    51,246,203,190,250,5,139,
    146,105,90,117,73,83,168,
    127,89,106,238,176,54,135,
    79,97,13,229,87,119,110,
    253,184,151,32,70,14,191
    },

    {
    3,134,209,61,87,248,16,
    140,69,243,241,249,240,83,
    184,156,4,81,67,134,241,
    25,176,185,228,181,65,200,
    143,165,255,165,222,193,94,
    146,188,178,68,171,218,177,
    198,3,208,189,1,204,100
    },

    {
    22,66,132,87,17,19,147,
    142,42,56,105,247,102,68,
    124,234,205,209,46,130,54,
    254,10,162,22,63,87,26,
    148,32,114,179,139,23,239,
    128,191,233,238,76,251,218,
    169,126,70,24,167,70,244
    },

    {
    156,153,114,200,16,71,190,
    214,189,154,249,102,57,40,
    212,144,211,72,187,194,106,
    32,131,241,173,161,83,169,
    128,245,153,168,115,72,94,
    90,15,107,218,206,232,88,
    162,109,52,236,54,42,212
    },

    {
    242,93,89,143,226,33,252,
    126,83,174,156,75,156,21,
    131,85,147,30,130,90,228,
    40,2,178,7,79,138,196,
    44,201,134,186,63,196,113,
    28,89,156,53,96,166,205,
    213,15,57,112,70,138,190
    },

    {
    148,127,27,140,97,157,84,
    2,67,71,99,182,13,250,
    113,100,84,156,156,16,242,
    216,176,107,248,176,40,208,
    235,220,159,243,169,56,65,
    159,199,244,77,142,201,210,
    81,198,4,7,43,126,122
    },

    {
    23,101,250,171,81,20,89,
    128,51,189,208,81,88,108,
    69,78,59,131,199,38,130,
    102,215,54,7,153,251,58,
    80,98,150,172,193,144,115,
    212,46,119,19,184,10,188,
    14,237,241,16,173,125,191
    },

    {
    97,19,206,235,176,242,227,
    101,12,97,196,163,218,152,
    104,40,3,63,3,39,15,
    31,4,138,217,199,156,12,
    239,16,55,165,35,249,155,
    252,132,220,53,22,68,104,
    63,246,158,93,50,126,104
    },

    {
    136,86,29,88,238,198,176,
    148,161,188,49,187,54,131,
    114,216,90,238,169,131,120,
    150,234,162,232,205,105,255,
    161,150,158,52,226,116,23,
    23,218,114,33,50,169,53,
    18,105,210,117,86,89,125
    },

    {
    178,13,149,148,121,91,82,
    38,29,64,153,12,44,11,
    161,74,139,68,88,35,124,
    42,191,86,119,137,56,127,
    38,40,69,17,89,107,234,
    226,90,137,67,200,96,232,
    79,43,143,239,180,33,81
    },

    {
    254,112,239,101,191,19,131,
    123,153,174,191,181,101,3,
    46,221,48,253,2,188,62,
    48,225,85,249,74,118,101,
    32,58,51,4,196,252,82,
    178,127,201,119,193,107,155,
    252,77,54,194,83,192,240
    },

    {
    141,117,43,233,154,235,63,
    50,253,75,91,22,195,180,
    105,62,225,65,245,173,73,
    222,82,82,21,138,31,105,
    125,103,13,104,162,169,56,
    165,203,198,233,117,38,240,
    39,243,174,64,209,60,76
    },

    {
    159,196,56,192,208,188,198,
    122,162,99,117,240,187,85,
    122,142,242,127,51,213,189,
    170,210,194,148,133,133,29,
    217,17,181,176,52,189,178,
    36,200,26,107,114,103,92,
    121,166,6,179,52,234,142
    },

    {
    164,250,146,128,92,129,33,
    77,59,62,251,236,133,203,
    203,174,137,91,50,172,10,
    10,235,227,82,61,235,101,
    200,199,150,159,83,13,162,
    42,100,220,210,25,21,56,
    254,76,23,196,161,169,214
    },

    {
    22,112,203,180,148,28,10,
    150,46,218,99,193,146,186,
    52,61,36,113,59,173,108,
    178,223,233,55,161,52,50,
    17,128,164,93,112,43,216,
    202,37,131,180,130,100,82,
    237,77,23,199,63,30,158
    },

    {
    47,156,173,221,226,110,149,
    164,39,71,206,74,250,7,
    218,158,122,242,131,252,231,
    165,119,158,248,92,168,3,
    119,157,186,136,214,160,95,
    7,206,124,253,149,155,145,
    105,46,239,35,23,126,58
    },

    {
    159,159,134,88,135,222,68,
    64,110,148,41,201,20,219,
    144,41,1,72,230,41,167,
    16,194,210,133,8,165,130,
    47,225,148,17,203,103,84,
    43,236,41,98,94,76,91,
    133,8,79,85,16,175,221
    },

    {
    115,161,236,118,192,144,104,
    255,59,191,32,78,18,14,
    108,203,171,32,254,80,206,
    178,84,132,206,60,193,226,
    4,8,188,193,118,133,91,
    87,13,9,191,67,23,181,
    124,134,134,191,130,95,195
    },

    {
    202,187,20,83,145,231,14,
    74,153,208,76,200,34,106,
    157,107,9,255,160,250,224,
    5,251,57,230,149,200,39,
    39,79,170,143,95,141,130,
    62,7,170,74,174,17,85,
    238,144,174,152,202,247,202
    },

    {
    173,144,151,112,43,60,201,
    219,175,238,226,103,153,58,
    119,15,237,206,33,193,243,
    253,102,63,183,24,77,166,
    242,11,72,143,127,79,246,
    188,238,128,118,67,87,183,
    175,79,239,6,108,52,153
    },

    {
    19,148,26,226,91,100,230,
    169,250,250,171,233,136,16,
    69,214,232,55,111,210,171,
    116,246,96,204,142,82,166,
    104,230,80,151,80,59,13,
    94,247,35,244,34,181,156,
    5,183,210,139,3,95,85
    },

    {
    125,219,35,9,131,243,232,
    189,133,142,219,41,228,14,
    55,114,188,66,24,210,163,
    124,99,75,61,165,94,228,
    127,6,12,114,246,253,243,
    77,231,101,85,4,164,200,
    117,117,240,63,15,76,203
    },

    {
    9,134,85,112,185,245,169,
    239,37,166,32,55,179,23,
    7,216,118,32,145,113,152,
    81,254,2,110,33,225,26,
    221,253,151,32,87,233,78,
    155,18,169,163,207,196,34,
    77,135,154,3,192,96,146
    },

    {
    75,91,117,7,210,246,189,
    146,84,107,248,11,41,245,
    123,204,73,243,191,15,151,
    245,1,109,44,157,204,112,
    153,55,210,68,226,149,209,
    80,22,29,254,243,76,121,
    172,17,1,242,167,176,116
    },

    {
    230,254,102,33,70,173,175,
    228,171,233,19,239,231,124,
    38,24,147,237,60,159,138,
    243,245,16,215,222,97,60,
    1,116,241,4,46,3,242,
    237,32,9,197,54,119,77,
    137,95,57,179,123,232,16
    },

    {
    183,187,60,61,212,166,108,
    187,190,151,215,122,188,39,
    86,57,254,185,238,247,79,
    142,209,107,232,255,138,122,
    212,48,23,47,163,228,255,
    32,224,187,246,195,5,116,
    28,160,96,142,147,222,171
    },

    {
    229,93,42,194,149,50,89,
    58,204,91,225,69,212,47,
    124,178,247,73,3,94,144,
    13,244,241,225,151,159,73,
    16,171,96,76,202,185,250,
    174,10,152,121,68,111,6,
    140,114,166,133,234,104,94
    },

    {
    186,157,156,42,213,188,78,
    242,32,136,103,58,86,99,
    171,229,203,151,101,196,15,
    109,148,140,4,130,208,135,
    52,214,212,167,66,145,243,
    54,61,240,104,223,212,87,
    181,68,19,247,26,217,165
    },

    {
    20,68,71,101,64,63,241,
    25,177,222,2,32,2,140,
    138,20,101,194,10,249,205,
    25,11,29,236,53,13,41,
    149,115,115,159,108,172,186,
    56,97,223,149,79,240,33,
    74,244,153,175,230,173,139
    },

    {
    112,160,137,249,193,254,166,
    20,237,27,169,28,25,166,
    89,133,11,238,230,171,153,
    149,249,253,37,58,38,48,
    226,198,210,91,107,161,50,
    129,73,109,104,128,240,157,
    106,209,245,142,188,44,209
    },

    {
    94,155,95,27,57,178,126,
    126,85,239,193,85,88,143,
    21,44,88,255,219,33,48,
    160,202,89,107,12,103,123,
    111,249,20,123,41,72,38,
    150,235,92,3,204,197,84,
    176,187,243,59,226,49,184
    },

    {
    202,228,64,73,214,118,70,
    4,246,243,163,181,137,217,
    140,24,167,19,201,69,35,
    1,179,95,32,201,86,26,
    111,210,1,98,46,226,39,
    226,217,237,225,141,89,245,
    240,244,44,2,143,156,58
    },

    {
    157,246,3,119,213,223,157,
    231,63,233,113,19,222,44,
    151,145,130,222,78,190,130,
    93,215,44,86,109,232,234,
    249,24,206,73,167,235,11,
    22,201,8,161,6,173,196,
    202,81,229,72,71,33,216
    },

    {
    245,6,229,112,241,89,47,
    99,172,118,22,20,193,113,
    230,122,251,107,255,10,232,
    55,158,24,188,30,159,178,
    246,6,211,101,134,171,245,
    216,223,48,231,212,198,167,
    71,142,46,234,237,200,4
    },
};

// GLobals used here, for each event structure used,
// Used as globals for stack reasons
EV_E_PLAYSOUND			EPlaySound;

EV_S_CHANGESTATE			SChangeState;
EV_S_CHANGEDEST				SChangeDest;
EV_S_SETPOSITION			SSetPosition;
EV_S_GETNEWPATH				SGetNewPath;
EV_S_BEGINTURN				SBeginTurn;
EV_S_CHANGESTANCE			SChangeStance;
EV_S_SETDIRECTION			SSetDirection;
EV_S_SETDESIREDDIRECTION			SSetDesiredDirection;
EV_S_BEGINFIREWEAPON	SBeginFireWeapon;
EV_S_FIREWEAPON				SFireWeapon;
EV_S_WEAPONHIT				SWeaponHit;
EV_S_STRUCTUREHIT			SStructureHit;
EV_S_WINDOWHIT				SWindowHit;
EV_S_MISS							SMiss;
EV_S_NOISE						SNoise;
EV_S_STOP_MERC				SStopMerc;
EV_S_SENDPATHTONETWORK SUpdateNetworkSoldier;

extern	BOOLEAN				gfAmINetworked;

BOOLEAN AddGameEventToQueue( UINT32 uiEvent, UINT16 usDelay, PTR pEventData, UINT8 ubQueueID );
BOOLEAN ExecuteGameEvent( EVENT *pEvent );


BOOLEAN AddGameEvent( UINT32 uiEvent, UINT16 usDelay, PTR pEventData )
{
	if (usDelay == DEMAND_EVENT_DELAY)
	{
		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("AddGameEvent: Sending Local and network #%d", uiEvent));
		#ifdef NETWORKED
		if(gfAmINetworked)
			SendEventToNetwork(uiEvent, usDelay, pEventData);
		#endif
		return( AddGameEventToQueue( uiEvent, 0, pEventData, DEMAND_EVENT_QUEUE ) );
	}
	else if( uiEvent < EVENTS_LOCAL_AND_NETWORK)
	{
		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("AddGameEvent: Sending Local and network #%d", uiEvent));
		#ifdef NETWORKED
		if(gfAmINetworked)
			SendEventToNetwork(uiEvent, usDelay, pEventData);
		#endif
		return( AddGameEventToQueue( uiEvent, usDelay, pEventData, PRIMARY_EVENT_QUEUE ) );
	}
	else if( uiEvent < EVENTS_ONLY_USED_LOCALLY)
	{
		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("AddGameEvent: Sending Local #%d", uiEvent));
		return( AddGameEventToQueue( uiEvent, usDelay, pEventData, PRIMARY_EVENT_QUEUE ) );
	}
	else if( uiEvent < EVENTS_ONLY_SENT_OVER_NETWORK)
	{
		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("AddGameEvent: Sending network #%d", uiEvent));
		#ifdef NETWORKED
		if(gfAmINetworked)
			SendEventToNetwork(uiEvent, usDelay, pEventData);
		#endif
		return(TRUE);
	}
	// There is an error with the event
	else
		return(FALSE);
}

BOOLEAN AddGameEventFromNetwork( UINT32 uiEvent, UINT16 usDelay, PTR pEventData )
{
		return( AddGameEventToQueue( uiEvent, usDelay, pEventData, PRIMARY_EVENT_QUEUE ) );
}

BOOLEAN AddGameEventToQueue( UINT32 uiEvent, UINT16 usDelay, PTR pEventData, UINT8 ubQueueID )
{
	 UINT32		uiDataSize;

	 // Check range of Event ui
	 if ( uiEvent < 0 || uiEvent > NUM_EVENTS )
	 {
		  // Set debug message!
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Unknown event type");
			return( FALSE );
	 }

	 // Switch on event type and set size accordingly
	 switch( uiEvent )
	 {
			case E_PLAYSOUND:

				uiDataSize = sizeof( EV_E_PLAYSOUND );
				break;

			case S_CHANGESTATE:

				uiDataSize = sizeof( EV_S_CHANGESTATE );
				break;


			case S_CHANGEDEST:

				uiDataSize = sizeof( EV_S_CHANGEDEST );
				break;


			case S_SETPOSITION:

				uiDataSize = sizeof( EV_S_SETPOSITION );
				break;

			case S_GETNEWPATH:

				uiDataSize = sizeof( EV_S_GETNEWPATH );
				break;

			case S_BEGINTURN:

				uiDataSize = sizeof( EV_S_BEGINTURN );
				break;

			case S_CHANGESTANCE:

				uiDataSize = sizeof( EV_S_CHANGESTANCE );
				break;

			case S_SETDIRECTION:

				uiDataSize = sizeof( EV_S_SETDIRECTION );
				break;

			case S_SETDESIREDDIRECTION:

				uiDataSize = sizeof( EV_S_SETDESIREDDIRECTION );
				break;

			case S_FIREWEAPON:

				uiDataSize = sizeof( EV_S_FIREWEAPON );
				break;

			case S_BEGINFIREWEAPON:

				uiDataSize = sizeof( EV_S_BEGINFIREWEAPON );
				//Delay this event
				break;

			case S_WEAPONHIT:

				uiDataSize = sizeof( EV_S_WEAPONHIT );
				break;

			case S_STRUCTUREHIT:
				uiDataSize = sizeof( EV_S_STRUCTUREHIT );
				break;

			case S_WINDOWHIT:
				uiDataSize = sizeof( EV_S_STRUCTUREHIT );
				break;

			case S_MISS:
				uiDataSize = sizeof( EV_S_MISS );
				break;

			case S_NOISE:
				uiDataSize = sizeof( EV_S_NOISE );
				break;

			case S_STOP_MERC:
				uiDataSize = sizeof( EV_S_STOP_MERC );
				break;

			case S_SENDPATHTONETWORK:
				uiDataSize = sizeof(EV_S_SENDPATHTONETWORK);
				break;

			case 	S_UPDATENETWORKSOLDIER:
				uiDataSize = sizeof(EV_S_UPDATENETWORKSOLDIER);
				break;

			default:

				// Set debug msg: unknown message!
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Event Type mismatch");
				return( FALSE );

	 }


	 CHECKF( AddEvent( uiEvent, usDelay,  pEventData, uiDataSize, ubQueueID ) );

	 // successful
	 return( TRUE );
}

BOOLEAN  DequeAllGameEvents( BOOLEAN fExecute )
{
	EVENT					*pEvent;
	UINT32				uiQueueSize, cnt;
	BOOLEAN				fCompleteLoop = FALSE;
	// First dequeue all primary events


	while( EventQueueSize( PRIMARY_EVENT_QUEUE ) > 0 )
	{
		// Get Event
		if ( RemoveEvent( &pEvent, 0, PRIMARY_EVENT_QUEUE) == FALSE )
		{
			return( FALSE );
		}

		if ( fExecute )
		{
			// Check if event has a delay and add to secondary queue if so
			if ( pEvent->usDelay > 0 )
			{
				AddGameEventToQueue( pEvent->uiEvent, pEvent->usDelay, pEvent->pData, SECONDARY_EVENT_QUEUE );
			}
			else
			{
				ExecuteGameEvent( pEvent );
			}
		}

		// Delete event
		FreeEvent( pEvent );

	};

	// NOW CHECK SECONDARY QUEUE FOR ANY EXPRIED EVENTS
	// Get size of queue
	uiQueueSize = EventQueueSize( SECONDARY_EVENT_QUEUE );

	for ( cnt = 0; cnt < uiQueueSize; cnt++ )
	{
		if ( PeekEvent( &pEvent, cnt, SECONDARY_EVENT_QUEUE) == FALSE )
		{
			return( FALSE );
		}

		// Check time
		if ( ( GetJA2Clock() - pEvent->TimeStamp ) > pEvent->usDelay )
		{
			if ( fExecute )
			{
				ExecuteGameEvent( pEvent );
			}

			// FLag as expired
			pEvent->uiFlags = EVENT_EXPIRED;

		}

	}

	do
	{
		uiQueueSize = EventQueueSize( SECONDARY_EVENT_QUEUE );

		for ( cnt = 0; cnt < uiQueueSize; cnt++ )
		{
			if ( PeekEvent( &pEvent, cnt, SECONDARY_EVENT_QUEUE) == FALSE )
			{
				return( FALSE );
			}

			// Check time
			if ( pEvent->uiFlags & EVENT_EXPIRED )
			{
				RemoveEvent( &pEvent, cnt, SECONDARY_EVENT_QUEUE );
				FreeEvent( pEvent );
				// Restart loop
				break;

			}

		}

		if ( cnt == uiQueueSize )
		{
			fCompleteLoop = TRUE;
		}

	} while( fCompleteLoop == FALSE );

	return( TRUE );
}

BOOLEAN DequeueAllDemandGameEvents( BOOLEAN fExecute )
{
	EVENT					*pEvent;
	BOOLEAN				fCompleteLoop = FALSE;

	// Dequeue all events on the demand queue (only)

	while( EventQueueSize( DEMAND_EVENT_QUEUE ) > 0 )
	{
		// Get Event
		if ( RemoveEvent( &pEvent, 0, DEMAND_EVENT_QUEUE) == FALSE )
		{
			return( FALSE );
		}

		if ( fExecute )
		{
			// Check if event has a delay and add to secondary queue if so
			if ( pEvent->usDelay > 0 )
			{
				AddGameEventToQueue( pEvent->uiEvent, pEvent->usDelay, pEvent->pData, SECONDARY_EVENT_QUEUE );
			}
			else
			{
				ExecuteGameEvent( pEvent );
			}
		}

		// Delete event
		FreeEvent( pEvent );

	};

	return( TRUE );
}



BOOLEAN ExecuteGameEvent( EVENT *pEvent )
{
	SOLDIERTYPE		*pSoldier;

	// Switch on event type
	switch( pEvent->uiEvent )
	{
			case E_PLAYSOUND:

				memcpy( &EPlaySound, pEvent->pData, pEvent->uiDataSize );

				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Play Sound");
				PlayJA2Sample( EPlaySound.usIndex, EPlaySound.usRate, EPlaySound.ubVolume, EPlaySound.ubLoops, EPlaySound.uiPan );
				break;

			case S_CHANGESTATE:

				memcpy( &SChangeState, pEvent->pData, pEvent->uiDataSize );

				// Get soldier pointer from ID
				if ( GetSoldier( &pSoldier, SChangeState.usSoldierID ) == FALSE )
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SChangeState.uiUniqueId )
				{
					break;
				}

				// Call soldier function
//				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Event Pump: ChangeState %S (%d)", gAnimControl[ SChangeState.ubNewState ].zAnimStr, SChangeState.usSoldierID ) );
				EVENT_InitNewSoldierAnim( pSoldier, SChangeState.usNewState, SChangeState.usStartingAniCode, SChangeState.fForce );
				break;

			case S_CHANGEDEST:

				memcpy( &SChangeDest, pEvent->pData, pEvent->uiDataSize );

				// Get soldier pointer from ID
				if ( GetSoldier( &pSoldier, SChangeDest.usSoldierID ) == FALSE )
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Event Pump: Invalid Soldier ID #%d", SChangeDest.usSoldierID) );
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SChangeDest.uiUniqueId )
				{
					break;
				}

				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Change Dest");
				EVENT_SetSoldierDestination( pSoldier, SChangeDest.usNewDestination );
				break;

			case S_SETPOSITION:

				memcpy( &SSetPosition, pEvent->pData, pEvent->uiDataSize );

				// Get soldier pointer from ID
				if ( GetSoldier( &pSoldier, SSetPosition.usSoldierID ) == FALSE )
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SSetPosition.uiUniqueId )
				{
					break;
				}

				// Call soldier function
//				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: SetPosition ( %f %f ) ( %d )", SSetPosition.dNewXPos, SSetPosition.dNewYPos, SSetPosition.usSoldierID ) );
				EVENT_SetSoldierPosition( pSoldier, SSetPosition.dNewXPos, SSetPosition.dNewYPos );
				break;

			case S_GETNEWPATH:

				memcpy( &SGetNewPath, pEvent->pData, pEvent->uiDataSize );

				// Get soldier pointer from ID
				if ( GetSoldier( &pSoldier, SGetNewPath.usSoldierID ) == FALSE )
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SGetNewPath.uiUniqueId )
				{
					break;
				}
				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: GetNewPath");
				EVENT_GetNewSoldierPath( pSoldier, SGetNewPath.sDestGridNo, SGetNewPath.usMovementAnim );
				break;

			case S_BEGINTURN:

				memcpy( &SBeginTurn, pEvent->pData, pEvent->uiDataSize );

				// Get soldier pointer from ID
				if ( GetSoldier( &pSoldier, SBeginTurn.usSoldierID ) == FALSE )
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SBeginTurn.uiUniqueId )
				{
					break;
				}

				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: BeginTurn");
				EVENT_BeginMercTurn( pSoldier, FALSE, 0 );
				break;

			case S_CHANGESTANCE:

				memcpy( &SChangeStance, pEvent->pData, pEvent->uiDataSize );

				// Get soldier pointer from ID
				if ( GetSoldier( &pSoldier, SChangeStance.usSoldierID ) == FALSE )
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SChangeStance.uiUniqueId )
				{
					break;
				}
				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: ChangeStance");
				ChangeSoldierStance( pSoldier, SChangeStance.ubNewStance );
				break;

			case S_SETDIRECTION:

				memcpy( &SSetDirection, pEvent->pData, pEvent->uiDataSize );

				// Get soldier pointer from ID
				if ( GetSoldier( &pSoldier, SSetDirection.usSoldierID ) == FALSE )
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SSetDirection.uiUniqueId )
				{
					break;
				}

				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: SetDirection: Dir( %d )", SSetDirection.usNewDirection)  );
				EVENT_SetSoldierDirection( pSoldier, SSetDirection.usNewDirection );
				break;

			case S_SETDESIREDDIRECTION:

				memcpy( &SSetDesiredDirection, pEvent->pData, pEvent->uiDataSize );

				// Get soldier pointer from ID
				if ( GetSoldier( &pSoldier, SSetDesiredDirection.usSoldierID ) == FALSE )
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SSetDesiredDirection.uiUniqueId )
				{
					break;
				}

				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: SetDesiredDirection: Dir( %d )", SSetDesiredDirection.usDesiredDirection)  );
				EVENT_SetSoldierDesiredDirection( pSoldier, SSetDesiredDirection.usDesiredDirection );
				break;


			case S_BEGINFIREWEAPON:

				memcpy( &SBeginFireWeapon, pEvent->pData, pEvent->uiDataSize );

				// Get soldier pointer from ID
				if ( GetSoldier( &pSoldier, SBeginFireWeapon.usSoldierID ) == FALSE )
				{
					 pSoldier = NULL;
					 break;
					 // Handle Error?
					 // DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SBeginFireWeapon.uiUniqueId )
				{
					break;
				}

				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Begin Fire Weapon");
				pSoldier->sTargetGridNo = SBeginFireWeapon.sTargetGridNo;
				pSoldier->bTargetLevel = SBeginFireWeapon.bTargetLevel;
				pSoldier->bTargetCubeLevel = SBeginFireWeapon.bTargetCubeLevel;
				EVENT_FireSoldierWeapon( pSoldier, SBeginFireWeapon.sTargetGridNo );
				break;

			case S_FIREWEAPON:

				memcpy( &SFireWeapon, pEvent->pData, pEvent->uiDataSize );

				// Get soldier pointer from ID
				if ( GetSoldier( &pSoldier, SFireWeapon.usSoldierID ) == FALSE )
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

					// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SFireWeapon.uiUniqueId )
				{
					break;
				}


				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: FireWeapon");
				pSoldier->sTargetGridNo = SFireWeapon.sTargetGridNo;
				pSoldier->bTargetLevel = SFireWeapon.bTargetLevel;
				pSoldier->bTargetCubeLevel = SFireWeapon.bTargetCubeLevel;
				FireWeapon( pSoldier, SFireWeapon.sTargetGridNo  );
				break;


			case S_WEAPONHIT:

				memcpy( &SWeaponHit, pEvent->pData, pEvent->uiDataSize );
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: WeaponHit %d Damage", SWeaponHit.sDamage ) );
				WeaponHit( SWeaponHit.usSoldierID, SWeaponHit.usWeaponIndex, SWeaponHit.sDamage, SWeaponHit.sBreathLoss, SWeaponHit.usDirection, SWeaponHit.sXPos, SWeaponHit.sYPos, SWeaponHit.sZPos, SWeaponHit.sRange, SWeaponHit.ubAttackerID, SWeaponHit.fHit, SWeaponHit.ubSpecial, SWeaponHit.ubLocation );
				break;

			case S_STRUCTUREHIT:

				memcpy( &SStructureHit, pEvent->pData, pEvent->uiDataSize );
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: StructureHit" ) );
				StructureHit( SStructureHit.iBullet, SStructureHit.usWeaponIndex, SStructureHit.bWeaponStatus, SStructureHit.ubAttackerID, SStructureHit.sXPos, SStructureHit.sYPos, SStructureHit.sZPos, SStructureHit.usStructureID, SStructureHit.iImpact, TRUE );
				break;

			case S_WINDOWHIT:

				memcpy( &SWindowHit, pEvent->pData, pEvent->uiDataSize );
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: WindowHit" ) );
				WindowHit( SWindowHit.sGridNo, SWindowHit.usStructureID, SWindowHit.fBlowWindowSouth, SWindowHit.fLargeForce );
				break;

			case S_MISS:

				memcpy( &SMiss, pEvent->pData, pEvent->uiDataSize );
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: Shot Miss ( obsolete )" ) );
				//ShotMiss( SMiss.ubAttackerID );
				break;

			case S_NOISE:
				memcpy( &SNoise, pEvent->pData, pEvent->uiDataSize );
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: Noise from %d at %d/%d, type %d volume %d", SNoise.ubNoiseMaker, SNoise.sGridNo, SNoise.bLevel, SNoise.ubNoiseType, SNoise.ubVolume ) );
				OurNoise( SNoise.ubNoiseMaker, SNoise.sGridNo, SNoise.bLevel, SNoise.ubTerrType, SNoise.ubVolume, SNoise.ubNoiseType );
				break;

			case S_STOP_MERC:

				memcpy( &SStopMerc, pEvent->pData, pEvent->uiDataSize );

				// Get soldier pointer from ID
				if ( GetSoldier( &pSoldier, SStopMerc.usSoldierID ) == FALSE )
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				if( pSoldier-> uiUniqueSoldierIdValue != SStopMerc.uiUniqueId )
				{
					break;
				}

				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Event Pump: Stop Merc at Gridno %d", SStopMerc.sGridNo ));
				EVENT_StopMerc( pSoldier, SStopMerc.sGridNo, SStopMerc.bDirection );
				break;


			default:

				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Event Received");
				return( FALSE );

	}

	return( TRUE );
}


BOOLEAN ClearEventQueue( void )
{
	// clear out the event queue
	EVENT					*pEvent;
	while( EventQueueSize( PRIMARY_EVENT_QUEUE ) > 0 )
	{
		// Get Event
		if ( RemoveEvent( &pEvent, 0, PRIMARY_EVENT_QUEUE) == FALSE )
		{
			return( FALSE );
		}
	}

	return( TRUE );
}

