

#include "stdafx.h"
#include "Utils.h"
#include <sstream>
#include <string>
#include <wchar.h>

bool Utils::validateUrl(const wchar_t * url) {
	bool bOk = true;
	HRESULT hResult = IsValidURL(NULL, url, 0);
	if (hResult != S_OK) {
		bOk = false;
	}

	return bOk;
}

bool Utils::getFilenameFromUrl(const wchar_t * url, wchar_t* buffer, size_t buffSize) {
	const wchar_t * search = wcsrchr(url, L'/');

	if (search)
	{
		search++;
		if (wcscpy_s(buffer, buffSize, search) == 0)
		{
			return true;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return false;
	}

}

bool Utils::loadFromUrl(const wchar_t * szUrl,std::wstringstream* buffer) {
	HINTERNET hIurl, hInet;
	unsigned long contentLen;
	unsigned long len;
	char buf[BUF_SIZE];
	unsigned long numrcved;
	unsigned long total = 0;

	if (InternetAttemptConnect(0) != ERROR_SUCCESS)
	{
		return false;
	}

	hInet = InternetOpen(L"SimpleDownloader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hInet == NULL)
	{
		return false;
	}

	hIurl = InternetOpenUrl(hInet, szUrl, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE, 0);
	if (hIurl == NULL)
	{
		InternetCloseHandle(hInet);
		return false;
	}

	if (!HttpQueryInfo(hIurl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLen, &len, NULL))
	{
		InternetCloseHandle(hIurl);
		InternetCloseHandle(hInet);
		return false;
	}

	do
	{
		// Read a buffer of info
		if (!InternetReadFile(hIurl, &buf, BUF_SIZE, &numrcved))
		{
			InternetCloseHandle(hIurl);
			InternetCloseHandle(hInet);
			return false;
		}

		if (numrcved > 0) {
			wchar_t * temp = new wchar_t[numrcved];
			size_t convertedChars = 0;
			mbstowcs_s(&convertedChars, temp, numrcved+1, buf, _TRUNCATE);

			(*buffer) << temp;
		}
		total += numrcved;
		// Call update function, if specified
	} while (numrcved > 0);

	InternetCloseHandle(hIurl);
	InternetCloseHandle(hInet);
	return true;
}


bool Utils::getInfo(const wchar_t * szUrl, long * lSize, std::wstring * filename) {
	HINTERNET hIurl, hInet;
	wchar_t header[5000];
	unsigned long contentLen;
	unsigned long len;

	if (InternetAttemptConnect(0) != ERROR_SUCCESS)
	{
		return false;
	}


	hInet = InternetOpen(L"SimpleDownloader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hInet == NULL)
	{
		return false;
	}

	hIurl = InternetOpenUrl(hInet, szUrl, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE, 0);
	if (hIurl == NULL)
	{
		InternetCloseHandle(hInet);
		return false;
	}

	if (!HttpQueryInfo(hIurl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLen, &len, NULL))
	{
		*lSize = -1;
	}
	else
	{
		*lSize = contentLen;
	}

	len = 5000;

	if (HttpQueryInfo(hIurl, HTTP_QUERY_CONTENT_DISPOSITION, header, &len, NULL))
	{
		wchar_t * ptr = wcsstr(header, L"filename=");
		*filename += std::wstring(ptr + 9);
	}
	
	InternetCloseHandle(hIurl);
	InternetCloseHandle(hInet);
	return true;
}
