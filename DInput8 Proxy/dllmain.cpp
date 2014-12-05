#include "dinput.h"
     
HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
    char realLib[MAX_PATH] = { 0 };
    GetSystemDirectoryA(realLib, sizeof(realLib));
    strcat_s(realLib, MAX_PATH, "\\dinput8.dll"); 
    HMODULE hLibrary = LoadLibraryA(realLib);
     
    if(hLibrary)
    {
		FARPROC originalProc = GetProcAddress(hLibrary, "DirectInput8Create");
     
		if(originalProc)
		{
			return ((HRESULT(WINAPI*)(HINSTANCE,DWORD,REFIID,LPVOID *,LPUNKNOWN))originalProc)(hinst, dwVersion, riidltf, ppvOut, punkOuter);
		}
    }

	return E_FAIL;
}
    
          
     
DWORD WINAPI lpDllLoadingThread(LPVOID lpParam)
{     
	// Load hook
	LoadLibraryA("WatchDogsScriptHook.dll");
	return 0;
}
     
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch(ul_reason_for_call)
    {
		case DLL_PROCESS_ATTACH:
            {
				CreateThread(0, 0, lpDllLoadingThread, 0, 0, 0);
				break;
            }
    }
	
	return TRUE;
}