#pragma once

#include "resource.h"
#include <stdio.h>
#include "Utils.h"

void worker();

DWORD WINAPI worker_procedure(LPVOID lpParam);


/*class Item {
private:
	wchar_t* _link;
	wchar_t* _hostname;
	wchar_t* _path;
	long _size;
	int _status;

public:
	Item(wchar_t* link, wchar_t* hostname, wchar_t* path, long size, int status) 
		: _link(link), _hostname(hostname), _path(path), _status(status) { }

	
	~Item() 
	{
		free(_link); free(_hostname); free(_path);
	}

	wchar_t * toString() 
	{
		wchar_t * output = (wchar_t*) malloc(sizeof(wchar_t)*100);
		wsprintf(output, L"%s %d %s", _link, _size, L"Waiting");
		return output;
	}
};*/
