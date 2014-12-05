#include "stdafx.h"
#include "FileHelper.h"


vector<string> FileHelper::GetAllFiles(const char *path, const char* fileExt)
{
        vector<string> dllFiles;
 
		// Set up path.
        char pluginPath[MAX_PATH] = {0};
		char tempDll[MAX_PATH] = {0};
        strcpy_s(pluginPath, MAX_PATH, path);
        strcat_s(pluginPath, MAX_PATH, fileExt);
 
        WIN32_FIND_DATAA findData;
 
		// Get first file in directory.
        HANDLE firstFile = FindFirstFileA(pluginPath, &findData);
        if(firstFile == INVALID_HANDLE_VALUE)
        {
			return dllFiles;
        }
 
		// Store.
        strcat_s(tempDll, MAX_PATH, path);
        strcat_s(tempDll, MAX_PATH, findData.cFileName);
        dllFiles.push_back(tempDll);

		// Find all other files.
        while(FindNextFileA(firstFile, &findData) != 0)
        {
			// Ensure there is no crap left over from last iteration.
			memset(tempDll, 0x0, MAX_PATH);
 
			// Copy and save.
			strcat_s(tempDll, MAX_PATH, path);
			strcat_s(tempDll, MAX_PATH, findData.cFileName);
			dllFiles.push_back(tempDll);
        }
 
        return dllFiles;
}