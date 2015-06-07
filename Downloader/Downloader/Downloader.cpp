// Downloader.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Downloader.h"
#include "hwndec.h"
#include "Worker.h"
#include "LimitSingleInstance.h"
#include <stdio.h>
#include <string>
#include <sstream>


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hWin;

#define START_X	40
#define START_Y	40
#define SPACING 20
#define HEIGHT	20


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void AppendText( const HWND &hwnd, TCHAR *newText );
void updateOutput();

/* Local variables */
//WorkQueue downloadQueue;
WorkManager wManager;

int outputType = 0;

// {F9CAE869-C3E3-46c7-9867-E6BB1FEF533B}
CLimitSingleInstance g_SingleInstanceObj(TEXT("Global\\{F9CAE869-C3E3-46c7-9867-E6BB1FEF533B}"));



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	// Should inspect lpCmdLine is it is empty, if not, add it into job queue
	// UNREFERENCED_PARAMETER(lpCmdLine);

	LoadString(hInstance, IDC_DOWNLOADER, szWindowClass, MAX_LOADSTRING);

	// limit single instance
	if (g_SingleInstanceObj.IsAnotherInstanceRunning())
	{
		//parse command line paramter
		if (lpCmdLine != NULL) {
			TCHAR * ptr = _tcsstr(lpCmdLine, _T("-startdownload="));
			if (ptr != NULL)
			{
				HWND hInstanceWindow = FindWindow(szWindowClass, NULL);
				if (hInstanceWindow != NULL) {
					COPYDATASTRUCT cpd;
					cpd.dwData = 0;
					cpd.cbData = (_tcslen(lpCmdLine) + 1) * sizeof(TCHAR);
					cpd.lpData = (void *)(lpCmdLine + 15);

					SendMessage(hInstanceWindow, WM_COPYDATA, (WPARAM)hInstanceWindow, (LPARAM)(LPVOID)&cpd);
				}
			}
		}
		return 1;
	}
		
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	TCHAR buffer[MAX_PATH];
	GetTempPath(MAX_PATH, buffer);
	_tcscat_s(buffer,MAX_PATH, _T("Bravo"));
	CreateDirectory(buffer, NULL);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_DOWNLOADER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DOWNLOADER));

	if (lpCmdLine != NULL) {
		TCHAR * ptr = _tcsstr(lpCmdLine, _T("-startdownload="));
		if (ptr != NULL)
		{
				COPYDATASTRUCT cpd;
				cpd.dwData = 0;
				cpd.cbData = (_tcslen(lpCmdLine) + 1) * sizeof(TCHAR);
				cpd.lpData = (void *)(lpCmdLine + 15);

				SendMessage(hWin, WM_COPYDATA, NULL, (LPARAM)(LPVOID)&cpd);
				SendMessage(hWin, WM_AUTO_START, NULL, NULL);
		}
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DOWNLOADER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	//wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.hbrBackground	= CreateSolidBrush(RGB(240, 240, 240));
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DOWNLOADER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   hWin = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		hInput = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER, START_X, START_Y, 500, HEIGHT, hWnd, NULL, hInst, NULL);
		hBTNAddLink = CreateWindowEx(NULL, L"BUTTON", L"Add Link", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, START_X + 520, START_Y, 80, HEIGHT, hWnd, NULL, hInst, NULL);
		hBTNAddBulk = CreateWindowEx(NULL, L"BUTTON", L"Add Bulk", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, START_X + 610, START_Y, 80, HEIGHT, hWnd, NULL, hInst, NULL);
		hMaxParall = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_RIGHT, START_X, START_Y + 2 * HEIGHT, 50, HEIGHT, hWnd, NULL, hInst, NULL);
		hBTNMaxParall = CreateWindowEx(NULL, L"BUTTON", L"Apply", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 320, START_Y + 2 * HEIGHT, 80, 20, hWnd, NULL, hInst, NULL);
		SetWindowText(hMaxParall, L"1");
		hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_VISIBLE | WS_VSCROLL | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE, START_X, /*100*/ START_Y + 6 * HEIGHT, 500, 300, hWnd, NULL, hInst, NULL);
		hBTNStart = CreateWindowEx(NULL, L"BUTTON", L"Start", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, START_X + 520, /*100*/ START_Y + 5 * HEIGHT + SPACING, 170, 50, hWnd, NULL, hInst, NULL);
		hBTNPause = CreateWindowEx(NULL, L"BUTTON", L"Pause", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, START_X + 520, /*160*/ START_Y + 5 * HEIGHT + SPACING + 60, 170, 50, hWnd, NULL, hInst, NULL);
		hBTNStop = CreateWindowEx(NULL, L"BUTTON", L"Stop", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, START_X + 520, /*220*/ START_Y + 5 * HEIGHT + SPACING + 120, 170, 50, hWnd, NULL, hInst, NULL);

		hChkboxAll = CreateWindow(TEXT("button"), TEXT("All"),
			WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
			START_X, START_Y + 6 * HEIGHT + 300 + 5, 185, 20,
			hWnd, (HMENU)IDC_CHECKBOX_ALL, hInst, NULL);
		CheckDlgButton(hWnd, IDC_CHECKBOX_ALL, BST_CHECKED);
		hChkboxFailed = CreateWindow(TEXT("button"), TEXT("Download Failed"),
			WS_VISIBLE | WS_GROUP | WS_CHILD | BS_AUTOCHECKBOX,
			START_X, START_Y + 6 * HEIGHT + 300 + 25, 185, 20,
			hWnd, (HMENU)IDC_CHECKBOX_FAILED, hInst, NULL);

		hChkboxOK = CreateWindow(TEXT("button"), TEXT("Download OK"),
			WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
			START_X, START_Y + 6 * HEIGHT + 300 + 45, 185, 20,
			hWnd, (HMENU)IDC_CHECKBOX_OK, hInst, NULL);

		hChkboxDownloading = CreateWindow(TEXT("button"), TEXT("Downloading"),
			WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
			START_X, START_Y + 6 * HEIGHT + 300 + 65, 185, 20,
			hWnd, (HMENU)IDC_CHECKBOX_DOWNLOADING, hInst, NULL);

		hChkboxWaiting = CreateWindow(TEXT("button"), TEXT("Waiting"),
			WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
			START_X, START_Y + 6 * HEIGHT + 300 + 85, 185, 20,
			hWnd, (HMENU)IDC_CHECKBOX_WAITING, hInst, NULL);
		updateOutput();
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDC_CHECKBOX_ALL:
		{
								 LRESULT dwPos = SendMessage(hChkboxAll, BM_GETCHECK, 0, 0);
								 if (dwPos == BST_CHECKED)
								 {
									 CheckDlgButton(hWnd, IDC_CHECKBOX_FAILED, BST_UNCHECKED);
									 CheckDlgButton(hWnd, IDC_CHECKBOX_OK, BST_UNCHECKED);
									 CheckDlgButton(hWnd, IDC_CHECKBOX_DOWNLOADING, BST_UNCHECKED);
									 CheckDlgButton(hWnd, IDC_CHECKBOX_WAITING, BST_UNCHECKED);
									 outputType = 0;

								 }
								 else if (dwPos == BST_UNCHECKED) {
									 // do some stuff
									 CheckDlgButton(hWnd, IDC_CHECKBOX_ALL, BST_CHECKED);
								 }
								 outputType = 0;
								 updateOutput();
		}

			break;
		case IDC_CHECKBOX_FAILED:
		{
									LRESULT dwPos = SendMessage(hChkboxFailed, BM_GETCHECK, 0, 0);
									if (dwPos == BST_CHECKED)
									{
										CheckDlgButton(hWnd, IDC_CHECKBOX_ALL, BST_UNCHECKED);
										CheckDlgButton(hWnd, IDC_CHECKBOX_OK, BST_UNCHECKED);
										CheckDlgButton(hWnd, IDC_CHECKBOX_DOWNLOADING, BST_UNCHECKED);
										CheckDlgButton(hWnd, IDC_CHECKBOX_WAITING, BST_UNCHECKED);
										outputType = ItemStatus::Failed;

									}
									else if (dwPos == BST_UNCHECKED) {
										// do some stuff
										CheckDlgButton(hWnd, IDC_CHECKBOX_ALL, BST_CHECKED);
										outputType = 0;
									}
									updateOutput();
		}

			break;
		case IDC_CHECKBOX_OK:
		{
								LRESULT dwPos = SendMessage(hChkboxOK, BM_GETCHECK, 0, 0);
								if (dwPos == BST_CHECKED)
								{
									CheckDlgButton(hWnd, IDC_CHECKBOX_ALL, BST_UNCHECKED);
									CheckDlgButton(hWnd, IDC_CHECKBOX_FAILED, BST_UNCHECKED);
									CheckDlgButton(hWnd, IDC_CHECKBOX_DOWNLOADING, BST_UNCHECKED);
									CheckDlgButton(hWnd, IDC_CHECKBOX_WAITING, BST_UNCHECKED);
									outputType = ItemStatus::OK;

								}
								else if (dwPos == BST_UNCHECKED) {
									// do some stuff
									CheckDlgButton(hWnd, IDC_CHECKBOX_ALL, BST_CHECKED);
									outputType = 0;
								}
								updateOutput();
		}

			break;
		case IDC_CHECKBOX_DOWNLOADING:
		{
										 LRESULT dwPos = SendMessage(hChkboxDownloading, BM_GETCHECK, 0, 0);
										 if (dwPos == BST_CHECKED)
										 {
											 CheckDlgButton(hWnd, IDC_CHECKBOX_ALL, BST_UNCHECKED);
											 CheckDlgButton(hWnd, IDC_CHECKBOX_FAILED, BST_UNCHECKED);
											 CheckDlgButton(hWnd, IDC_CHECKBOX_OK, BST_UNCHECKED);
											 CheckDlgButton(hWnd, IDC_CHECKBOX_WAITING, BST_UNCHECKED);
											 outputType = ItemStatus::Downloading;

										 }
										 else if (dwPos == BST_UNCHECKED) {
											 // do some stuff
											 CheckDlgButton(hWnd, IDC_CHECKBOX_ALL, BST_CHECKED);
											 outputType = 0;
										 }
										 updateOutput();
		}

			break;
		case IDC_CHECKBOX_WAITING:
		{
									 LRESULT dwPos = SendMessage(hChkboxWaiting, BM_GETCHECK, 0, 0);
									 if (dwPos == BST_CHECKED)
									 {
										 CheckDlgButton(hWnd, IDC_CHECKBOX_ALL, BST_UNCHECKED);
										 CheckDlgButton(hWnd, IDC_CHECKBOX_FAILED, BST_UNCHECKED);
										 CheckDlgButton(hWnd, IDC_CHECKBOX_DOWNLOADING, BST_UNCHECKED);
										 CheckDlgButton(hWnd, IDC_CHECKBOX_OK, BST_UNCHECKED);
										 outputType = ItemStatus::Waiting;

									 }
									 else if (dwPos == BST_UNCHECKED) {
										 CheckDlgButton(hWnd, IDC_CHECKBOX_ALL, BST_CHECKED);
										 outputType = 0;
									 }
									 updateOutput();
		}

			break;
		case BN_CLICKED:
			if (hBTNAddLink == (HWND)lParam)
			{
				TCHAR szBuffer[INTERNET_MAX_URL_LENGTH];

				int iRead = GetWindowText(hInput, szBuffer, INTERNET_MAX_URL_LENGTH);
				if (iRead != 0)
				{
					if (Utils::validateUrl(szBuffer))
					{
						wManager.add(szBuffer);
						updateOutput();
					}
					else
					{
						MessageBox(hWnd, L"Provided URL is not valid", L"Info", MB_OK);
					}
				}
				else
				{
					MessageBox(hWnd, L"Provide link", L"Info", MB_OK);
				}
			}
			if (hBTNAddBulk == (HWND)lParam)
			{
				TCHAR szUrl[INTERNET_MAX_URL_LENGTH];

				int iRead = GetWindowText(hInput, szUrl, INTERNET_MAX_URL_LENGTH);
				if (iRead != 0)
				{
					if (Utils::validateUrl(szUrl))
					{
						wManager.addBulk(szUrl);
						updateOutput();
					}
					else
					{
						MessageBox(hWnd, L"Provided URL is not valid", L"Info", MB_OK);
					}
				}
				else
				{
					MessageBox(hWnd, L"Provide link", L"Info", MB_OK);

				}
			}
			if (hBTNMaxParall == (HWND)lParam)
			{
				TCHAR szBuffer[1000];
				GetWindowText(hMaxParall, szBuffer, 1000);
				int mxParallel = _wtoi(szBuffer);
				if (mxParallel > 10 || mxParallel < 1) {
					MessageBox(hWnd, L"Valid values are 1 - 10", L"Info", MB_OK);
				}
				else {
					wManager.setMaxThreads(mxParallel);
				}
			}
			if (hBTNStart == (HWND)lParam)
			{
				int decision = MessageBox(hWnd, L"Start download job", L"Info", MB_OKCANCEL);
				if (decision == IDOK)
					wManager.start();
			}

			if (hBTNPause == (HWND)lParam)
			{
				int decision = MessageBox(hWnd, L"Pause download job", L"Info", MB_OKCANCEL);
				if (decision == IDOK)
					wManager.pause();
				updateOutput();

			}
			if (hBTNStop == (HWND)lParam)
			{
				int decision = MessageBox(hWnd, L"Stop download job", L"Info", MB_OKCANCEL);
				if (decision == IDOK)
					wManager.stop();
				updateOutput();

			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
	{
					 hdc = BeginPaint(hWnd, &ps);
					 SetBkMode(hdc, TRANSPARENT);
					 // TODO: Add any drawing code here...
					 EndPaint(hWnd, &ps);
					 break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_UPDATE_OUTPUT:
		updateOutput();
		break;
	case WM_ERROR_OUTPUT:
	{
		wstring * message = (wstring *)lParam;
		MessageBox(hWnd, message->c_str(), L"Info", MB_OK);
		delete message;

	}
		break;
	case WM_COPYDATA:
	{
		COPYDATASTRUCT * pcds = (COPYDATASTRUCT *)lParam;
		LPCTSTR lpszString = (LPCTSTR)(pcds->lpData);
		MessageBox(hWnd, lpszString, L"Info", MB_OK);
		if (Utils::validateUrl(lpszString))
		{
			wManager.add(lpszString);
			updateOutput();
		}
		break;
	}
	case WM_AUTO_START:
		wManager.start();
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void AppendText( const HWND &hwnd, TCHAR *newText )
{
    // get edit control from dialog
    HWND hwndOutput = GetDlgItem( hwnd, WND_OUTPUT );

    // get new length to determine buffer size
    int outLength = GetWindowTextLength( hwndOutput ) + lstrlen(newText) + 1;

    // create buffer to hold current and new text
    TCHAR * buf = ( TCHAR * ) GlobalAlloc( GPTR, outLength * sizeof(TCHAR) );
    if (!buf) return;

    // get existing text from edit control and put into buffer
    GetWindowText( hwndOutput, buf, outLength );

    // append the newText to the buffer
    _tcscat_s( buf, outLength, newText );

    // Set the text in the edit control
    SetWindowText( hwndOutput, buf );

    // free the buffer
    GlobalFree( buf );
}

void updateOutput()
{
	std::vector<Item *>::const_iterator it;
	std::wstring buffer;
	std::wstringstream ss;
	for (it = wManager.cbegin(); it != wManager.cend(); ++it)
	{
		if (outputType == 0 || (*it)->getStatus() == outputType)
		{
			ss << (*it)->toString() <<  L"\r\n";
		}
	}
	SetWindowText(hOutput, ss.str().c_str());
}


