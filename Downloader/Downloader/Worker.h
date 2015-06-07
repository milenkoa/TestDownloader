#ifndef __WORKER_H
#define __WORKER_H

#include <Windows.h>
#include <vector>
#include "Item.h"

enum WorkOperation  { NoOp = 0, Start, Pause, Stop };

class WorkManager
{
private:
	HANDLE hWorkerThreads[10];
	int mMaxNumOfThreads;
	int mCurrNumOfThreads;
	volatile int mOperation; /* 0 no op, 1 start, 2 pause, 3 stop*/
	CRITICAL_SECTION csNumThreads;
	bool mDownloadVoluntaryStop;

	/**/
	std::vector<Item* > downloadList;
	std::wstring rootDlPath;
	unsigned int next;
	CRITICAL_SECTION csDownloadList;


	void setOperation(int _op)
	{
		mOperation = _op;
	}
public:
	HANDLE hPause;
	HANDLE hWaitForWork;
	WorkManager() {
		mOperation = WorkOperation::NoOp;
		mMaxNumOfThreads = 1;
		mCurrNumOfThreads = 0;
		InitializeCriticalSection(&csNumThreads);
		next = 0;
		TCHAR buffer[MAX_PATH];
		GetTempPath(MAX_PATH, buffer);
		_tcscat_s(buffer, MAX_PATH, _T("Bravo\\"));
		rootDlPath = buffer;
		InitializeCriticalSection(&csDownloadList);
	}

	~WorkManager() {
		DeleteCriticalSection(&csNumThreads);
		DeleteCriticalSection(&csDownloadList);
	}

	void add(Item * _item);
	void add(const wchar_t * _url);
	void addBulk(wchar_t * _url);
	void setMaxThreads(int _max);
	bool shouldFinish();
	bool shouldStop();
	bool shouldDownloadVoluntaryStop();
	bool start();
	bool stop();
	bool pause();

	Item* getNext();
	std::vector<Item *>::const_iterator cbegin()
	{
		return downloadList.cbegin();
	}

	std::vector<Item *>::const_iterator cend()
	{
		return downloadList.cend();
	}
	void restart();

};
#endif