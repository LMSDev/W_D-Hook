#include "stdafx.h"
#include "Logger.h"

static CRITICAL_SECTION lock;
static FILE * logfile;

void Logger::Initialize(char* fileName)
{
	InitializeCriticalSection(&lock);
	logfile = fopen(fileName, "at");
}

void Logger::LogMessage(char* message, ...)
{
	if (!logfile)
	{
		MessageBoxA(0, "Logger::LogMessage: File pointer not initialized", "Error", 0);
		return;
	}

	// Thread safe
	EnterCriticalSection(&lock);

	// Log time stamp
	SYSTEMTIME time;
	GetLocalTime(&time);	
	fprintf(logfile, "[%02d:%02d:%02d.%03d] ", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
	printf("[%02d:%02d:%02d.%03d] ", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

	// Process arguments
	va_list	args;	
	va_start(args, message);
	vfprintf(logfile, message, args);
	vprintf(message, args);
	fflush(logfile);
	va_end(args);

	// Leave again
	LeaveCriticalSection(&lock);
}

void Logger::Shutdown()
{
	if (logfile)
	{
		EnterCriticalSection (&lock);
		fflush(logfile);
		fclose(logfile);       
		logfile = NULL;
		LeaveCriticalSection(&lock);
		DeleteCriticalSection(&lock);
	}
}