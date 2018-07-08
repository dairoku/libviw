// =============================================================================
//  Window.hpp
//
//  MIT License
//
//  Copyright (c) 2014-2018 Dairoku Sekiguchi
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
// =============================================================================
/*!
	\file		viw/Window.h
	\author		Dairoku Sekiguchi
	\version	1.0.0
	\date		2014/02/22
	\brief		Header file for viw library types

	This file defines types for viw library
*/

#ifndef VIW_WINDOW_H
#define VIW_WINDOW_H

// Includes --------------------------------------------------------------------
#include <Windows.h>


// Namespace -------------------------------------------------------------------
namespace viw
{
	// -------------------------------------------------------------------------
	// Window class
	// -------------------------------------------------------------------------
	class	Window
	{
	public:
		// Constructors and Destructor -----------------------------------------
		// ---------------------------------------------------------------------
		// ~Window
		// ---------------------------------------------------------------------
		virtual ~Window()
		{
		}

	protected:
		// Member Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// onWM_CREATE
		// ---------------------------------------------------------------------
		virtual bool	onWM_CREATE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_DESTROY
		// ---------------------------------------------------------------------
		virtual bool	onWM_DESTROY(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_MOVE
		// ---------------------------------------------------------------------
		virtual bool	onWM_MOVE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_SIZE
		// ---------------------------------------------------------------------
		virtual bool	onWM_SIZE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_ACTIVATE
		// ---------------------------------------------------------------------
		virtual bool	onWM_ACTIVATE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_SETFOCUS
		// ---------------------------------------------------------------------
		virtual bool	onWM_SETFOCUS(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_KILLFOCUS
		// ---------------------------------------------------------------------
		virtual bool	onWM_KILLFOCUS(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_ENABLE
		// ---------------------------------------------------------------------
		virtual bool	onWM_ENABLE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_PAINT
		// ---------------------------------------------------------------------
		virtual bool	onWM_PAINT(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_CLOSE
		// ---------------------------------------------------------------------
		virtual bool	onWM_CLOSE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_QUIT
		// ---------------------------------------------------------------------
		virtual bool	onWM_QUIT(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_SHOWWINDOW
		// ---------------------------------------------------------------------
		virtual bool	onWM_SHOWWINDOW(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_SETCURSOR
		// ---------------------------------------------------------------------
		virtual bool	onWM_SETCURSOR(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_KEYDOWN
		// ---------------------------------------------------------------------
		virtual bool	onWM_KEYDOWN(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_KEYUP
		// ---------------------------------------------------------------------
		virtual bool	onWM_KEYUP(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_CHAR
		// ---------------------------------------------------------------------
		virtual bool	onWM_CHAR(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_SYSKEYDOWN
		// ---------------------------------------------------------------------
		virtual bool	onWM_SYSKEYDOWN(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_SYSKEYUP
		// ---------------------------------------------------------------------
		virtual bool	onWM_SYSKEYUP(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_SYSCHAR
		// ---------------------------------------------------------------------
		virtual bool	onWM_SYSCHAR(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_COMMAND
		// ---------------------------------------------------------------------
		virtual bool	onWM_COMMAND(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_SYSCOMMAND
		// ---------------------------------------------------------------------
		virtual bool	onWM_SYSCOMMAND(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_TIMER
		// ---------------------------------------------------------------------
		virtual bool	onWM_TIMER(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_HSCROLL
		// ---------------------------------------------------------------------
		virtual bool	onWM_HSCROLL(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_VSCROLL
		// ---------------------------------------------------------------------
		virtual bool	onWM_VSCROLL(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_INITMENU
		// ---------------------------------------------------------------------
		virtual bool	onWM_INITMENU(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_INITMENUPOPUP
		// ---------------------------------------------------------------------
		virtual bool	onWM_INITMENUPOPUP(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_MENUSELECT
		// ---------------------------------------------------------------------
		virtual bool	onWM_MENUSELECT(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_MENUCHAR
		// ---------------------------------------------------------------------
		virtual bool	onWM_MENUCHAR(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_MOUSEMOVE
		// ---------------------------------------------------------------------
		virtual bool	onWM_MOUSEMOVE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_LBUTTONDOWN
		// ---------------------------------------------------------------------
		virtual bool	onWM_LBUTTONDOWN(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_LBUTTONUP
		// ---------------------------------------------------------------------
		virtual bool	onWM_LBUTTONUP(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_LBUTTONDBLCLK
		// ---------------------------------------------------------------------
		virtual bool	onWM_LBUTTONDBLCLK(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_RBUTTONDOWN
		// ---------------------------------------------------------------------
		virtual bool	onWM_RBUTTONDOWN(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_RBUTTONUP
		// ---------------------------------------------------------------------
		virtual bool	onWM_RBUTTONUP(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_RBUTTONDBLCLK
		// ---------------------------------------------------------------------
		virtual bool	onWM_RBUTTONDBLCLK(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_MBUTTONDOWN
		// ---------------------------------------------------------------------
		virtual bool	onWM_MBUTTONDOWN(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_MBUTTONUP
		// ---------------------------------------------------------------------
		virtual bool	onWM_MBUTTONUP(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_MBUTTONDBLCLK
		// ---------------------------------------------------------------------
		virtual bool	onWM_MBUTTONDBLCLK(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_MOUSEWHEEL
		// ---------------------------------------------------------------------
		virtual bool	onWM_MOUSEWHEEL(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_MOUSEHWHEEL
		// ---------------------------------------------------------------------
		virtual bool	onWM_MOUSEHWHEEL(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_DROPFILES
		// ---------------------------------------------------------------------
		virtual bool	onWM_DROPFILES(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_MOUSEHOVER
		// ---------------------------------------------------------------------
		virtual bool	onWM_MOUSEHOVER(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_NCMOUSELEAVE
		// ---------------------------------------------------------------------
		virtual bool	onWM_NCMOUSELEAVE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_MOUSELEAVE
		// ---------------------------------------------------------------------
		virtual bool	onWM_MOUSELEAVE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_HOTKEY
		// ---------------------------------------------------------------------
		virtual bool	onWM_HOTKEY(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// onProcessOtherMessages
		// ---------------------------------------------------------------------
		virtual bool	onProcessOtherMessages(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}

		// Static Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// onWM_CREATE
		// ---------------------------------------------------------------------
		static LRESULT CALLBACK	windowFunc(HWND inHWND, UINT inMessage, WPARAM inWParam, LPARAM inLParam)
		{
			Window		*window;
			LRESULT		result = 0;
			bool		processed = false;

		#ifdef _WIN64
			window = (Window *)::GetWindowLongPtr(inHWND, GWLP_USERDATA);
		#else
			window = (Window *)LongToPtr(GetWindowLongPtr(inHWND, GWLP_USERDATA));
		#endif
			if (window == NULL)
				return DefWindowProc(inHWND, inMessage, inWParam, inLParam);

			switch (inMessage)
			{
				case WM_CREATE:
					processed = window->onWM_CREATE(inMessage, inWParam, inLParam, &result);
					break;
				case WM_DESTROY:
					processed = window->onWM_DESTROY(inMessage, inWParam, inLParam, &result);
					break;
				case WM_MOVE:
					processed = window->onWM_MOVE(inMessage, inWParam, inLParam, &result);
					break;
				case WM_SIZE:
					processed = window->onWM_SIZE(inMessage, inWParam, inLParam, &result);
					break;
				case WM_ACTIVATE:
					processed = window->onWM_ACTIVATE(inMessage, inWParam, inLParam, &result);
					break;
				case WM_SETFOCUS:
					processed = window->onWM_SETFOCUS(inMessage, inWParam, inLParam, &result);
					break;
				case WM_KILLFOCUS:
					processed = window->onWM_KILLFOCUS(inMessage, inWParam, inLParam, &result);
					break;
				case WM_ENABLE:
					processed = window->onWM_ENABLE(inMessage, inWParam, inLParam, &result);
					break;
				case WM_PAINT:
					processed = window->onWM_PAINT(inMessage, inWParam, inLParam, &result);
					break;
				case WM_CLOSE:
					processed = window->onWM_CLOSE(inMessage, inWParam, inLParam, &result);
					break;
				case WM_QUIT:
					processed = window->onWM_QUIT(inMessage, inWParam, inLParam, &result);
					break;
				case WM_SHOWWINDOW:
					processed = window->onWM_SHOWWINDOW(inMessage, inWParam, inLParam, &result);
					break;
				case WM_SETCURSOR:
					processed = window->onWM_SETCURSOR(inMessage, inWParam, inLParam, &result);
					break;
				case WM_KEYDOWN:
					processed = window->onWM_KEYDOWN(inMessage, inWParam, inLParam, &result);
					break;
				case WM_KEYUP:
					processed = window->onWM_KEYUP(inMessage, inWParam, inLParam, &result);
					break;
				case WM_CHAR:
					processed = window->onWM_CHAR(inMessage, inWParam, inLParam, &result);
					break;
				case WM_SYSKEYDOWN:
					processed = window->onWM_SYSKEYDOWN(inMessage, inWParam, inLParam, &result);
					break;
				case WM_SYSKEYUP:
					processed = window->onWM_SYSKEYUP(inMessage, inWParam, inLParam, &result);
					break;
				case WM_SYSCHAR:
					processed = window->onWM_SYSCHAR(inMessage, inWParam, inLParam, &result);
					break;
				case WM_COMMAND:
					processed = window->onWM_COMMAND(inMessage, inWParam, inLParam, &result);
					break;
				case WM_SYSCOMMAND:
					processed = window->onWM_SYSCOMMAND(inMessage, inWParam, inLParam, &result);
					break;
				case WM_TIMER:
					processed = window->onWM_TIMER(inMessage, inWParam, inLParam, &result);
					break;
				case WM_HSCROLL:
					processed = window->onWM_HSCROLL(inMessage, inWParam, inLParam, &result);
					break;
				case WM_VSCROLL:
					processed = window->onWM_VSCROLL(inMessage, inWParam, inLParam, &result);
					break;
				case WM_INITMENU:
					processed = window->onWM_INITMENU(inMessage, inWParam, inLParam, &result);
					break;
				case WM_INITMENUPOPUP:
					processed = window->onWM_INITMENUPOPUP(inMessage, inWParam, inLParam, &result);
					break;
				case WM_MENUSELECT:
					processed = window->onWM_MENUSELECT(inMessage, inWParam, inLParam, &result);
					break;
				case WM_MENUCHAR:
					processed = window->onWM_MENUCHAR(inMessage, inWParam, inLParam, &result);
					break;
				case WM_MOUSEMOVE:
					processed = window->onWM_MOUSEMOVE(inMessage, inWParam, inLParam, &result);
					break;
				case WM_LBUTTONDOWN:
					processed = window->onWM_LBUTTONDOWN(inMessage, inWParam, inLParam, &result);
					break;
				case WM_LBUTTONUP:
					processed = window->onWM_LBUTTONUP(inMessage, inWParam, inLParam, &result);
					break;
				case WM_LBUTTONDBLCLK:
					processed = window->onWM_LBUTTONDBLCLK(inMessage, inWParam, inLParam, &result);
					break;
				case WM_RBUTTONDOWN:
					processed = window->onWM_RBUTTONDOWN(inMessage, inWParam, inLParam, &result);
					break;
				case WM_RBUTTONUP:
					processed = window->onWM_RBUTTONUP(inMessage, inWParam, inLParam, &result);
					break;
				case WM_RBUTTONDBLCLK:
					processed = window->onWM_RBUTTONDBLCLK(inMessage, inWParam, inLParam, &result);
					break;
				case WM_MBUTTONDOWN:
					processed = window->onWM_MBUTTONDOWN(inMessage, inWParam, inLParam, &result);
					break;
				case WM_MBUTTONUP:
					processed = window->onWM_MBUTTONUP(inMessage, inWParam, inLParam, &result);
					break;
				case WM_MBUTTONDBLCLK:
					processed = window->onWM_MBUTTONDBLCLK(inMessage, inWParam, inLParam, &result);
					break;
				case WM_MOUSEWHEEL:
					processed = window->onWM_MOUSEWHEEL(inMessage, inWParam, inLParam, &result);
					break;
				case WM_MOUSEHWHEEL:
					processed = window->onWM_MOUSEHWHEEL(inMessage, inWParam, inLParam, &result);
					break;
				case WM_DROPFILES:
					processed = window->onWM_DROPFILES(inMessage, inWParam, inLParam, &result);
					break;
				case WM_MOUSEHOVER:
					processed = window->onWM_MOUSEHOVER(inMessage, inWParam, inLParam, &result);
					break;
				case WM_NCMOUSELEAVE:
					processed = window->onWM_NCMOUSELEAVE(inMessage, inWParam, inLParam, &result);
					break;
				case WM_MOUSELEAVE:
					processed = window->onWM_MOUSELEAVE(inMessage, inWParam, inLParam, &result);
					break;
				case WM_HOTKEY:
					processed = window->onWM_HOTKEY(inMessage, inWParam, inLParam, &result);
					break;
				default:
					processed = window->onProcessOtherMessages(inMessage, inWParam, inLParam, &result);
					break;
			}

			if (processed == false)
				return DefWindowProc(inHWND, inMessage, inWParam, inLParam);

			return result;
		}
	};
};

#endif	// #ifdef VIW_WINDOW_H

