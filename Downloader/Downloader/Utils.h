
#include "stdafx.h"
#include <Windows.h>
#include <WinInet.h>
#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <urlmon.h>
#include <string>
#include <ostream>

const int BUF_SIZE = 10240;

class Utils {
public:
	static bool getFilenameFromUrl(const wchar_t * url, wchar_t* buffer, size_t buffSize);
	static bool getInfo(const wchar_t * szUrl, long * lSize, std::wstring * filename);
	static bool validateUrl(const wchar_t * szUrl);
	static bool loadFromUrl(const wchar_t * szUrl, std::wstringstream* buffer);
};