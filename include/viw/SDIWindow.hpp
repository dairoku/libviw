// =============================================================================
//  SDIWindow.hpp
//
//  Written in 2014 by Dairoku Sekiguchi (sekiguchi at acm dot org)
//
//  To the extent possible under law, the author(s) have dedicated all copyright
//  and related and neighboring rights to this software to the public domain worldwide.
//  This software is distributed without any warranty.
//
//  You should have received a copy of the CC0 Public Domain Dedication along with
//  this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
// =============================================================================
/*!
	\file		viw/SDIWindow.h
	\author		Dairoku Sekiguchi
	\version	1.0.0
	\date		2014/02/22
	\brief		Header file for viw library types

	This file defines types for viw library
*/

#ifndef VIW_SDI_WINDOW_H
#define VIW_SDI_WINDOW_H

// Includes --------------------------------------------------------------------
#include "viw/Window.hpp"
#include <commctrl.h>
#include <process.h>	//	_beginthread, _endthread
#include <string.h>
#include <stdio.h>
#include <vector>

// Macros ----------------------------------------------------------------------
#define	VIW_SDI_WINDOW_CLASS_NAME			TEXT("viw::SDIWindow")
#pragma warning(disable:4996)
#ifdef _WIN64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


// Namespace -------------------------------------------------------------------
namespace viw
{
	// -------------------------------------------------------------------------
	// SDIWindow class
	// -------------------------------------------------------------------------
	class	SDIWindow : public Window
	{
	public:
		// Constatns -----------------------------------------------------------
		const static int	WINDOW_AUTO_POS				= -100000;
		const static int	WINDOW_POS_SPACE			= 20;
		const static int	WINDOW_DEFAULT_POS			= 20;
		const static int	WINDOW_DEFAULT_SIZE			= 320;

		// Constructors and Destructor -----------------------------------------
		// ---------------------------------------------------------------------
		//	SDIWindow
		// ---------------------------------------------------------------------
		SDIWindow()
		{
			mWindowState			= WINDOW_INIT_STATE;
			mPosX					= WINDOW_AUTO_POS;
			mPosY					= WINDOW_AUTO_POS;
			mWindowTitle			= NULL;

			mWindowH				= NULL;
			mToolbarH				= NULL;
			mStatusbarH				= NULL;
			mRebarH					= NULL;
			mMenuH					= NULL;
			mPopupMenuH				= NULL;
			mAppIconH				= NULL;

			mModuleH				= ::GetModuleHandle(NULL);
			mEventHandle			= NULL;
			mThreadHandle			= NULL;

			mIsFullScreenMode		= false;
			mIsMenubarEnabled		= true;
			mIsToolbarEnabled		= true;
			mIsStatusbarEnabled		= true;

			mMonitorNum				= 0;
			mMonitorRect			= NULL;
		}
		// ---------------------------------------------------------------------
		//	~SDIWindow
		// ---------------------------------------------------------------------
		virtual ~SDIWindow()
		{
			if (mWindowState == WINDOW_OPEN_STATE)
				::PostMessage(mWindowH, WM_CLOSE, 0, 0);

			if (mThreadHandle != NULL)
			{
				::WaitForSingleObject(mThreadHandle, INFINITE);
				::CloseHandle(mThreadHandle);
			}

			if (mEventHandle != NULL)
				::CloseHandle(mEventHandle);

			if (mWindowTitle != NULL)
				delete mWindowTitle;
		}

		// Member Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// showWindow
		// ---------------------------------------------------------------------
		void	showWindow(const char *inWindowName = NULL,
					int inPosX = WINDOW_AUTO_POS,
					int inPosY = WINDOW_DEFAULT_POS)
		{
			static int	sPrevPosX	= 0;
			static int	sPrevPosY	= 0;
			static int	sWindowNum	= 0;

			if (mWindowState != WINDOW_INIT_STATE || mThreadHandle != NULL)
				return;

			mPosX = inPosX;
			mPosY = inPosY;
			if (mPosX == WINDOW_AUTO_POS ||
				mPosY == WINDOW_AUTO_POS)
			{
				if (sPrevPosX == 0 && sPrevPosY == 0)
				{
					mPosX = WINDOW_DEFAULT_POS;
					mPosY = WINDOW_DEFAULT_POS;
				}
				else
				{
					mPosX = sPrevPosX + WINDOW_POS_SPACE;
					mPosY = sPrevPosY + WINDOW_POS_SPACE;

					if (mPosX >= ::GetSystemMetrics(SM_CXMAXIMIZED) - WINDOW_POS_SPACE)
						mPosX = WINDOW_DEFAULT_POS;
					
					if (mPosY >= ::GetSystemMetrics(SM_CYMAXIMIZED) - WINDOW_POS_SPACE)
						mPosY = WINDOW_DEFAULT_POS;
				}

				sPrevPosX = mPosX;
				sPrevPosY = mPosY;
			}

			char	windowName[80];
			if (inWindowName == NULL)
			{
				sprintf_s(windowName, 80, "%s%d", "Untitled", sWindowNum);
				inWindowName = windowName;
			}

			size_t	bufSize = ::strlen(inWindowName) + 1;
			mWindowTitle = new char[bufSize];
			if (mWindowTitle == NULL)
			{
				printf("Error: Can't allocate mWindowTitle\n");
				return;
			}
			::strcpy_s(mWindowTitle, bufSize, inWindowName);
			if (registerWindowClass() == 0)
			{
				printf("Error: registerWindowClass() failed\n");
				return;
			}

			mEventHandle = ::CreateEvent(NULL, false, false, NULL);
			if (mEventHandle == NULL)
			{
				printf("Error: Can't create Event object\n");
				delete mWindowTitle;
				return;
			}

			//	Must use _beginthreadex instead of _beginthread, CreateThread
			mThreadHandle = (HANDLE )::_beginthreadex(
				NULL,
				0,
				threadFunc,
				this,
				0,
				NULL);
			if (mThreadHandle == NULL)
			{
				printf("Error: Can't create process thread\n");
				::CloseHandle(mEventHandle);
				delete mWindowTitle;
				return;
			}
			::WaitForSingleObject(mEventHandle, INFINITE);

			sWindowNum++;
		}
		// ---------------------------------------------------------------------
		// closeWindow
		// ---------------------------------------------------------------------
		void	closeWindow()
		{
			if (mWindowState == WINDOW_OPEN_STATE)
				::PostMessage(mWindowH, WM_CLOSE, 0, 0);
		}
		// ---------------------------------------------------------------------
		// isWindowOpen
		// ---------------------------------------------------------------------
		bool	isWindowOpen()
		{
			if (mWindowState == WINDOW_OPEN_STATE)
				return true;
			return false;
		}
		// ---------------------------------------------------------------------
		// isWindowClosed
		// ---------------------------------------------------------------------
		bool	isWindowClosed()
		{
			if (mWindowState == WINDOW_CLOSED_STATE)
				return true;
			return false;
		}
		// ---------------------------------------------------------------------
		// waitForWindowClose
		// ---------------------------------------------------------------------
		bool	waitForWindowClose(DWORD inTimeout = INFINITE)
		{
			if (mThreadHandle == NULL)
				return false;

			if (WaitForSingleObject(mThreadHandle, inTimeout) == WAIT_OBJECT_0)
				return true;

			return false;
		}
		// ---------------------------------------------------------------------
		// waitForWindowCloseMulti
		// ---------------------------------------------------------------------
		static bool	waitForWindowCloseMulti(SDIWindow *inWindowArray[], int inArrayLen,
												bool inWaitAll = false, DWORD inTimeout = INFINITE)
		{
			std::vector< HANDLE >	handles;

			for (int i = 0; i < inArrayLen; i++)
				if (inWindowArray[i]->mThreadHandle != NULL)
					handles.push_back(inWindowArray[i]->mThreadHandle);

			if (WaitForMultipleObjects((DWORD )handles.size(), &(handles[0]), inWaitAll, inTimeout) == WAIT_TIMEOUT)
				return false;

			return true;
		}
		// ---------------------------------------------------------------------
		// isFullScreenMode
		// ---------------------------------------------------------------------
		bool	isFullScreenMode()
		{
			return mIsFullScreenMode;
		}
		// ---------------------------------------------------------------------
		// enableFullScreenMode
		// ---------------------------------------------------------------------
		void	enableFullScreenMode(int inMonitorIndex = -1)
		{
			if (mIsFullScreenMode)
				return;

			RECT	rectList[MONITOR_ENUM_MAX];
			RECT	monitorRect = {0, 0, 0, 0};
			int		index = 0;

			mMonitorNum = 0;
			mMonitorRect = rectList;
			::EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC )monitorEnumProc, (LPARAM )this);
			::GetWindowRect(mWindowH, &mLastWindowRect);

			switch (inMonitorIndex)
			{
				case -1:
					POINT	pos;
					for (int i = 0; i < mMonitorNum; i++)
					{
						pos.x = mLastWindowRect.left;
						pos.y = mLastWindowRect.top;
						if (::PtInRect(&mMonitorRect[i], pos) == TRUE)
						{
							index = i;
							break;
						}
					}
					monitorRect = mMonitorRect[index];
					break;
				case -2:
					for (int i = 0; i < mMonitorNum; i++)
					{
						if (mMonitorRect[i].left < monitorRect.left)
							monitorRect.left = mMonitorRect[i].left;
						if (mMonitorRect[i].top < monitorRect.top)
							monitorRect.top = mMonitorRect[i].top;
						if (mMonitorRect[i].right > monitorRect.right)
							monitorRect.right = mMonitorRect[i].right;
						if (mMonitorRect[i].bottom > monitorRect.bottom)
							monitorRect.bottom = mMonitorRect[i].bottom;
					}
					break;
				default:
					index = inMonitorIndex;
					if (index < 0 || index >= mMonitorNum)
						index = 0;
					monitorRect = mMonitorRect[index];
					break;
			}
			::SetWindowLong(mWindowH, GWL_STYLE, WS_POPUP | WS_VISIBLE);
			::SetWindowPos(mWindowH, HWND_TOPMOST,
				monitorRect.left, monitorRect.top,
				monitorRect.right, monitorRect.bottom,
				SWP_SHOWWINDOW);
			disableMenubar();
			disableToolbar();
			disableStatusbar();
			::CheckMenuItem(mMenuH, IDM_FULL_SCREEN, MF_CHECKED);
			mIsFullScreenMode = true;
			updateViewRect();
		}
		// ---------------------------------------------------------------------
		// disableFullScreenMode
		// ---------------------------------------------------------------------
		void	disableFullScreenMode()
		{
			if (!mIsFullScreenMode)
				return;
			::SetWindowLong(mWindowH, GWL_STYLE,
				WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
			::SetWindowPos(mWindowH, HWND_NOTOPMOST,
				mLastWindowRect.left, mLastWindowRect.top,
				mLastWindowRect.right - mLastWindowRect.left,
				mLastWindowRect.bottom - mLastWindowRect.top,
				SWP_SHOWWINDOW);
			enableMenubar();
			enableToolbar();
			enableStatusbar();
			::CheckMenuItem(mMenuH, IDM_FULL_SCREEN, MF_UNCHECKED);
			mIsFullScreenMode = false;
			updateViewRect();
		}
		// ---------------------------------------------------------------------
		// isMenubarEnabled
		// ---------------------------------------------------------------------
		bool	isMenubarEnabled()
		{
			return mIsMenubarEnabled;
		}
		// ---------------------------------------------------------------------
		// enableMenubar
		// ---------------------------------------------------------------------
		void	enableMenubar()
		{
			if (mIsMenubarEnabled)
				return;
			::SetMenu(mWindowH, mMenuH);
			::CheckMenuItem(mMenuH, IDM_MENUBAR, MF_CHECKED);
			mIsMenubarEnabled = true;
		}
		// ---------------------------------------------------------------------
		// disableMenubar
		// ---------------------------------------------------------------------
		void	disableMenubar()
		{
			if (!mIsMenubarEnabled)
				return;
			::SetMenu(mWindowH, NULL);
			::CheckMenuItem(mMenuH, IDM_MENUBAR, MF_UNCHECKED);
			mIsMenubarEnabled = false;
		}
		// ---------------------------------------------------------------------
		// isToolbarEnabled
		// ---------------------------------------------------------------------
		bool	isToolbarEnabled()
		{
			return mIsToolbarEnabled;
		}
		// ---------------------------------------------------------------------
		// enableToolbar
		// ---------------------------------------------------------------------
		void	enableToolbar()
		{
			if (mIsToolbarEnabled)
				return;
			::ShowWindow(mRebarH, SW_SHOW);
			::CheckMenuItem(mMenuH, IDM_TOOLBAR, MF_CHECKED);
			mIsToolbarEnabled = true;
			updateViewRect(true);
		}
		// ---------------------------------------------------------------------
		// disableToolbar
		// ---------------------------------------------------------------------
		void	disableToolbar()
		{
			if (!mIsToolbarEnabled)
				return;
			::ShowWindow(mRebarH, SW_HIDE);
			::CheckMenuItem(mMenuH, IDM_TOOLBAR, MF_UNCHECKED);
			mIsToolbarEnabled = false;
			updateViewRect(true);
		}
		// ---------------------------------------------------------------------
		// isStatusbarEnabled
		// ---------------------------------------------------------------------
		bool	isStatusbarEnabled()
		{
			return mIsStatusbarEnabled;
		}
		// ---------------------------------------------------------------------
		// enableStatusbar
		// ---------------------------------------------------------------------
		void	enableStatusbar()
		{
			if (mIsStatusbarEnabled)
				return;
			::ShowWindow(mStatusbarH, SW_SHOW);
			::CheckMenuItem(mMenuH, IDM_STATUSBAR, MF_CHECKED);
			mIsStatusbarEnabled = true;
		}
		// ---------------------------------------------------------------------
		// disableStatusbar
		// ---------------------------------------------------------------------
		void	disableStatusbar()
		{
			if (!mIsStatusbarEnabled)
				return;
			::ShowWindow(mStatusbarH, SW_HIDE);
			::CheckMenuItem(mMenuH, IDM_STATUSBAR, MF_UNCHECKED);
			mIsStatusbarEnabled = false;
		}

	protected:
		// Constatns -----------------------------------------------------------
		const static int	MONITOR_ENUM_MAX			= 32;

		enum MenuEventID
		{
			IDM_NEW			=	1024,
			IDM_OPEN,
			IDM_SAVE,
			IDM_SAVE_AS,
			IDM_PRINT,
			IDM_PRINT_PREVIEW,
			IDM_PRINT_SETUP,
			IDM_CLOSE,
			IDM_EXIT,
			IDM_UNDO,
			IDM_REDO,
			IDM_CUT,
			IDM_COPY,
			IDM_PASTE,
			IDM_MENUBAR,
			IDM_TOOLBAR,
			IDM_STATUSBAR,
			IDM_FULL_SCREEN,
			IDM_CASCADE_WINDOW,
			IDM_TILE_WINDOW,
			IDM_ABOUT
		};

		enum WindowState
		{
			WINDOW_INIT_STATE	=	1,
			WINDOW_OPEN_STATE,
			WINDOW_CLOSED_STATE
		};

		// Typedefs ------------------------------------------------------------
		typedef BOOL		(WINAPI *WINAPI_InitCommonControlsEx)(LPINITCOMMONCONTROLSEX);
		typedef HIMAGELIST	(WINAPI *WINAPI_ImageList_Create)(int cx, int cy, UINT flags, int cInitial, int cGrow);
		typedef int			(WINAPI *WINAPI_ImageList_AddMasked)(HIMAGELIST himl, HBITMAP hbmImage, COLORREF crMask);

		// Member Variables ----------------------------------------------------
		int					mWindowState;
		int					mPosX, mPosY;
		char				*mWindowTitle;

		HWND				mWindowH;
		HWND				mToolbarH;
		HWND				mStatusbarH;
		HWND				mRebarH;
		HMENU				mMenuH;
		HMENU				mPopupMenuH;
		HICON				mAppIconH;

		HINSTANCE			mModuleH;
		HANDLE				mThreadHandle;
		HANDLE				mEventHandle;

		bool				mIsFullScreenMode;
		RECT				mLastWindowRect;
		bool				mIsMenubarEnabled;
		bool				mIsToolbarEnabled;
		bool				mIsStatusbarEnabled;

		int					mMonitorNum;
		RECT				*mMonitorRect;

		// Static Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		//	threadFunc
		// ---------------------------------------------------------------------
		static unsigned int _stdcall	threadFunc(void *arg)
		{
			SDIWindow	*window = (SDIWindow *)arg;	
			LPCTSTR	windowName;
			int	result;

			result = window->initBeforeCreateWindow();
			if (result == 0)
			{
				printf("Error: initBeforeCreateWindow() failed %d\n", result);
				window->mWindowState = WINDOW_CLOSED_STATE;
				return 1;
			}

		#ifdef _UNICODE
			int wcharsize = ::MultiByteToWideChar(CP_ACP, 0, window->mWindowTitle, -1, NULL, 0);
			windowName = new WCHAR[wcharsize];
			::MultiByteToWideChar(CP_ACP, 0, window->mWindowTitle, -1, (LPWSTR )windowName, wcharsize);
		#else
			windowName = (LPCSTR )imageDisp->mWindowTitle;
		#endif
			//	Create WinDisp window
			window->mWindowH = ::CreateWindow(
					VIW_SDI_WINDOW_CLASS_NAME,		//	window class name
					windowName,						//	window title
					WS_OVERLAPPEDWINDOW,			//	normal window style
					window->mPosX,					//	x
					window->mPosY,					//	y
					WINDOW_DEFAULT_SIZE,			//	width
					WINDOW_DEFAULT_SIZE,			//	height
					HWND_DESKTOP,					//	no parent window
					window->mMenuH,					//	no menus
					window->mModuleH,				//	handle to this module
					NULL);							//	no lpParam

			if (window->mWindowH == NULL)
			{
				printf("Error: CreateWindow() failed\n");
				window->mWindowState = WINDOW_CLOSED_STATE;
				return 1;
			}

		#ifdef _UNICODE
			delete windowName;
		#endif

		#ifdef _WIN64
			::SetWindowLongPtr(window->mWindowH, GWLP_USERDATA, (LONG_PTR )window);
		#else
			::SetWindowLongPtr(window->mWindowH, GWLP_USERDATA, PtrToLong(window));
		#endif

			window->mWindowState = WINDOW_OPEN_STATE;

			result = window->initAfterCreateWindow();
			if (result == 0)
			{
				::DestroyWindow(window->mWindowH);
				window->mWindowH = NULL;
				printf("Error: initAfterCreateWindow() failed %d\n", result);
				window->mWindowState = WINDOW_CLOSED_STATE;
				return 1;
			}

			::ShowWindow(window->mWindowH, SW_SHOW);
			::SetEvent(window->mEventHandle);

			MSG	msg;
			while (::GetMessage(&msg, NULL, 0, 0) != 0)
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}

			window->mWindowState = WINDOW_CLOSED_STATE;
			return 0;
		}
		// ---------------------------------------------------------------------
		//	monitorEnumProc
		// ---------------------------------------------------------------------
		static BOOL CALLBACK	monitorEnumProc(HMONITOR hMon, HDC hdcMon, LPRECT inMonRect, LPARAM inLParam)
		{
			SDIWindow	*window = (SDIWindow *)inLParam;

			if (window->mMonitorRect == NULL)
			{
				printf("Error: window->mMonitorRect == NULL\n");
				return FALSE;
			}

			printf("Monitor(%d) %d, %d, %d, %d\n", window->mMonitorNum, 
				inMonRect->left, inMonRect->top, inMonRect->right, inMonRect->bottom);
			window->mMonitorRect[window->mMonitorNum].left		= inMonRect->left;
			window->mMonitorRect[window->mMonitorNum].top		= inMonRect->top;
			window->mMonitorRect[window->mMonitorNum].right		= inMonRect->right;
			window->mMonitorRect[window->mMonitorNum].bottom	= inMonRect->bottom;
			if (window->mMonitorNum == MONITOR_ENUM_MAX - 1)
				return FALSE;
			window->mMonitorNum++;
			return TRUE;
		}

		// Member Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		//	onIDM_New
		// ---------------------------------------------------------------------
		virtual bool	onIDM_New(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_Open
		// ---------------------------------------------------------------------
		virtual bool	onIDM_Open(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_Save
		// ---------------------------------------------------------------------
		virtual bool	onIDM_Save(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_SaveAs
		// ---------------------------------------------------------------------
		virtual bool	onIDM_SaveAs(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_PRINT
		// ---------------------------------------------------------------------
		virtual bool	onIDM_PRINT(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_PrintPreview
		// ---------------------------------------------------------------------
		virtual bool	onIDM_PrintPreview(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_PrintSetup
		// ---------------------------------------------------------------------
		virtual bool	onIDM_PrintSetup(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_CLOSE
		// ---------------------------------------------------------------------
		virtual bool	onIDM_CLOSE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			::PostQuitMessage(0);
			return true;
		}
		// ---------------------------------------------------------------------
		//	onIDM_EXIT
		// ---------------------------------------------------------------------
		virtual bool	onIDM_EXIT(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			::PostQuitMessage(0);
			return true;
		}
		// ---------------------------------------------------------------------
		//	onIDM_Undo
		// ---------------------------------------------------------------------
		virtual bool	onIDM_Undo(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_Redo
		// ---------------------------------------------------------------------
		virtual bool	onIDM_Redo(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_CUT
		// ---------------------------------------------------------------------
		virtual bool	onIDM_CUT(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_COPY
		// ---------------------------------------------------------------------
		virtual bool	onIDM_COPY(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_PASTE
		// ---------------------------------------------------------------------
		virtual bool	onIDM_PASTE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_MENUBAR
		// ---------------------------------------------------------------------
		virtual bool	onIDM_MENUBAR(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			if (isMenubarEnabled())
				disableMenubar();
			else
				enableMenubar();

			return true;
		}
		// ---------------------------------------------------------------------
		//	onIDM_TOOLBAR
		// ---------------------------------------------------------------------
		virtual bool	onIDM_TOOLBAR(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			if (isToolbarEnabled())
				disableToolbar();
			else
				enableToolbar();

			return true;
		}
		// ---------------------------------------------------------------------
		//	onIDM_STATUSBAR
		// ---------------------------------------------------------------------
		virtual bool	onIDM_STATUSBAR(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			if (isStatusbarEnabled())
				disableStatusbar();
			else
				enableStatusbar();

			return true;
		}
		// ---------------------------------------------------------------------
		//	onIDM_FULL_SCREEN
		// ---------------------------------------------------------------------
		virtual bool	onIDM_FULL_SCREEN(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			if (isFullScreenMode())
				disableFullScreenMode();
			else
				enableFullScreenMode();

			return true;
		}
		// ---------------------------------------------------------------------
		//	onIDM_CASCADE_WINDOW
		// ---------------------------------------------------------------------
		virtual bool	onIDM_CASCADE_WINDOW(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_TILE_WINDOW
		// ---------------------------------------------------------------------
		virtual bool	onIDM_TILE_WINDOW(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onIDM_ABOUT
		// ---------------------------------------------------------------------
		virtual bool	onIDM_ABOUT(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			::MessageBox(mWindowH,
				TEXT("viw Window Ver.1.0.0\nDairoku Sekiguchi (2014/02/22)"),
				TEXT("About viw Window"), MB_OK);
			return true;
		}
		// ---------------------------------------------------------------------
		//	onWM_DESTROY
		// ---------------------------------------------------------------------
		virtual bool	onWM_DESTROY(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			::PostQuitMessage(0);
			return true;
		}
		// ---------------------------------------------------------------------
		//	onWM_SIZE
		// ---------------------------------------------------------------------
		virtual bool	onWM_SIZE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			SendMessage(mRebarH, inMessage, inWParam, inLParam);
			SendMessage(mStatusbarH, inMessage, inWParam, inLParam);
			return true;
		}
		// ---------------------------------------------------------------------
		// onWM_CHAR
		// ---------------------------------------------------------------------
		virtual bool	onWM_CHAR(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			switch (inWParam)
			{
				case VK_ESCAPE:
					if (isFullScreenMode())
						disableFullScreenMode();
					else
						::PostQuitMessage(0);
					return true;
				case 'f':
				case 'F':
					if (isFullScreenMode())
						disableFullScreenMode();
					else
						enableFullScreenMode();
					return true;
			}
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_COMMAND
		// ---------------------------------------------------------------------
		virtual bool	onWM_COMMAND(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			switch (LOWORD(inWParam))
			{
				case IDM_NEW:
					return onIDM_New(inMessage, inWParam, inLParam, outResult);
				case IDM_OPEN:
					return onIDM_Open(inMessage, inWParam, inLParam, outResult);
				case IDM_SAVE:
					return onIDM_Save(inMessage, inWParam, inLParam, outResult);
				case IDM_SAVE_AS:
					return onIDM_SaveAs(inMessage, inWParam, inLParam, outResult);
				case IDM_PRINT:
					return onIDM_PRINT(inMessage, inWParam, inLParam, outResult);
				case IDM_PRINT_PREVIEW:
					return onIDM_PrintPreview(inMessage, inWParam, inLParam, outResult);
				case IDM_PRINT_SETUP:
					return onIDM_PrintSetup(inMessage, inWParam, inLParam, outResult);
				case IDM_CLOSE:
					return onIDM_CLOSE(inMessage, inWParam, inLParam, outResult);
				case IDM_EXIT:
					return onIDM_EXIT(inMessage, inWParam, inLParam, outResult);
				case IDM_UNDO:
					return onIDM_Undo(inMessage, inWParam, inLParam, outResult);
				case IDM_REDO:
					return onIDM_Redo(inMessage, inWParam, inLParam, outResult);
				case IDM_CUT:
					return onIDM_CUT(inMessage, inWParam, inLParam, outResult);
				case IDM_COPY:
					return onIDM_COPY(inMessage, inWParam, inLParam, outResult);
				case IDM_PASTE:
					return onIDM_PASTE(inMessage, inWParam, inLParam, outResult);
				case IDM_MENUBAR:
					return onIDM_MENUBAR(inMessage, inWParam, inLParam, outResult);
				case IDM_TOOLBAR:
					return onIDM_TOOLBAR(inMessage, inWParam, inLParam, outResult);
				case IDM_STATUSBAR:
					return onIDM_STATUSBAR(inMessage, inWParam, inLParam, outResult);
				case IDM_FULL_SCREEN:
					return onIDM_FULL_SCREEN(inMessage, inWParam, inLParam, outResult);
				case IDM_CASCADE_WINDOW:
					return onIDM_CASCADE_WINDOW(inMessage, inWParam, inLParam, outResult);
				case IDM_TILE_WINDOW:
					return onIDM_TILE_WINDOW(inMessage, inWParam, inLParam, outResult);
				case IDM_ABOUT:
					return onIDM_ABOUT(inMessage, inWParam, inLParam, outResult);
			}
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_RBUTTONDOWN
		// ---------------------------------------------------------------------
		virtual bool	onWM_RBUTTONDOWN(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			POINT	pos;

			pos.x = (short )LOWORD(inLParam);
			pos.y = (short )HIWORD(inLParam);

			HMENU	menu = ::GetSubMenu(mPopupMenuH, 0);
			::ClientToScreen(mWindowH, &pos);
			::TrackPopupMenu(menu, TPM_LEFTALIGN, pos.x, pos.y, 0, mWindowH, NULL);

			return true;
		}
		// ---------------------------------------------------------------------
		// updateWindowSize
		// ---------------------------------------------------------------------
/*		virtual void	updateWindowSize()
		{
		}*/
		// ---------------------------------------------------------------------
		// updateViewRect
		// ---------------------------------------------------------------------
		virtual void	updateViewRect(bool inIsRedraw = false)
		{
			//RECT	rect = getViewRect();
		}
		// ---------------------------------------------------------------------
		// updateStatusBar
		// ---------------------------------------------------------------------
		virtual void	updateStatusBar()
		{
		}
		// ---------------------------------------------------------------------
		// getViewRect
		// ---------------------------------------------------------------------
		RECT	getViewRect(RECT *ioRect = NULL)
		{
			RECT	rebarRect, statusbarRect, rect;

			::GetWindowRect(mRebarH, &rebarRect);
			int	rebarHeight = rebarRect.bottom - rebarRect.top;
			if (!isToolbarEnabled())
				rebarHeight = 0;
			::GetWindowRect(mStatusbarH, &statusbarRect);
			int	statusbarHeight = statusbarRect.bottom - statusbarRect.top;
			if (!isStatusbarEnabled())
				statusbarHeight = 0;

			if (ioRect == NULL)
			{
				::GetClientRect(mWindowH, &rect);
				rect.top += rebarHeight;
				rect.bottom -= statusbarHeight;
				return rect;
			}

			rect = *ioRect;
			rect.top += rebarHeight;
			rect.bottom += rebarHeight;

			ioRect->bottom += rebarHeight + statusbarHeight;

			return rect;
		}
		// ---------------------------------------------------------------------
		// registerWindowClass
		// ---------------------------------------------------------------------
		int	registerWindowClass()
		{
			WNDCLASSEX	wcx;

			if (::GetClassInfoEx(
					mModuleH,
					VIW_SDI_WINDOW_CLASS_NAME,
					&wcx) != 0)
			{
				return -1;	// success (Win32 style result)
			}

			initIcon();

			::ZeroMemory(&wcx,sizeof(WNDCLASSEX));
			wcx.cbSize = sizeof(WNDCLASSEX); 

			wcx.hInstance		= mModuleH;
			wcx.lpszClassName	= VIW_SDI_WINDOW_CLASS_NAME;
			wcx.lpfnWndProc		= windowFunc;
			wcx.style			= CS_DBLCLKS; // Class styles

			wcx.cbClsExtra		= 0;
			wcx.cbWndExtra		= 0;

			wcx.hIcon			= mAppIconH;
			wcx.hIconSm			= mAppIconH;
			wcx.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
			wcx.lpszMenuName	= NULL;

			wcx.hbrBackground	= (HBRUSH)COLOR_WINDOW; // Background brush

			if (::RegisterClassEx(&wcx) == 0)
				return 0;	// fail (Win32 style result)

			return -1;	// success (Win32 style result)
		}
		// ---------------------------------------------------------------------
		//	loadComctl32
		// ---------------------------------------------------------------------
		int	loadComctl32(WINAPI_ImageList_Create *outImageList_Create,
				WINAPI_ImageList_AddMasked *outImageList_AddMasked)
		{
			static HINSTANCE	sComctl32H = NULL;
			static WINAPI_ImageList_Create		sImageList_Create;
			static WINAPI_ImageList_AddMasked	sImageList_AddMasked;

			//	to bypass "comctl32.lib" linking, do some hack...
			if (sComctl32H == NULL)
			{
				WINAPI_InitCommonControlsEx	initCommonControlsEx;
				sComctl32H = ::LoadLibrary(TEXT("comctl32.dll"));
				if (sComctl32H == NULL)
				{
					printf("Can't load comctl32.dll. Panic\n");
					return 0;	// fail (Win32 style result)
				}

				initCommonControlsEx	= (WINAPI_InitCommonControlsEx )::GetProcAddress(sComctl32H, "InitCommonControlsEx");
				sImageList_Create		= (WINAPI_ImageList_Create )::GetProcAddress(sComctl32H, "ImageList_Create");
				sImageList_AddMasked		= (WINAPI_ImageList_AddMasked )::GetProcAddress(sComctl32H, "ImageList_AddMasked");
				if (initCommonControlsEx == NULL ||
					sImageList_Create == NULL ||
					sImageList_AddMasked == NULL)
				{
					printf("GetProcAddress() failed. Panic\n");
					return 0;	// fail (Win32 style result)
				}

				// Ensure that the common control DLL is loaded
				INITCOMMONCONTROLSEX icx;
				icx.dwSize = sizeof(INITCOMMONCONTROLSEX);
				icx.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES; // Specify BAR classes
				(*initCommonControlsEx)(&icx); // Load the common control DLL
			}

			*outImageList_Create = sImageList_Create;
			*outImageList_AddMasked = sImageList_AddMasked;

			return -1;	// success (Win32 style result)
		}
		// ---------------------------------------------------------------------
		//	initBeforeCreateWindow
		// ---------------------------------------------------------------------
		virtual int	initBeforeCreateWindow()
		{
			initMenu();
			return -1;	// success (Win32 style result)
		}
		// ---------------------------------------------------------------------
		//	initAfterCreateWindow
		// ---------------------------------------------------------------------
		virtual int	initAfterCreateWindow()
		{
			initToolbar();
			initCursor();
			return -1;	// success (Win32 style result)
		}
		// ---------------------------------------------------------------------
		//	initMenu
		// ---------------------------------------------------------------------
		virtual void	initMenu()
		{
			mMenuH		= ::CreateMenu();
			mPopupMenuH	= ::CreateMenu();
			HMENU	popupSubMenuH = CreateMenu();
			HMENU	fileMenuH = CreateMenu();
			HMENU	editMenuH = CreateMenu();
			HMENU	viewMenuH = CreateMenu();
			HMENU	zoomMenuH = CreateMenu();
			HMENU	windowMenuH = CreateMenu();
			HMENU	helpMenuH = CreateMenu();

			::AppendMenu(mMenuH, MF_POPUP, (UINT_PTR )fileMenuH, TEXT("&File"));
			::AppendMenu(mMenuH, MF_POPUP, (UINT_PTR )editMenuH, TEXT("&Edit"));
			::AppendMenu(mMenuH, MF_POPUP, (UINT_PTR )viewMenuH, TEXT("&View"));
			::AppendMenu(mMenuH, MF_POPUP, (UINT_PTR )windowMenuH, TEXT("&Window"));
			::AppendMenu(mMenuH, MF_POPUP, (UINT_PTR )helpMenuH, TEXT("&Help"));

			::AppendMenu(mPopupMenuH, MF_POPUP, (UINT_PTR )popupSubMenuH, TEXT("&RightClickMenu"));
			::AppendMenu(popupSubMenuH, MF_POPUP, (UINT_PTR )fileMenuH, TEXT("&File"));
			::AppendMenu(popupSubMenuH, MF_POPUP, (UINT_PTR )editMenuH, TEXT("&Edit"));
			::AppendMenu(popupSubMenuH, MF_POPUP, (UINT_PTR )viewMenuH, TEXT("&View"));
			::AppendMenu(popupSubMenuH, MF_POPUP, (UINT_PTR )windowMenuH, TEXT("&Window"));
			::AppendMenu(popupSubMenuH, MF_POPUP, (UINT_PTR )helpMenuH, TEXT("&Help"));

			::AppendMenu(fileMenuH, MF_ENABLED, IDM_NEW, TEXT("&New"));
			::AppendMenu(fileMenuH, MF_ENABLED, IDM_OPEN, TEXT("&Open..."));
			::AppendMenu(fileMenuH, MF_ENABLED, IDM_SAVE, TEXT("&Save"));
			::AppendMenu(fileMenuH, MF_ENABLED, IDM_SAVE_AS, TEXT("Save &As..."));
			::AppendMenu(fileMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(fileMenuH, MF_ENABLED, IDM_PRINT, TEXT("&Print..."));
			::AppendMenu(fileMenuH, MF_ENABLED, IDM_PRINT_PREVIEW, TEXT("Print Pre&view"));
			::AppendMenu(fileMenuH, MF_ENABLED, IDM_PRINT_SETUP, TEXT("P&rint Setup"));
			::AppendMenu(fileMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(fileMenuH, MF_ENABLED, IDM_CLOSE, TEXT("&Close"));
			//::AppendMenu(fileMenuH, MF_ENABLED, IDM_EXIT, TEXT("E&xit"));

			::AppendMenu(editMenuH, MF_GRAYED, IDM_UNDO, TEXT("&Undo"));
			::AppendMenu(editMenuH, MF_GRAYED, IDM_REDO, TEXT("&Redo"));
			::AppendMenu(editMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(editMenuH, MF_GRAYED, IDM_CUT, TEXT("Cu&t"));
			::AppendMenu(editMenuH, MF_ENABLED, IDM_COPY, TEXT("&Copy"));
			::AppendMenu(editMenuH, MF_GRAYED, IDM_PASTE, TEXT("&Paste"));

			::AppendMenu(viewMenuH, MF_ENABLED, IDM_MENUBAR, TEXT("&Menubar"));
			::AppendMenu(viewMenuH, MF_ENABLED, IDM_TOOLBAR, TEXT("&Toolbar"));
			::AppendMenu(viewMenuH, MF_ENABLED, IDM_STATUSBAR, TEXT("&Status Bar"));
			::AppendMenu(viewMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(viewMenuH, MF_ENABLED, IDM_FULL_SCREEN, TEXT("&Full Screen"));

			::AppendMenu(windowMenuH, MF_GRAYED, IDM_CASCADE_WINDOW, TEXT("&Cascade"));
			::AppendMenu(windowMenuH, MF_GRAYED, IDM_TILE_WINDOW, TEXT("&Tile"));

			::AppendMenu(helpMenuH, MF_ENABLED, IDM_ABOUT, TEXT("&About"));

			//setCursorMode(mCursorMode);

			::CheckMenuItem(mMenuH, IDM_MENUBAR, MF_CHECKED);
			::CheckMenuItem(mMenuH, IDM_TOOLBAR, MF_CHECKED);
			::CheckMenuItem(mMenuH, IDM_STATUSBAR, MF_CHECKED);
		}
		// ---------------------------------------------------------------------
		//	initToolbar
		// ---------------------------------------------------------------------
		virtual void	initToolbar()
		{
			WINAPI_ImageList_Create		imageList_Create;
			WINAPI_ImageList_AddMasked	imageList_AddMasked;

			if (loadComctl32(&imageList_Create, &imageList_AddMasked) == 0)
				return;

			mRebarH = ::CreateWindowEx(WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS
					| RBS_BANDBORDERS | WS_CLIPCHILDREN | CCS_NODIVIDER,
				0, 0, 0, 0, mWindowH, NULL, mModuleH, NULL);

			REBARINFO	rInfo;
			::ZeroMemory(&rInfo, sizeof(REBARINFO));
			rInfo.cbSize = sizeof(REBARINFO);
			::SendMessage(mRebarH, RB_SETBARINFO, 0, (LPARAM )&rInfo);

			mToolbarH = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
				WS_VISIBLE | WS_CHILD| CCS_NODIVIDER | CCS_NORESIZE | TBSTYLE_FLAT,
				0, 0, 0, 0, mRebarH, NULL, mModuleH, NULL);

			::SendMessage(mToolbarH, CCM_SETVERSION, (WPARAM )5, (LPARAM )0);
			::SendMessage(mToolbarH, TB_BUTTONSTRUCTSIZE, (WPARAM )sizeof(TBBUTTON), 0);

			int	bitmapNum;
			const BITMAPINFOHEADER	*bitmapHeader;
			getToolbarBitmapHeader(&bitmapNum, (const unsigned char **)&bitmapHeader);

			HIMAGELIST	imageList = (*imageList_Create)(
				bitmapHeader->biWidth,
				bitmapHeader->biHeight,
				ILC_COLOR24 | ILC_MASK, 1, 1);
			HBITMAP	bitmap;
			for (int i = 0; i < bitmapNum; i++)
			{
				bitmap = CreateDIBitmap(
							GetDC(mToolbarH),
							(BITMAPINFOHEADER *)bitmapHeader,
							CBM_INIT,
							getToolbarBitmapData(i),
							(BITMAPINFO *)bitmapHeader,
							DIB_RGB_COLORS);
				(*imageList_AddMasked)(imageList, bitmap, RGB(255, 0, 255));
				DeleteObject(bitmap);
			}

			int	buttonNum;
			const TBBUTTON	*tbb;

			getToolbarData(&buttonNum, &tbb);

			SendMessage(mToolbarH, TB_SETIMAGELIST, (WPARAM )0, (LPARAM )imageList);
			SendMessage(mToolbarH, TB_ADDBUTTONS, (WPARAM )buttonNum, (LPARAM )tbb);

			REBARBANDINFO	rbInfo;
			ZeroMemory(&rbInfo, sizeof(REBARBANDINFO));
			rbInfo.cbSize = sizeof(REBARBANDINFO);
			rbInfo.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE;
			rbInfo.fStyle = RBBS_CHILDEDGE;
			rbInfo.cxMinChild = 0;
			rbInfo.cyMinChild = 25;
			rbInfo.cx = 120;
			rbInfo.lpText = 0;
			rbInfo.hwndChild = mToolbarH;
			SendMessage(mRebarH, RB_INSERTBAND, (WPARAM )-1, (LPARAM )&rbInfo);

			mStatusbarH = ::CreateWindowEx(0, STATUSCLASSNAME, NULL,
				WS_CHILD | WS_VISIBLE,
				0, 0, 0, 0, mWindowH, NULL, NULL, NULL);
			updateStatusBar();
		}
		// ---------------------------------------------------------------------
		//	initCursor
		// ---------------------------------------------------------------------
		virtual void	initCursor()
		{
		}
		// ---------------------------------------------------------------------
		//	initIcon
		// ---------------------------------------------------------------------
		virtual void	initIcon()
		{
			const unsigned char	*data = getAppIconData();

			mAppIconH = ::CreateIconFromResourceEx((PBYTE )data, 296, TRUE, 0x00030000, 16, 16, 0);
		}
		// ---------------------------------------------------------------------
		//	getToolbarData
		// ---------------------------------------------------------------------
		virtual void	getToolbarData(int *outButtnNum,  const TBBUTTON **outTBUTTON)
		{
			static const TBBUTTON tbb[18] = {
				// iBitmap, idCommand, fsState, fsStyle, bReserved[2], dwData, iString
				{ MAKELONG(0, 0), IDM_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(1, 0), IDM_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(2, 0), IDM_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(0, 0), 0, TBSTATE_ENABLED, TBSTYLE_SEP, { 0, 0 }, 0, 0 },
				{ MAKELONG(3, 0), IDM_PRINT, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(4, 0), IDM_PRINT_PREVIEW, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(5, 0), IDM_PRINT_SETUP, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(0, 0), 0, TBSTATE_ENABLED, TBSTYLE_SEP, { 0, 0 }, 0, 0 },
				{ MAKELONG(6, 0), IDM_UNDO, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(7, 0), IDM_REDO, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(0, 0), 0, TBSTATE_ENABLED, TBSTYLE_SEP, { 0, 0 }, 0, 0 },
				{ MAKELONG(8, 0), IDM_CUT, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(9, 0), IDM_COPY, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(10, 0), IDM_PASTE, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(0, 0), 0, TBSTATE_ENABLED, TBSTYLE_SEP, { 0, 0 }, 0, 0 },
				{ MAKELONG(11, 0), IDM_FULL_SCREEN, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(0, 0), 0, TBSTATE_ENABLED, TBSTYLE_SEP, { 0, 0 }, 0, 0 },
				{ MAKELONG(12, 0), IDM_ABOUT, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 }
			};
			
			*outButtnNum = 18;
			*outTBUTTON = tbb;
		}
		// ---------------------------------------------------------------------
		//	getToolbarBitmapHeader
		// ---------------------------------------------------------------------
		virtual void getToolbarBitmapHeader(int *outBitmapNum, const unsigned char **outHeader)
		{
			static const unsigned char header[] = {
				0x28, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xC3, 0x0E, 0x00, 0x00, 0xC3, 0x0E,
				0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x34,
				0x8A, 0x38, 0x00, 0x0D, 0x26, 0xA1, 0x00, 0xCA, 0xCF, 0xE6, 0x00, 0x1A, 0x7D, 0xC2, 0x00,
				0x7A, 0xB6, 0xDC, 0x00, 0x00, 0xCC, 0xFF, 0x00, 0xBF, 0xD8, 0xE9, 0x00, 0x9C, 0x53, 0x00,
				0x00, 0xD1, 0xB5, 0x96, 0x00, 0xEB, 0xE2, 0xD7, 0x00, 0x42, 0x42, 0x42, 0x00, 0x6F, 0x6F,
				0x6F, 0x00, 0x86, 0x86, 0x86, 0x00, 0xE6, 0xE4, 0xE5, 0x00, 0xF6, 0xF6, 0xF6, 0x00
			};

			*outBitmapNum = 13;
			*outHeader = header;
		}
		// ---------------------------------------------------------------------
		//	getToolbarBitmapData
		// ---------------------------------------------------------------------
		virtual const unsigned char	*getToolbarBitmapData(int inIndex)
		{
			static const unsigned char data[13][128] = {{	// New Document Icon
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF,
					0xF0, 0x00, 0x00, 0x0F, 0xBB, 0xBB, 0xBB, 0xBB, 0xF0, 0x00, 0x00, 0x0F, 0xBF, 0xFF, 0xFF,
					0xFB, 0xF0, 0x00, 0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xF0, 0x00, 0x00, 0x0F, 0xBF, 0xFF,
					0xFF, 0xFB, 0xF0, 0x00, 0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xF0, 0x00, 0x0F, 0xFF, 0xBF,
					0xFF, 0xFF, 0xFB, 0xF0, 0x0F, 0xFF, 0x4F, 0xFF, 0xFF, 0xFB, 0xBD, 0xF0, 0x0F, 0x4F, 0x4F,
					0x4F, 0xFF, 0xFB, 0xDF, 0xF0, 0xFF, 0xF4, 0x44, 0xFF, 0xFB, 0xBD, 0xFF, 0x00, 0xF4, 0x44,
					0xF4, 0x44, 0xFF, 0xFF, 0xF0, 0x00, 0xFF, 0x74, 0x44, 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x0F,
					0x4F, 0x4F, 0x4F, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x4F, 0xFF, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x0F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00
				},{		// Open Icon
					0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x75, 0x55, 0x55, 0x55, 0x55, 0x57,
					0x5F, 0x0F, 0x55, 0x55, 0x55, 0x55, 0x55, 0x5F, 0x5F, 0xFE, 0x55, 0x55, 0x55, 0x55, 0x55,
					0xEF, 0x5F, 0xF7, 0x55, 0x55, 0x55, 0x55, 0x57, 0xFF, 0x5F, 0xF5, 0x55, 0x55, 0x55, 0x55,
					0x5E, 0xFF, 0x5F, 0xFF, 0x5F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x5F, 0x0F, 0xFF, 0xFF, 0xFF,
					0xFF, 0xFF, 0xFF, 0x5F, 0xFA, 0x88, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x5F, 0xF8, 0xDA, 0xFF,
					0xFF, 0xFF, 0xFF, 0xFF, 0x5F, 0xF8, 0xAF, 0xD8, 0xDF, 0xFF, 0xFF, 0xFF, 0x5F, 0xF8, 0xDA,
					0xFD, 0x88, 0xF5, 0x55, 0x55, 0x5F, 0xF9, 0x88, 0x88, 0x88, 0x9F, 0xFF, 0xFF, 0xFF, 0x0F,
					0xFF, 0xE8, 0x8D, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x88, 0xDF, 0xF0, 0x00, 0x00, 0x00,
					0x0F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00
				}, {	// Save Icon
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
					0xF0, 0x0F, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x0F, 0x88, 0x88, 0x88, 0x88, 0x88,
					0x88, 0xF0, 0x0F, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x0F, 0x88, 0x88, 0x88, 0x88,
					0x88, 0x88, 0xF0, 0x0F, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x0F, 0x88, 0x88, 0x88,
					0x88, 0x88, 0x88, 0xF0, 0x0F, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x0F, 0x88, 0x8F,
					0xFF, 0xFF, 0xF8, 0x88, 0xF0, 0x0F, 0x88, 0x8F, 0xFF, 0xFF, 0xF8, 0x88, 0xF0, 0x0F, 0x88,
					0x8F, 0xFF, 0x88, 0xF8, 0x88, 0xF0, 0x0F, 0x88, 0x8F, 0xFF, 0x88, 0xF8, 0x8D, 0xF0, 0x0F,
					0x88, 0x8F, 0xFF, 0x88, 0xF8, 0xDF, 0xF0, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				}, {	//	Cut Icon
					0x00, 0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0xFF, 0x00, 0x0F, 0x3B, 0xBB, 0x3F, 0xF3, 0xBB, 0xB3,
					0xF0, 0x0F, 0xB3, 0xF3, 0xBF, 0xFB, 0x3F, 0x3B, 0xF0, 0x0F, 0xBF, 0xFF, 0xBE, 0xEB, 0xFF,
					0xFB, 0xEF, 0x0F, 0xB3, 0xF3, 0xBE, 0xEB, 0x3F, 0x3B, 0xF0, 0x0F, 0x3B, 0xBB, 0xBB, 0xBB,
					0xBB, 0xB3, 0xF0, 0x00, 0xFF, 0xFE, 0xBE, 0xEB, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFE, 0xBE,
					0xEB, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xCB, 0xBC, 0xF0, 0x00, 0x00, 0x00, 0x00, 0xF3,
					0xBC, 0xCB, 0x3F, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xB9, 0x9B, 0xCF, 0x00, 0x00, 0x00, 0x0F,
					0x3B, 0xCF, 0xFC, 0xB3, 0xF0, 0x00, 0x00, 0x0F, 0xCB, 0x9F, 0xF9, 0xBC, 0xF0, 0x00, 0x00,
					0x0F, 0xBC, 0xFF, 0xFF, 0xCB, 0xF0, 0x00, 0x00, 0x0F, 0xC9, 0xF0, 0x0F, 0x9C, 0xF0, 0x00,
					0x00, 0x0F, 0xFF, 0xF0, 0x0F, 0xFF, 0xF0, 0x00
				}, {	//	Copy Icon
					0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x0F, 0xBB, 0xBB, 0xBB, 0xBB, 0xF0, 0x00,
					0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xF0, 0x00, 0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xF0,
					0x00, 0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xF0, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB,
					0xFB, 0xBB, 0xF0, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xFF, 0xFB, 0xF0, 0x0F, 0xBF, 0xFF, 0xFF,
					0xFB, 0xFF, 0xFB, 0xF0, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xFF, 0xFB, 0xF0, 0x0F, 0xBF, 0xFF,
					0xFF, 0xFB, 0xFF, 0xFB, 0xF0, 0x0F, 0xBB, 0xBB, 0xBB, 0xBB, 0xFF, 0xFB, 0xF0, 0x0F, 0xFF,
					0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xF0, 0x00, 0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xF0, 0x00,
					0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xF0, 0x00, 0x00, 0x0F, 0xBB, 0xBB, 0xBB, 0xBB, 0xF0,
					0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0
				}, {	//	Paste Icon
					0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFB, 0xBB, 0xBB, 0xBF, 0xBB, 0xBB, 0xBF,
					0x00, 0xFB, 0xFF, 0xFF, 0xBF, 0xFF, 0xFF, 0xBF, 0x00, 0xFB, 0xFF, 0xFF, 0xBF, 0xFF, 0xFF,
					0xBF, 0x00, 0xFB, 0xFF, 0xFF, 0xBF, 0xFF, 0xFF, 0xBF, 0x00, 0xFB, 0xFF, 0xFF, 0xBF, 0xFF,
					0xFF, 0xBF, 0x00, 0xFB, 0xFF, 0xFF, 0xBF, 0xFF, 0xFF, 0xBF, 0x00, 0xFB, 0xBB, 0xBB, 0xBF,
					0xFF, 0xFF, 0xBF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFB, 0xFF,
					0xFF, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFB,
					0xBB, 0xBB, 0xBB, 0xBB, 0xBF, 0x00, 0x00, 0xFF, 0xFB, 0xBE, 0xEB, 0xBF, 0xFF, 0x00, 0x00,
					0x00, 0xFF, 0xBE, 0xEB, 0xEF, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xDB, 0xBD, 0xF0, 0x00, 0x00,
					0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xF0, 0x00, 0x00
				}, {	//	Undo Icon
					0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xDA, 0xFF, 0x00, 0x00,
					0x00, 0x00, 0x0F, 0xF9, 0x88, 0x9F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0xFA, 0x88, 0x89, 0xFF,
					0x00, 0x00, 0x00, 0x00, 0x0F, 0xE8, 0x88, 0xDF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x88,
					0x8D, 0xEF, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFD, 0x88, 0x8F, 0xF0, 0x00, 0x00, 0x00, 0x00,
					0xFF, 0xD8, 0x89, 0xF0, 0x0F, 0xFF, 0xFF, 0xFF, 0x0F, 0xFD, 0x88, 0xF0, 0x0F, 0x88, 0x88,
					0xDF, 0xF0, 0xFA, 0x88, 0xF0, 0x0F, 0x88, 0x88, 0x8D, 0xFF, 0xFA, 0x88, 0xF0, 0x0F, 0x88,
					0x88, 0x9F, 0xFF, 0xFD, 0x88, 0xF0, 0x0F, 0x88, 0x88, 0x8D, 0xAA, 0xD8, 0x89, 0xF0, 0x0F,
					0x88, 0xE8, 0x88, 0x88, 0x88, 0x8F, 0xF0, 0x0F, 0xE8, 0xFF, 0x98, 0x88, 0x89, 0xFF, 0x00,
					0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00
				}, {	//	Redo Icon
					0x00, 0x00, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xAD, 0xFF, 0x00,
					0x00, 0x00, 0x00, 0x0F, 0xF9, 0x88, 0x9F, 0xF0, 0x00, 0x00, 0x00, 0xFF, 0x98, 0x88, 0xAF,
					0x00, 0x00, 0x00, 0x0F, 0xFD, 0x88, 0x8E, 0xF0, 0x00, 0x00, 0x00, 0xFE, 0xD8, 0x88, 0xEF,
					0x00, 0x00, 0x00, 0x0F, 0xE8, 0x88, 0xDF, 0xF0, 0x00, 0x00, 0x00, 0x0F, 0x98, 0x8D, 0xFF,
					0x00, 0x00, 0x00, 0x00, 0x0F, 0x88, 0xDF, 0xF0, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0x88, 0xAF,
					0x0F, 0xE8, 0x88, 0x88, 0xF0, 0x0F, 0x88, 0xAF, 0xFF, 0x88, 0x88, 0x88, 0xF0, 0x0F, 0x88,
					0xDF, 0xFF, 0xF9, 0x88, 0x88, 0xF0, 0x0F, 0x98, 0x8D, 0xAA, 0xD8, 0x88, 0x88, 0xF0, 0x0F,
					0xE8, 0x88, 0x88, 0x88, 0x89, 0x8D, 0xF0, 0x00, 0xFE, 0x98, 0x88, 0x89, 0xFA, 0xDF, 0xF0,
					0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00
				}, {	// Print Icon
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0,
					0x00, 0x00, 0x0F, 0xBB, 0xBB, 0xBB, 0xBB, 0xF0, 0x00, 0xFF, 0xFF, 0xBF, 0xFF, 0xFF, 0xFB,
					0xFF, 0xFF, 0xFB, 0xBB, 0xBF, 0xFF, 0xFF, 0xFB, 0xBB, 0xBF, 0xFB, 0xBB, 0xBB, 0xBB, 0xBB,
					0xBB, 0xBB, 0xBF, 0xFB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBF, 0xFB, 0xBB, 0xBB, 0xBB,
					0xBB, 0xBB, 0xBB, 0xBF, 0xFB, 0xFB, 0xFB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBF, 0xFB, 0xBB, 0xBB,
					0xBB, 0xBB, 0xBB, 0xBB, 0xBF, 0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0xFF, 0x00, 0xFB,
					0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0x00, 0x00,
					0xFB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				}, {	//	Print Preview Icon
					0x0F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
					0x00, 0xFD, 0x8D, 0xFF, 0xFF, 0xCB, 0xBB, 0xBF, 0x00, 0xFF, 0xD8, 0x88, 0x8A, 0xFF, 0xFF,
					0xBF, 0x00, 0x0F, 0xF8, 0x9F, 0xA8, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xF8, 0xFF, 0xF8, 0xFF,
					0xFF, 0xBF, 0x00, 0x00, 0xF8, 0xAF, 0xA8, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFA, 0x88, 0x8A,
					0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFB, 0xFF,
					0xFF, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFB, 0xFF, 0xFF, 0xFB, 0xBB, 0xBF, 0x00, 0x00, 0xFB,
					0xFF, 0xFF, 0xFB, 0xBB, 0xDF, 0x00, 0x00, 0xFB, 0xFF, 0xFF, 0xFB, 0xBD, 0xFF, 0x00, 0x00,
					0xFB, 0xBB, 0xBB, 0xBB, 0xDF, 0xF0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				}, {	//	Print Setup Icon
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
					0x00, 0x00, 0xFB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBF, 0x00, 0x00, 0xFB, 0xFF, 0xBF, 0xFF, 0xFF,
					0xBF, 0x00, 0x00, 0xFB, 0xFF, 0xBF, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFB, 0xBB, 0xBB, 0xBB,
					0xBB, 0xBF, 0x00, 0x00, 0xFB, 0xFF, 0xBF, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFB, 0xFF, 0xBF,
					0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFB, 0xFF, 0xBF, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFB, 0xFF,
					0xBF, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0xFB, 0xFF, 0xBF, 0xFB, 0xBB, 0xBF, 0x00, 0x00, 0xFB,
					0xFF, 0xBF, 0xFB, 0xBB, 0xDF, 0x00, 0x00, 0xFB, 0xFF, 0xBF, 0xFB, 0xBD, 0xFF, 0x00, 0x00,
					0xFB, 0xBB, 0xBB, 0xBB, 0xDF, 0xF0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				}, {	//	Full Screen Icon
					0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
					0xBF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0xFB, 0xFB, 0xBB, 0xBB, 0xBB, 0xFF,
					0xFF, 0xBF, 0xFB, 0xFB, 0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xBF, 0xFB, 0xFB, 0xFF, 0xFF, 0xFB,
					0xFF, 0xFF, 0xBF, 0xFB, 0xFB, 0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xBF, 0xFB, 0xFB, 0xFF, 0xFF,
					0xFB, 0xFF, 0xFF, 0xBF, 0xFB, 0xFB, 0xBB, 0xBB, 0xB9, 0xFF, 0xFF, 0xBF, 0xFB, 0xFB, 0xBB,
					0xBB, 0x9D, 0xDF, 0xDF, 0xBF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFD, 0xBD, 0xBF, 0xBF, 0xFB, 0xFF,
					0xFF, 0xFF, 0xFF, 0xDB, 0xBF, 0xBF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFD, 0xBB, 0xBF, 0xBF, 0xFB,
					0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0xFB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBF,
					0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
				}, {	//	Help
					0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x3C, 0xBB, 0xBB, 0xC3, 0xFF,
					0x00, 0x0F, 0xFD, 0xBB, 0xBB, 0xBB, 0xBB, 0xDF, 0xF0, 0x0F, 0xDB, 0xBB, 0xBF, 0xFB, 0xBB,
					0xBD, 0xF0, 0xF3, 0xBB, 0xBB, 0xBF, 0xFB, 0xBB, 0xBB, 0x3F, 0xFC, 0xBB, 0xBB, 0xBB, 0xBB,
					0xBB, 0xBB, 0xCF, 0xFB, 0xBB, 0xBB, 0xBF, 0xFB, 0xBB, 0xBB, 0xBF, 0xFB, 0xBB, 0xBB, 0xBF,
					0xFF, 0xDB, 0xBB, 0xBF, 0xFB, 0xBB, 0xBB, 0xBD, 0xFF, 0xFB, 0xBB, 0xBF, 0xFB, 0xBB, 0xBB,
					0xBB, 0xBF, 0xFB, 0xBB, 0xBF, 0xFC, 0xBB, 0xBF, 0xFB, 0xBF, 0xFB, 0xBB, 0xCF, 0xF3, 0xBB,
					0xBF, 0xFF, 0xFF, 0xFB, 0xBB, 0x3F, 0x0F, 0xDB, 0xBD, 0xEF, 0xFE, 0xDB, 0xBD, 0xF0, 0x0F,
					0xFD, 0xBB, 0xBB, 0xBB, 0xBB, 0xDF, 0xF0, 0x00, 0xFF, 0x3C, 0xBB, 0xBB, 0xC3, 0xFF, 0x00,
					0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00
				}};
			return data[inIndex];
		}
		// ---------------------------------------------------------------------
		//	getAppIconData
		// ---------------------------------------------------------------------
		virtual const unsigned char	*getAppIconData()
		{
			static const unsigned char	data[] =
			{
				0x28, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x9B, 0x52, 0x00, 0x00,
				0x80, 0x00, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0xC0, 0xC0, 0xC0,
				0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00,
				0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x44,
				0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0xFF, 0xF4, 0x44, 0x44, 0x44,
				0x44, 0x44, 0x4F, 0xFF, 0xFF, 0x44, 0x44, 0x44, 0x44, 0x44, 0x4F, 0xFF, 0xFF, 0x44, 0x44,
				0x44, 0x44, 0x44, 0x4F, 0xFF, 0xFF, 0x44, 0x44, 0x44, 0x44, 0x44, 0xFF, 0xF4, 0xFF, 0xF4,
				0x44, 0x44, 0x44, 0x44, 0xFF, 0xF4, 0xFF, 0xF4, 0x44, 0x44, 0x44, 0x44, 0xFF, 0xF4, 0xFF,
				0xF4, 0x44, 0x44, 0x44, 0x4F, 0xFF, 0x44, 0x4F, 0xFF, 0x44, 0x44, 0x44, 0x4F, 0xFF, 0x44,
				0x4F, 0xFF, 0x44, 0x44, 0x44, 0x4F, 0xFF, 0x44, 0x4F, 0xFF, 0x44, 0x44, 0x44, 0xFF, 0xF4,
				0x44, 0x44, 0xFF, 0xF4, 0x44, 0x44, 0xFF, 0xF4, 0x44, 0x44, 0xFF, 0xF4, 0x44, 0x44, 0xFF,
				0xF4, 0x44, 0x44, 0xFF, 0xF4, 0x44, 0x4F, 0xFF, 0x44, 0x44, 0x44, 0x4F, 0xFF, 0x44, 0x44,
				0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			};
			return data;
		}
	};
};

#endif	// #ifdef VIW_SDI_WINDOW_H

