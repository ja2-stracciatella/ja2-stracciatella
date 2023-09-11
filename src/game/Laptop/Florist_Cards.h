#ifndef __FLORIST_CARDS_H
#define __FLORIST_CARDS_H

#include "Types.h"

#define FLOR_CARD_TEXT_FILE       BINARYDATADIR "/flowercard.edt"
#define FLOR_CARD_TEXT_TITLE_SIZE 5 * 80


void EnterFloristCards(void);
void ExitFloristCards(void);
void RenderFloristCards(void);


extern INT8			gbCurrentlySelectedCard;

#endif
