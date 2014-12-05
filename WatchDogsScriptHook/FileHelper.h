#pragma once
#include <iostream>
#include <vector>

using namespace std;

class FileHelper
{
public:
	static vector<string> GetAllFiles(const char *pszPath, const char* extenstion);
};