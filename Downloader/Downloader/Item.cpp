#include "stdafx.h"
#include "Utils.h"
#include "Item.h"
#include "Worker.h"
#include <sstream>
#include <Windows.h>
#define WM_UPDATE_OUTPUT (WM_APP + 0x0001)

extern HWND hWin;
extern WorkManager wManager;


Item::Item(wstring& _url, wstring& downloadFolder, int _id) 
{
	mUrl = wstring(_url);
	mDownloadFolder = downloadFolder;
	mId = _id;
	long size;
	wstring filename;
	if (Utils::getInfo(mUrl.c_str(), &size, &filename))
	{
		mSize = size;
		if (filename.size() == 0)
		{
			TCHAR buffer[MAX_PATH];
			Utils::getFilenameFromUrl(mUrl.c_str(), buffer, MAX_PATH);
			wstring fileName = buffer;
			mFileName = fileName;

		}
		else
		{
			mFileName = filename;
		}
	}
	else
	{
		throw exception("Failed to retrieve file size");
	}
	mStatus = ItemStatus::Waiting;
	mAbsFilePath = downloadFolder + mFileName;
}

wstring Item::toString()
{
	if (mSize < 0)
	{
		return std::to_wstring(mId) + wstring(L". ") + mUrl + wstring(L" unknown ") + getStatusString();
	}
	else if (mSize < 1024)
	{
		return std::to_wstring(mId) + wstring(L". ") + mUrl + wstring(L" ") + std::to_wstring(mSize) + wstring(L"B ") + getStatusString();
	}
	else
	{
		long sizeKb = mSize / 1025;
		return std::to_wstring(mId) + wstring(L". ") + mUrl + wstring(L" ") + std::to_wstring(sizeKb) + wstring(L"KB ") + getStatusString();
	}
	
}

void Item::setStatus(int _status) 
{
	mStatus = _status;
	//update screen
	PostMessage(hWin, WM_UPDATE_OUTPUT, NULL, NULL);
}

wstring Item::getStatusString()
{
	switch(mStatus)
	{
	case ItemStatus::Waiting:
		return wstring(L"Waiting");
		break;
	case ItemStatus::Downloading:
		return wstring(L"Downloading");
		break;
	case ItemStatus::OK:
		return wstring(L"OK");
		break;
	case ItemStatus::Failed:
		return wstring(L"Failed");
		break;
	default:
		return wstring(L"");
		break;
	}
}

void Item::checkDownloadLocation() {
	wstring absFilePath = mDownloadFolder + mFileName;
}

bool Item::download() 
{
	HINTERNET hIurl, hInet;
	//wchar_t header[80];
	unsigned long contentLen;
	unsigned long len;
	std::ofstream fout;
	unsigned char buf[BUF_SIZE];// input buffer
	unsigned long numrcved;
	unsigned long total = 0;

	if (InternetAttemptConnect(0) != ERROR_SUCCESS)
	{
		return false;
	}
	fout.open(mAbsFilePath, std::ios::binary | std::ios::out | std::ios::trunc);
	if (fout.fail())
	{
		return false;
	}


	hInet = InternetOpen(L"SimpleDownloader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hInet == NULL)
	{
		return false;
	}

	hIurl = InternetOpenUrl(hInet, mUrl.c_str(), NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE, 0);
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
			
		// Write buffer to disk
		fout.write((const char *)buf, numrcved);
		if (!fout.good())
		{
			InternetCloseHandle(hIurl);
			InternetCloseHandle(hInet);
			return false;
		}

		if (voluntaryStop())
		{
			setStatus(ItemStatus::Waiting);
			fout.close();
			cleanFile();
			goto cleanup;
		}
		// update running total
		total += numrcved;
	} while (numrcved > 0);

	setStatus(ItemStatus::OK);
cleanup:
	InternetCloseHandle(hIurl);
	InternetCloseHandle(hInet);
	fout.close();
	return true;
}

bool Item::voluntaryStop() 
{
	return wManager.shouldDownloadVoluntaryStop();
}

bool Item::cleanFile() 
{
	return (TRUE == DeleteFile(mAbsFilePath.c_str()));

}

