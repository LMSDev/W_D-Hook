// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
#include "stdafx.h"
#include "WatchDogsScriptHook.h"

static WatchDogsScriptHook* g_pScriptHook = NULL;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hModule); 

			g_pScriptHook = new WatchDogsScriptHook();
			if (g_pScriptHook)
			{
				g_pScriptHook->Initialize();
			}
			break;
		case DLL_PROCESS_DETACH:
			g_pScriptHook->Unload();
			delete g_pScriptHook;
			break;
	}

	return TRUE;
}