#ifndef __FLORIST_CARDS_H
#define __FLORIST_CARDS_H


#define		FLOR_CARD_TEXT_FILE							"BINARYDATA\\FlowerCard.edt"
#define		FLOR_CARD_TEXT_TITLE_SIZE				5 * 80 * 2



void GameInitFloristCards();
BOOLEAN EnterFloristCards();
void ExitFloristCards();
void HandleFloristCards();
void RenderFloristCards();


extern INT8			gbCurrentlySelectedCard;

#endif
