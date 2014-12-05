#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>

using namespace std;

struct LuaScriptThreadInfo
{
	char* fileName;
	bool isAlive;
	DWORD abortAttempts;
};

class LuaEngine
{
public:
	static void Initialize();
	static bool ExecuteScript(const char* scriptData, const char* name);
	static bool TryUnload(DWORD dwTimeout);
	static void VerifyThreadState();
	static bool IsReloading() { return LuaEngine::isReloadingScripts; }
	static LuaScriptThreadInfo* GetCurrentThreadSciptInfo();
	static void* GetLuaState();
	static std::map<DWORD, LuaScriptThreadInfo*> scriptThreads;
	static std::vector<int> d3d11PresentCallbacks;

private:
	static const char* ParseErrorString(void* L, int r);
	static DWORD WINAPI ExecuteThreadStub(LPVOID lpParam);
	static int Execute(void* L, const char* scriptRaw);
	static bool isReloadingScripts;
};