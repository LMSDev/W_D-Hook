#include "stdafx.h"
#include "WDOffsets.h"

DWORD64 WDOffsets::dwGameBase = 0;

void WDOffsets::Initialize(DWORD64 gameBase)
{
	dwGameBase = gameBase;
}