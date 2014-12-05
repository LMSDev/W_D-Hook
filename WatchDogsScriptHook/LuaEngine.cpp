#include "stdafx.h"
#include "LuaEngine.h"
#include "WDOffsets.h"
#include "LuaFunctions.h"
#include "Logger.h"
#include "D3D11Hook.h"
#include <cstdlib>

// Global flag to tell scripts that call wait that we're about to relaod
bool LuaEngine::isReloadingScripts = false;
std::map<DWORD, LuaScriptThreadInfo*> LuaEngine::scriptThreads;
std::vector<int> LuaEngine::d3d11PresentCallbacks;
static CRITICAL_SECTION luaEngine_loadLock;

#define MAX_THREAD_ABORT_ATTEMPTS 5

// ----------------------- START LUA STUFF -----------------------

int lua_GetAsyncKeyState(void* L);
static int lua_GetAsyncKeyState(void* L) 
{
	int argCount = LuaFunctions::lua_gettop(L);
	bool isDown = false;
	if (argCount == 1)
	{
		if (LuaFunctions::lua_type(L, 1) == LUA_TNUMBER)
		{
			const char* str = LuaFunctions::lua_tostring(L, 1);
			int vKey = atoi(str);
			short result = GetAsyncKeyState(vKey);
			if (result == -32767)
			{
				isDown = true;
			}
		}
	}

	// Push result
	LuaFunctions::lua_pushintenger(L, isDown);

	return 1;
}

int lua_print(void* L);
static int lua_print(void* L) 
{
	int argCount = LuaFunctions::lua_gettop(L);
	char buffer[4096];
	strcpy(buffer, "");

	for (int i=1; i <= argCount; i++)
	{
		// Attempt conversion
		const char* str = LuaFunctions::lua_tostring(L, i);
		if (str != NULL)
		{
			if (i>1)
			{
				strcat(buffer, "\t");
			}
			strcat(buffer, str);
		}
		else
		{
			Logger::LogMessage("\nConversion failed for %i (lua type: %i)\n", i, LuaFunctions::lua_type(L, i));
		}
	}

	// Try to get current script name
	char scriptName[MAX_PATH];
	LuaScriptThreadInfo* threadInfo = LuaEngine::GetCurrentThreadSciptInfo();
	if (threadInfo != NULL)
	{
		strcpy(scriptName, threadInfo->fileName);
	}
	else
	{
		strcpy(scriptName, "Unknown");
	}

	Logger::LogMessage("[%s] %s\n", scriptName, buffer);
	return 0;
}

int lua_Wait(void* L);
static int lua_Wait(void* L)
{
	// Handle thread forced shutdown
	if (LuaEngine::IsReloading())
	{
		// Kill thread if unresponsive
		LuaEngine::VerifyThreadState();

		// Skip sleeping part
		LuaFunctions::lua_pushintenger(L, 1);
		return 1;
	}

	// Read values
	int argCount = LuaFunctions::lua_gettop(L);
	if (LuaFunctions::lua_type(L, 1) == LUA_TNUMBER)
	{
		// Perform sleep
		const char* str = LuaFunctions::lua_tostring(L, 1);
		int value = atoi(str);

		// Sleep in short chunks to allow exiting if required
		while (value > 10)
		{
			Sleep(10);
			value -= 10;

			// Check if we need to shutdown
			if (LuaEngine::IsReloading())
			{
				// Force thread kill
				LuaScriptThreadInfo* threadInfo = LuaEngine::GetCurrentThreadSciptInfo();
				if (threadInfo != NULL) threadInfo->abortAttempts = MAX_THREAD_ABORT_ATTEMPTS;
				LuaEngine::VerifyThreadState();
			}
		}

		// Sleep remaining time
		Sleep(value);
	}

	LuaFunctions::lua_pushintenger(L, 0);
	return 1;
}

int lua_RegisterD3D11PresentCallback(void* L);
static int lua_RegisterD3D11PresentCallback(void* L)
{
	int argCount = LuaFunctions::lua_gettop(L);
	if (argCount == 1);
	{
		int regNum = LuaFunctions::luaL_ref(L ,LUA_REGISTRYINDEX);
		Logger::LogMessage("lua_RegisterD3D11PresentCallback: %i\n", regNum);
		LuaEngine::d3d11PresentCallbacks.push_back(regNum);
	}

	return 0;
}

int lua_DrawString(void* L);
static int lua_DrawString(void* L)
{
	int argCount = LuaFunctions::lua_gettop(L);
	if (argCount == 5);
	{
		// Get arguments
		UINT32 color = LuaFunctions::lua_tonumber(L, 5);
		Logger::LogMessage("Color: %i", color);
		float y = LuaFunctions::lua_tonumber(L, 4);
		Logger::LogMessage("Y: %i", y);
		float x = LuaFunctions::lua_tonumber(L, 3);
		Logger::LogMessage("X: %i", x);
		float fontSize = LuaFunctions::lua_tonumber(L, 2);
		Logger::LogMessage("Size: %i", fontSize);
		const char* str = LuaFunctions::lua_tostring(L, 1);
		if (str == NULL) return 0;
		WCHAR buffer[256];
		mbstowcs(buffer, str, 256);
		D3D11Hook::DrawString(buffer, fontSize, x, y, color);
		Logger::LogMessage("\n", str);
	}

	return 0;
}

// ----------------------- END LUA STUFF -----------------------

void LuaEngine::Initialize()
{
	InitializeCriticalSection(&luaEngine_loadLock);
	LuaFunctions::Initialize();
}


bool LuaEngine::ExecuteScript(const char* scriptData, const char* name)
{
	void* L = LuaEngine::GetLuaState();

	// Register our own functions
	luaL_Reg* reg = new luaL_Reg();
	reg->func = lua_GetAsyncKeyState;
	reg->name = "GetAsyncKeyState";
	reg->tempStub = 0x0000;
	LuaFunctions::lua_register(L, "wdHook", reg);

	reg->func = lua_Wait;
	reg->name = "Wait";
	reg->tempStub = 0x0000;
	LuaFunctions::lua_register(L, "wdHook", reg);

	reg->func = lua_print;
	reg->name = "print";
	reg->tempStub = 0x0000;
	LuaFunctions::lua_register(L, "_G", reg);

	reg->func = lua_RegisterD3D11PresentCallback;
	reg->name = "RegisterD3D11PresentCallback";
	reg->tempStub = 0x0000;
	LuaFunctions::lua_register(L, "wdHook", reg);

	reg->func = lua_DrawString;
	reg->name = "DrawString";
	reg->tempStub = 0x0000;
	LuaFunctions::lua_register(L, "wdHook", reg);

	delete reg;

	// Execute lua script in its own thread to allow looping
	const char** ptrs = new const char*[2];
	ptrs[0] = _strdup(scriptData);
	ptrs[1] = _strdup(name);
	CreateThread(0, 0, ExecuteThreadStub, (LPVOID)ptrs, 0, 0);
	return true;
}

bool LuaEngine::TryUnload(DWORD dwTimeout)
{
	EnterCriticalSection (&luaEngine_loadLock);

	// Let all scripts know we're reloading
	LuaEngine::isReloadingScripts = true;
	LeaveCriticalSection (&luaEngine_loadLock);

	// Sleep mustn't block other threads
	Sleep(dwTimeout);

	EnterCriticalSection (&luaEngine_loadLock);
	LuaEngine::d3d11PresentCallbacks.clear();
	int threadsAlive = 0;

	// Check if threads are still active
	for (std::map<DWORD, LuaScriptThreadInfo*>::iterator it=LuaEngine::scriptThreads.begin(); it!=LuaEngine::scriptThreads.end(); ++it)
	{
		if (it->second->isAlive)
		{
			threadsAlive++;
			Logger::LogMessage("LuaEngine::TryUnload: Failed to free [%s]\n", it->second->fileName);
		}
	}

	// Clear map
	LuaEngine::scriptThreads.clear();

	LuaEngine::isReloadingScripts = false;
	LeaveCriticalSection (&luaEngine_loadLock);
	return threadsAlive == 0;
}

void LuaEngine::VerifyThreadState()
{
	// Thread safe
	EnterCriticalSection (&luaEngine_loadLock);
	LuaScriptThreadInfo* threadInfo = LuaEngine::GetCurrentThreadSciptInfo();
	if (threadInfo != NULL)
	{
		threadInfo->abortAttempts++;
		if (threadInfo->abortAttempts > MAX_THREAD_ABORT_ATTEMPTS)
		{
			// Kill thread
			Logger::LogMessage("Thread [%s] is unresponsive, forcing shutdown. Possible memory leak\n", threadInfo->fileName);
			threadInfo->isAlive = false;

			// Allow loading thread to continue
			LeaveCriticalSection (&luaEngine_loadLock);

			// Kill
			ExitThread(0);
		}
	}
	else
	{
		Logger::LogMessage("lua_Wait: Failed to retrieve thread information!\n");
	}

	// Thread safe
	LeaveCriticalSection (&luaEngine_loadLock);
}

LuaScriptThreadInfo* LuaEngine::GetCurrentThreadSciptInfo()
{
	DWORD dwThreadID = GetCurrentThreadId();
	for (std::map<DWORD, LuaScriptThreadInfo*>::iterator it=LuaEngine::scriptThreads.begin(); it!=LuaEngine::scriptThreads.end(); ++it)
	{
		if (it->first == dwThreadID)
		{
			LuaScriptThreadInfo* threadInfo = LuaEngine::scriptThreads.at(dwThreadID);
			if (threadInfo != NULL)
			{
				return threadInfo;
			}
			else
			{
				Logger::LogMessage("%x has no assigned ThreadInfo object!\n", dwThreadID);
				return NULL;
			}
		}
	}

	Logger::LogMessage("%x not found in threads list!\n", dwThreadID);

	return NULL;
}

void* LuaEngine::GetLuaState()
{
	//Logger::LogVerbose("GetLuaState: Base: 0x%I64X\n", WDOffsets::GetGameBase());

	DWORD64* dwMain = (DWORD64*)((DWORD64)WDOffsets::GetGameBase() + 0x392AE38);
	 if(*dwMain == NULL)	return NULL;

	//Logger::LogVerbose("GetLuaState: Main: 0x%I64X\n", *dwMain);

	DWORD64* dwLua = (DWORD64*)(*dwMain + 8);
    if(*dwLua == NULL) return NULL;

	//Logger::LogMessage("GetLuaState: LuaState: 0x%I64X\n", *dwLua);
	return (void*)*dwLua;
}

const char* LuaEngine::ParseErrorString(void* L, int result)
{
	switch (result)
	{
		case LUA_OK:
			return "OK";
		case LUA_YIELD:
			return "YIELD";
		case LUA_ERRRUN:
			return "Run error";
		case LUA_ERRSYNTAX:
			return "Syntax error";
		case LUA_ERRMEM:
			return "Memory error";
	}

    return "Other";
}

DWORD WINAPI LuaEngine::ExecuteThreadStub(LPVOID lpParam)
{
	// Thread safe
	EnterCriticalSection (&luaEngine_loadLock);
	const char** ptrs  = reinterpret_cast<const char**>(lpParam);

	// Set up thread information
	LuaScriptThreadInfo* luaScriptThreadInfo = new LuaScriptThreadInfo();
	luaScriptThreadInfo->abortAttempts = 0;
	luaScriptThreadInfo->fileName = _strdup(ptrs[1]);
	luaScriptThreadInfo->isAlive = true;
	DWORD dwThreadId = GetCurrentThreadId();
	LuaEngine::scriptThreads.insert(std::pair<DWORD, LuaScriptThreadInfo*>(dwThreadId, luaScriptThreadInfo));
	Logger::LogMessage("Added %x thread\n", dwThreadId);

	// Execute call
	void* L = LuaEngine::GetLuaState();
	Logger::LogMessage("Loading %s into buffer\n", ptrs[1]);
	int result = LuaEngine::Execute(L, ptrs[0]);
	const char* stringResult = ParseErrorString(L, result);
	Logger::LogMessage("Call result: %s\n", stringResult);

	// Get thread
	LuaScriptThreadInfo* threadInfo = GetCurrentThreadSciptInfo();
	if (threadInfo == NULL)
	{
		Logger::LogMessage("ExecuteThreadStub: Failed to retrieve thread information!\n");

		// Free memory
		delete[] ptrs[0];
		delete[] ptrs[1];
		delete[] ptrs;
		LeaveCriticalSection (&luaEngine_loadLock);

		return 1;
	}

	// Thread-safe removal of thread from global list
	threadInfo->isAlive = false;
	LuaEngine::scriptThreads.erase(dwThreadId);

	// Free memory
	delete[] ptrs[0];
	delete[] ptrs[1];
	delete[] ptrs;

	LeaveCriticalSection (&luaEngine_loadLock);

	return 1;
}

int LuaEngine::Execute(void* L, const char* scriptData)
{
    int loadResult = LuaFunctions::pluaL_loadbuffer(L, scriptData, strlen(scriptData), "exec");
	if (loadResult != LUA_OK)
	{
		const char* err = LuaFunctions::lua_tostring(L, -1);
		Logger::LogMessage("Compilation error: %s\n", err);
		return LUA_ERRSYNTAX;
	}

	// Leave critical section because call below might never return (loop)
	LeaveCriticalSection (&luaEngine_loadLock);

    int result = LuaFunctions::plua_pcall(L, 0, -1, 0);
	EnterCriticalSection (&luaEngine_loadLock);
	if (result != LUA_OK)
	{
		const char* err = LuaFunctions::lua_tostring(L, -1);
		Logger::LogMessage("Execution error: %s\n", err);
	}

	return result;
}