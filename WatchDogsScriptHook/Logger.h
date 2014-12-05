#pragma once
#include "stdafx.h"

class Logger
{
public:
	static void Initialize(char* fileName);
	static void LogMessage(char* message, ...);
	static void Shutdown();
};