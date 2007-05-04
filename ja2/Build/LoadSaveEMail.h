#ifndef LOADSAVEEMAIL_H
#define LOADSAVEEMAIL_H

#include "EMail.h"
#include "FileMan.h"


BOOLEAN LoadEmailFromSavedGame(HWFILE File);
BOOLEAN SaveEmailToSavedGame(HWFILE File);

#endif
