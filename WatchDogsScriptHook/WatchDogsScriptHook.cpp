// WatchDogsScriptHook.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"
#include "WatchDogsScriptHook.h"
#include "Logger.h"
#include "WDOffsets.h"
#include "LuaEngine.h"
#include "FileHelper.h"
#include "D3D11Hook.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

DWORD WINAPI lpThreadStub(LPVOID lpParam);
bool hasConsole = false;

#define THREAD_FREE_TIMEOUT 1000

void WatchDogsScriptHook::Initialize()
{
	Logger::Initialize("WatchDogsScriptHook.log");
	Logger::LogMessage("WatchDogsScriptHook 0.14 by LMS <lms@lms-dev.com>\n");
	Logger::LogMessage("Thanks to Eric Rufelt for FW1FontWrapper\n");

	// Initialize offsets
	HMODULE hModule = GetModuleHandleA("Disrupt_b64.dll");
	WDOffsets::Initialize((DWORD64)hModule);
    Logger::LogMessage("Process base: 0x%I64X\n", (DWORD64)hModule);

	// Init lua engine
	LuaEngine::Initialize();

	// Spawn thread
	CreateThread(0, 0, lpThreadStub, this, 0, 0);

	Logger::LogMessage("Initialized\n");
}

void WatchDogsScriptHook::Unload()
{
	Logger::Shutdown();
	if (hasConsole)
	{
		fclose(stdout);
		hasConsole = !FreeConsole();
	}
}

DWORD WINAPI lpThreadStub(LPVOID lpParam)
{

	// Allow debugging
	DWORD64 dwAddr = (DWORD64)GetProcAddress(GetModuleHandleA("KERNELBASE.dll"), "IsDebuggerPresent");
	DWORD dwVirtualProtectBackup;
	bool result = VirtualProtect((BYTE*)dwAddr, 0x20, PAGE_READWRITE, &dwVirtualProtectBackup);
	if (result == NULL)
	{
		Logger::LogMessage("Failed to patch debugging.\n");
		return 0;
	}
	else
	{
		*(BYTE*)(dwAddr) = 0xB8;
		memset((BYTE*)(dwAddr + 0x1), 0x0, 0x4);
		*(BYTE*)(dwAddr + 0x5) = 0xC3;
		VirtualProtect((BYTE*)dwAddr, 0x20, dwVirtualProtectBackup, &dwVirtualProtectBackup);
	}

	// Wait for D3D to initialize
	while (FindWindowA(NULL, "Watch_Dogs") == NULL)
	{
		Sleep(100);
	}

	// Give D3D some time
	Sleep(5000);
	Logger::LogMessage("Injecting D3D\n");
	D3D11Hook::Initialize();

	WatchDogsScriptHook* inst = (WatchDogsScriptHook*)lpParam;
	inst->Tick();
	return 0;
}

void WatchDogsScriptHook::Tick()
{
	while (true)
	{
		if (GetAsyncKeyState(VK_F10))
		{
			Beep(1500, 500);
			WatchDogsScriptHook::LoadScripts();
			Sleep(1000);
		}

		if (GetAsyncKeyState(VK_F11))
		{
			if (hasConsole)
			{
				fclose(stdout);
				hasConsole = !FreeConsole();
			}
			else
			{
				hasConsole = AllocConsole();
				if (hasConsole)
				{
					freopen( "CONOUT$", "wb", stdout);
					Logger::LogMessage("Console attached\n");
				}
				else
				{
					Logger::LogMessage("Failed to attach console!\n");
				}
			}

			Sleep(1000);
		}

		Sleep(50);
	}
}

// (Re)-load scripts from scripts folder
void WatchDogsScriptHook::LoadScripts()
{
	// Give scripts time to finish
	Logger::LogMessage("WatchDogsScriptHook::LoadScripts(): Freeing current threads (%i ms)\n", THREAD_FREE_TIMEOUT);
	bool result = LuaEngine::TryUnload(THREAD_FREE_TIMEOUT);
	Logger::LogMessage("WatchDogsScriptHook::LoadScripts(): Unload state: %i\n", result);

	char path[MAX_PATH];
	if (GetCurrentDirectoryA(MAX_PATH, path) != 0)
	{
		char pszPluginPath[MAX_PATH] = { 0 };
        strcpy_s(pszPluginPath, MAX_PATH, path);
        strcat_s(pszPluginPath, MAX_PATH, "\\scripts\\");

		Logger::LogMessage("WatchDogsScriptHook::LoadScripts(): %s\n", &pszPluginPath);

		vector<string> scripts = FileHelper::GetAllFiles(pszPluginPath, "*.lua");
		for (int i = 0; i < scripts.size(); i++)
		{
			Logger::LogMessage("Loading %s\n", scripts[i].c_str());

			// Read file
			std::ifstream t;
			int length;
			t.open(scripts[i].c_str()); 
			t.seekg(0, std::ios::end); 
			length = t.tellg();
			t.seekg(0, std::ios::beg);
			char* buffer = new char[length];
			memset(buffer, 0x0, length);	// Make sure there is no shit at the end
			t.read(buffer, length);
			t.close();

			char fileName[MAX_PATH];
			char ext[MAX_PATH];
			_splitpath(scripts[i].c_str(), NULL, NULL, fileName, ext);
			strcat(fileName, ext);
			LuaEngine::ExecuteScript(buffer, fileName);
			delete[] buffer;
		}
	}
}