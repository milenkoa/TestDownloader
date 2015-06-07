#include "stdafx.h"
#include "Item.h"
#include "Worker.h"
#include <sstream>
#include <string>
#include "Utils.h"
#define WM_UPDATE_OUTPUT (WM_APP + 0x0001)
#define WM_ERROR_OUTPUT (WM_APP + 0x0002)
extern HWND hWin;

extern WorkManager wManager;

/*
 * worker_procedure executed by worker threads
 *
 */

__pragma(warning(push))
__pragma(warning(disable:4127))
__pragma(warning(disable:4100))
DWORD WINAPI worker_procedure(LPVOID lpParam) 
{
	while (1)
	{
		Item * t = NULL;
		t = wManager.getNext();
		if (t != NULL)
		{
			if (!t->download())
			{
				t->setStatus(ItemStatus::Failed);
			}
			if (wManager.shouldDownloadVoluntaryStop()) {
				break;
			}
			if (wManager.shouldFinish()) {
				break;

			}
			WaitForSingleObject(wManager.hPause, INFINITE);
		}
		else
		{
			WaitForSingleObject(wManager.hWaitForWork, INFINITE);
			//when awake, check if max parallel downloads changed
			if (wManager.shouldFinish()) {
				break;
			}

		}
	}
	return 0;

}
__pragma(warning(pop))


/*
* WorkerManager class: Member methods
*
*/
bool WorkManager::start()
{
	switch (mOperation)
	{
	case (WorkOperation::Start) :
		break;
	case (WorkOperation::Stop) :
	{
		mOperation = WorkOperation::Start;
		restart();
		// now create threads again
	}
	case (WorkOperation::NoOp) :
	{
		mOperation = WorkOperation::Start;

		hPause = CreateEvent(NULL, TRUE, TRUE, L"ThreadPause");
		if (hPause == NULL)
		{
			return false;
		}

		hWaitForWork = CreateEvent(NULL, TRUE, TRUE, L"ThreadWaitForWork");
		if (hWaitForWork == NULL)
		{
			return false;
		}

		for (int i = 0; i < mMaxNumOfThreads; i++)
		{
			hWorkerThreads[i] = CreateThread(NULL, 0, worker_procedure, NULL, 0, NULL);
			mCurrNumOfThreads++;
		}
		break;
	}

	case (WorkOperation::Pause) :
	{

		SetEvent(hPause);
		mOperation = WorkOperation::Start;
		break;
	}
	}
	return true;

}

bool WorkManager::pause()
{
	mOperation = WorkOperation::Pause;
	ResetEvent(hPause);
	return true;
}

bool WorkManager::stop()
{
	mOperation = WorkOperation::Stop;
	return true;
}

void WorkManager::setMaxThreads(int _max)
{
	if (mOperation == WorkOperation::NoOp)
	{
		mMaxNumOfThreads = _max;
	}
	else
	{
		//int currentMax = mMaxNumOfThreads;
		if (_max < mMaxNumOfThreads)
		{
			mMaxNumOfThreads = _max;
		}
		else
		{
			//create additional threads and set new value for mMaxNumOfThreads
			for (int i = mMaxNumOfThreads; i < _max; i++)
			{
				hWorkerThreads[i] = CreateThread(NULL, 0, worker_procedure, NULL, 0, NULL);
				mCurrNumOfThreads++;
			}
			mMaxNumOfThreads = _max;
		}
	}
}
/* used to terminate worker threads when max parallel downloads decreases */
bool WorkManager::shouldFinish()
{
	bool bFinish = false;
	EnterCriticalSection(&csNumThreads);
	if (mCurrNumOfThreads > mMaxNumOfThreads)
	{
		bFinish = true;
		mCurrNumOfThreads--;
	}
	LeaveCriticalSection(&csNumThreads);
	return bFinish;
}

bool WorkManager::shouldStop()
{
	if (mOperation != WorkOperation::Start)
	{
		return true;
	}
	if (mCurrNumOfThreads > mMaxNumOfThreads)
	{
		return true;
	}
	return false;
}

bool WorkManager::shouldDownloadVoluntaryStop()
{
	if (mOperation == WorkOperation::Stop)
	{
		return true;
	}
	else
	{
		return false;
	}

}

void  WorkManager::add(Item * _item)
{
	EnterCriticalSection(&csDownloadList);
	downloadList.push_back(_item);
	LeaveCriticalSection(&csDownloadList);
}

void WorkManager::add(const wchar_t * _url)
{
	wstring sUrl = wstring(_url);
	EnterCriticalSection(&csDownloadList);
	unsigned int id = downloadList.size();

	id++;
	Item * newItem;
	try
	{
		newItem = new Item(sUrl, rootDlPath, id);
	}
	catch (exception e)
	{
		LeaveCriticalSection(&csDownloadList);
		wstring * message = new wstring(L"Failed to add ");
		*message += wstring(_url);
		PostMessage(hWin, WM_ERROR_OUTPUT, NULL, (LPARAM)message);
		return;
	}

	downloadList.push_back(newItem);
	LeaveCriticalSection(&csDownloadList);
}

void WorkManager::addBulk(wchar_t * _url)
{
	std::wstringstream ss;
	std::wstring s;
	Utils::loadFromUrl(_url, &ss);
	while (std::getline(ss, s))
	{
		if (s[s.size() - 1] == '\r')
			s.resize(s.size() - 1);
		if (Utils::validateUrl(s.c_str()))
			add(s.c_str());
		else
		{
			LeaveCriticalSection(&csDownloadList);
			wstring * message = new wstring(L"Provided list is not valid");
			PostMessage(hWin, WM_ERROR_OUTPUT, NULL, (LPARAM)message);
			return;
		}
	}


}

Item * WorkManager::getNext()
{
	EnterCriticalSection(&csDownloadList);
	if (next >= downloadList.size()) {
		LeaveCriticalSection(&csDownloadList);
		return NULL;
	}
	Item * nextItem = downloadList.at(next);
	next++;
	LeaveCriticalSection(&csDownloadList);
	//TOOD: remove this to worker procedure
	nextItem->setStatus(ItemStatus::Downloading);
	return nextItem;
}


void WorkManager::restart()
{
	std::vector<Item *>::const_iterator it;
	int c = 0;
	for (it = downloadList.cbegin(); it != downloadList.cend() && ((*it)->getStatus() != ItemStatus::Waiting); c++, it++);
	next = c;
}

