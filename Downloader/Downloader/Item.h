#ifndef __ITEM_H
#define __ITEM_H

#include <string>
#include <vector>
#include <Windows.h>
//#include "Worker.h"
using namespace std;

//DWORD WINAPI worker_procedure(LPVOID lpParam);

//enum WorkOperation  { NoOp = 0, Start, Pause, Stop };
enum ItemStatus { Waiting = 1, Downloading = 2, OK, Failed, Cancelled};

class Item 
{
private:
	int mId;
	wstring mUrl;
	wstring mAbsFilePath;
	wstring mFileName;
	wstring mDownloadFolder;
	long mSize;
	int mStatus;

	wstring getStatusString();
public:
	Item(wstring& _url, wstring& downloadFolder, int _id);
	wstring toString();
	wstring getUrl()
	{
		return mUrl;
	}
	wstring getFilePath()
	{
		return mAbsFilePath;
	}
	void setStatus(int _status);

	int getStatus() 
	{
		return mStatus;
	}

	void checkDownloadLocation();

	bool download();
	bool voluntaryStop();
	bool cleanFile();
};

class WorkQueue 
{
private:
	std::vector<Item* > downloadList;
	std::wstring rootDlPath;
	unsigned int next;
	CRITICAL_SECTION cs;

public:
	WorkQueue() : next(0) 
	{
		TCHAR buffer[MAX_PATH];
		GetTempPath(MAX_PATH, buffer);
		_tcscat_s(buffer, MAX_PATH, _T("Bravo\\"));
		rootDlPath = buffer;
		InitializeCriticalSection(&cs);
	}

	~WorkQueue() 
	{
		DeleteCriticalSection(&cs);
	}

	void add(Item * _item);
	void add(const wchar_t * _url);
	void addBulk(wchar_t * _url);
	void restart();
	//bool hasNext();
	Item* getNext();
	std::vector<Item *>::const_iterator cbegin() 
	{
		return downloadList.cbegin();
	}

	std::vector<Item *>::const_iterator cend() 
	{
		return downloadList.cend();
	}

	void cleanup();

};

#endif