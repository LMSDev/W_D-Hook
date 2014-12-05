#pragma once
#include "stdafx.h"

class WatchDogsScriptHook
{
public:
	void Initialize();
	void Unload();
	void Tick();

private:
	void LoadScripts();
};