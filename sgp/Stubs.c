#include <stdio.h>
#include "Stubs.h"


void OutputDebugString(const char* str)
{
	fputs(str, stderr);
}
