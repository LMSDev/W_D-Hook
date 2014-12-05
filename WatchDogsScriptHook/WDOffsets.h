#pragma once
class WDOffsets
{
public:
	static void Initialize(DWORD64 gameBase);
	static DWORD64 GetGameBase() { return dwGameBase; }

private:
	static DWORD64 dwGameBase;
};
