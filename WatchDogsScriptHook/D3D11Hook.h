#pragma once
class D3D11Hook
{
public:
	static void Initialize();
	static void DrawString(const WCHAR* pszString, float fontSize, float x, float y, UINT32 color);
};