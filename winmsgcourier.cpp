/* winmsgcourier.cpp
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

winmsgcourier.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

winmsgcourier.cpp was developed by Carlos Justiniano for use on the 
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with winmsgcourier.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file winmsgcourier.cpp
 @brief msgCourier Windows Main
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence
 @note This is where a number of core objects are instantiated
*/

#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <wininet.h>
#include <direct.h> // for _chdir()
#include <shellapi.h>
#include <string>

#include "resource1.h"

#include "master.h"
#include "core.h"

#include "win32stackwalker.h"
#define MAX_LOADSTRING 100

#define SERVER_CMD_SHUTDOWN		1000
#define SERVER_CMD_SHOWHIDE		2000

HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// The title bar text

HWND g_hEdit = NULL;
HFONT g_hEditFont = NULL;

// Foward declarations of functions included in this code module:
ATOM				MSRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void ForcedExit(char *pMsg);
void DoShutdown();
bool bNotShutdown=true;
bool bHidden=true;

using namespace std;
//using namespace Botan;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// change directory to path specified by commandline argument
	string sCommandLine = (const char*)lpCmdLine;
	string::size_type idx;
	idx = sCommandLine.rfind('"');
	if (idx != string::npos)
		sCommandLine.erase(idx, 1);
	idx = sCommandLine.find('"');
	if (idx != string::npos)
		sCommandLine.erase(idx, 1);
	_chdir(sCommandLine.c_str());

	// Improve the time granularity of Sleep() and other timer functions
	// set to one millisecond.
	timeBeginPeriod(1);

	MSG msg;
	HACCEL hAccelTable;

	try 
	{
		// Initialize global strings
		LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(hInstance, IDC_WINMSGCOURIER, szWindowClass, MAX_LOADSTRING);

		// Allow only one instance of this app
		HANDLE h = CreateMutex(NULL,TRUE,szWindowClass);
		DWORD dwErrorCode = GetLastError();
		if (dwErrorCode == ERROR_ALREADY_EXISTS)
		{
			ReleaseMutex(h);
			return FALSE;
		}

		MSRegisterClass(hInstance);

		// Perform application initialization:
//nCmdShow = SW_MINIMIZE;
		if (!InitInstance (hInstance, nCmdShow)) 
		{
			return FALSE;
		}

		hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_WINMSGCOURIER);
		
		// Initialize WinSock.dll
		WORD wVersionRequested = MAKEWORD(1,1);
		WSADATA wsaData;
		int nRet = WSAStartup(wVersionRequested, &wsaData);
		if (nRet)
		{
			WSACleanup();
			THROW("Initialize WinSock.dll failed");
			return -1;
		}

		char *pargv[3];
		pargv[0] = "msgCourier";
		pargv[1] = "-d";
		pargv[2] = 0;
		int hr = cLog::Create(2, pargv);
		cLog::GetInstance()->SetEditCtl(g_hEdit);

		//
		// start system core
		//
		char szFileName[260];
		GetModuleFileName(GetModuleHandle(NULL),  szFileName, 260);
		string path = szFileName;
		string moduleName;
		string::size_type idxStart = path.rfind("\\");
		string::size_type idxEnd = path.rfind(".");
		if (idxStart != string::npos && idxEnd != string::npos)
		{
			moduleName = path.substr(idxStart+1, idxEnd - idxStart - 1);
		}
		cCore::Create(moduleName.c_str());

		// Main message loop:
		while (GetMessage(&msg, NULL, 0, 0)) 
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		DoShutdown();
		ReleaseMutex(h);
		return msg.wParam;
	}
	catch (char* str)
	{
		std::string sInfo;
		sInfo = "FATAL EXCEPTION: ";
		sInfo += str;
		//sInfo += "\n";
		//cWin32StackWalker::WalkStack(false);
		//cWin32StackWalker::CopyString(sInfo);
		LOGERROR(sInfo.c_str());
		ExitProcess(99);
		//exit(99);
	}
	catch (const char * str)
	{
		std::string sInfo;
		sInfo = "FATAL EXCEPTION: ";
		sInfo += str;
		//sInfo += "\n";
		//cWin32StackWalker::WalkStack(false);
		//cWin32StackWalker::CopyString(sInfo);
		LOGERROR(sInfo.c_str());
		ExitProcess(99);
		//exit(99);
	}
	catch (cException &ex) // msgCourier exceptions
	{
		ex.Log();
		ExitProcess(99);
		//exit(99);
	}
	catch (exception &ex) // STL exceptions
	{
		std::string sInfo;
		sInfo = "FATAL EXCEPTION: ";
		sInfo = ex.what();
		//sInfo += "\n";
		//cWin32StackWalker::WalkStack(false);
		//cWin32StackWalker::CopyString(sInfo);
		LOGERROR(sInfo.c_str());
		ExitProcess(99);
		//exit(99);
	}
	catch (...)
	{
		// dump stack to determine what when wrong.
		//std::string sInfo;
		//cWin32StackWalker::WalkStack(false);
		//cWin32StackWalker::CopyString(sInfo);
		//LOGERROR(sInfo.c_str());
		LOGERROR("Unknown exception. Caught in winmsgcourier.cpp::220");
		ExitProcess(99);
		//exit(99);
	}

	// end time improvements.
	timeEndPeriod(1);
	return 0;
}

ATOM MSRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_ICON);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; //(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDR_MENU;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_ICON);

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, 800, 400, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

	DWORD dwStyle  = /*WS_BORDER | */WS_CHILD | WS_VISIBLE | WS_VSCROLL |
					 ES_MULTILINE;
	dwStyle |= ES_READONLY;

	g_hEdit = CreateWindowEx(0/*WS_EX_DLGMODALFRAME*/, "EDIT", "", dwStyle,
	  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, hInstance, NULL);

	// fix text formating
	SendMessage(g_hEdit, EM_FMTLINES, (WPARAM)TRUE, 0);
	g_hEditFont = CreateFont(16,0,0,0, FW_NORMAL, FALSE, FALSE, FALSE,
							 ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY, FF_SWISS, "Courier New");

	SendMessage(g_hEdit, WM_SETFONT, (WPARAM)g_hEditFont, TRUE);

	RECT rcWin;
	GetClientRect(hWnd, &rcWin);
	MoveWindow(g_hEdit,0,0,rcWin.right-rcWin.left, rcWin.bottom - rcWin.top, TRUE);

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_INTERNET_CLASSES;
	InitCommonControlsEx(&icex); 

//#ifndef _DEBUG
//	ShowWindow(hWnd, SW_HIDE);
//#else
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
//#endif //_DEBUG
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case ID_HELP_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTDLG, hWnd, (DLGPROC)About);
				   break;
				case ID_FILE_EXIT:
					SendMessage(hWnd, WM_CLOSE,0,0);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;

  		case WM_COPYDATA:
  			PCOPYDATASTRUCT pcs;
  			pcs = (PCOPYDATASTRUCT) lParam;
  			if (pcs != NULL)
  			{
  				switch (pcs->dwData)
  				{
  					case SERVER_CMD_SHUTDOWN:
						if (g_hEditFont != NULL)
							DeleteObject(g_hEditFont);
						TerminateProcess(GetCurrentProcess(), 0);
  						break;
					case SERVER_CMD_SHOWHIDE:
						bHidden = !bHidden;
						if (bHidden)
							ShowWindow(hWnd, SW_HIDE);
						else
							ShowWindow(hWnd, SW_NORMAL);
						break;
  				}
  			}
  			break;

		case WM_CLOSE:
			{
				int iRet;
/*
#ifndef _DEBUG
				iRet = ::MessageBox(hWnd,
								"If you close this program this machine will not\n"
								"be able to service network requests.\n\n"
								"Are you sure you want to close this program?\n",
								"msgCourier", MB_YESNO | MB_ICONINFORMATION);
#else
*/
				iRet = IDYES;
//#endif //!_DEBUG
				if (iRet == IDYES)
				{
					if (g_hEditFont != NULL)
						DeleteObject(g_hEditFont);

					TerminateProcess(GetCurrentProcess(), 0);
					//abort();
					/*
					//cCore::GetInstance()->Shutdown();

					cLog::GetInstance()->SetEditCtl(NULL);
				    DestroyWindow(hWnd);

					PostQuitMessage(0);
					*/
				}
			}
			break;

		case WM_SIZE:
			{
				RECT rcWin;
				GetClientRect(hWnd, &rcWin);
				MoveWindow(g_hEdit,0,0,rcWin.right-rcWin.left, rcWin.bottom - rcWin.top, TRUE);
			}
			return 1;

		case WM_ERASEBKGND:
			return 1;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			{
				// Get the size of the Version Info Block
				DWORD dwLen = GetFileVersionInfoSize(APP_NAME, 0);

				// allocate memory for version block
				void *pVersionInfo = malloc(dwLen);
				if (pVersionInfo == 0)
					return TRUE;

				// Get the version block
				char szVersionBuffer[30];
				BOOL bRet = GetFileVersionInfo(APP_NAME,0,dwLen,pVersionInfo);
				if (bRet)
				{
					VS_FIXEDFILEINFO *fileinfo;
					unsigned int size;
					bRet = VerQueryValue(pVersionInfo, 
							TEXT("\\"), (LPVOID*)&fileinfo, &size);
					if (bRet)
					{
						sprintf(szVersionBuffer,"%d.%d.%d.%d",
								HIWORD(fileinfo->dwFileVersionMS),
								LOWORD(fileinfo->dwFileVersionMS),
								HIWORD(fileinfo->dwFileVersionLS),
								LOWORD(fileinfo->dwFileVersionLS));

						HWND hCtl = GetDlgItem(hDlg, IDC_VERSIONLABEL);
						SendMessage(hCtl, WM_SETTEXT, 0, (LPARAM)szVersionBuffer);
					}
				}
				// Cleanup
				free(pVersionInfo);
			}
			return TRUE;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			if (LOWORD(wParam) == IDC_WEBVISIT)
			{
				ShellExecute(hDlg, "open", "http://www.msgcourier.com", NULL,NULL,SW_SHOWDEFAULT);
			}
			break;
	}
    return FALSE;
}

void DoShutdown()
{
    LOG("Initiating shutdown sequence...");

	cCore::Destroy();

#ifdef _PLATFORM_WIN32
	 WSACleanup();
#endif //_PLATFORM_WIN32

    LOG("Clean shutdown complete.\n");
    cLog::Destroy();
}

