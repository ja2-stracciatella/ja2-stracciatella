#ifndef __ENGLISH_
#define __ENGLISH_

#include "Types.h"

#define SHIFT               16
#define CTRL                17
#define ALT                 18

#define F1                  124
#define F2                  125
#define F3                  126
#define F4                  127
#define F5                  128
#define F6                  129
#define F7                  130
#define F8                  131
#define F9                  132
#define F10                 133
#define F11                 134
#define F12                 135

#define SHIFT_F1            368
#define SHIFT_F2            369
#define SHIFT_F3            370
#define SHIFT_F4            371
#define SHIFT_F5            372
#define SHIFT_F6            373
#define SHIFT_F7            374
#define SHIFT_F8            375
#define SHIFT_F9            376
#define SHIFT_F10           377
#define SHIFT_F11           378
#define SHIFT_F12           379

#define ALT_F1              624
#define ALT_F2              625
#define ALT_F3              626
#define ALT_F4              627
#define ALT_F5              628
#define ALT_F6              629
#define ALT_F7              630
#define ALT_F8              631
#define ALT_F9              632
#define ALT_F10             633
#define ALT_F11             634
#define ALT_F12             635

#define CTRL_F1             880
#define CTRL_F2             881
#define CTRL_F3             882
#define CTRL_F4             883
#define CTRL_F5             884
#define CTRL_F6             885
#define CTRL_F7             886
#define CTRL_F8             887
#define CTRL_F9             888
#define CTRL_F10            889
#define CTRL_F11            890
#define CTRL_F12            891

#define ESC                 27
#define TAB                 9
#define CAPS                20
#define SCRL_LOCK           145 //*** this may be incorrect! DB
#define SNAPSHOT						44
#define PAUSE               19
#define NUM_LOCK            144
#define BACKSPACE           8

#define INSERT              245
#define DEL                 246

#ifndef JA2
// Stupid definition causes problems with headers that use the keyword END -- DB
	#define KEY_END           247
#else
	#define END								247
#endif

#define DNARROW             248
#define PGDN                249
#define LEFTARROW           250
#define RIGHTARROW          251
#define HOME                252
#define UPARROW             253
#define PGUP                254

#define SHIFT_TAB           265

#define SHIFT_INSERT        501
#define SHIFT_DELETE        502
#define SHIFT_END           503
#define SHIFT_DNARROW       504
#define SHIFT_PGDN          505
#define SHIFT_LEFTARROW     506
#define SHIFT_RIGHTARROW    507
#define SHIFT_HOME          508
#define SHIFT_UPARROW       509
#define SHIFT_PGUP          510

#define ALT_TAB             521

#define ALT_INSERT          757
#define ALT_DELETE          758
#define ALT_END             759
#define ALT_DNARROW         760
#define ALT_PGDN            761
#define ALT_LEFTARROW       762
#define ALT_RIGHTARROW      763
#define ALT_HOME            764
#define ALT_UPARROW         765
#define ALT_PGUP            766

#define CTRL_TAB            777

#define CTRL_INSERT         1013
#define CTRL_DELETE         1014
#define CTRL_END            1015
#define CTRL_DNARROW        1016
#define CTRL_PGDN           1017
#define CTRL_LEFTARROW      1018
#define CTRL_RIGHTARROW     1019
#define CTRL_HOME           1020
#define CTRL_UPARROW        1021
#define CTRL_PGUP           1022

#define CURSOR              1023

#define ENTER               13
#define SPACE               32

#endif
