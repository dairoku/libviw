// =============================================================================
//  ImageWindow.hpp
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
	\file		viw/ImageWindow.hpp
	\author		Dairoku Sekiguchi
	\version	1.0.0
	\date		2014/03/02
	\brief		Header file for viw library types

	This file defines types for viw library
*/

#ifndef VIW_IMAGE_WINDOW_H
#define VIW_IMAGE_WINDOW_H

// Includes --------------------------------------------------------------------
#include "viw/SDIWindow.hpp"
#include "viw/model/BitmapBuffer.hpp"
#include <math.h>
#include <typeinfo.h>


// Namespace -------------------------------------------------------------------
namespace viw
{
	// -------------------------------------------------------------------------
	// SDIWindow class
	// -------------------------------------------------------------------------
	template <typename ImageBufferType> class	ImageWindow :
			public SDIWindow,
			public model::BitmapBuffer<ImageBufferType>
	{
	public:
		// Constatns -----------------------------------------------------------
		enum CursorMode
		{
			CURSOR_MODE_SCROLL_TOOL	=	1,
			CURSOR_MODE_ZOOM_TOOL,
			CURSOR_MODE_INFO_TOOL
		};

		// Constructors and Destructor -----------------------------------------
		// ---------------------------------------------------------------------
		//	ImageWindow
		// ---------------------------------------------------------------------
		ImageWindow()
			: SDIWindow()
		{
			mArrowCursor			= NULL;
			mScrollCursor			= NULL;
			mZoomPlusCursor			= NULL;
			mZoomMinusCursor		= NULL;
			mInfoCursor				= NULL;
			mPixValueFont			= NULL;

			mCursorMode				= CURSOR_MODE_SCROLL_TOOL;
			mMouseDownMode			= 0;
			mIsMouseDragging		= false;

			::ZeroMemory(&mImageViewRect, sizeof(mImageViewRect));
			::ZeroMemory(&mImageViewSize, sizeof(mImageViewSize));
			::ZeroMemory(&mImageViewOffset, sizeof(mImageViewOffset));
			::ZeroMemory(&mImageViewOffsetStart, sizeof(mImageViewOffsetStart));
			mImageViewScale			= 0;
			mImagePrevScale			= 0;
			mFileNameIndex			= 0;

			mDrawOverlayFunc		= NULL;
			mOverlayFuncData		= NULL;

			mFPSValue				= 0;
			::ZeroMemory(&mFPSData, sizeof(mFPSData));
			mFPSDataCount			= 0;

			mFrequency				= 0;
			mPrevCount				= 0;

			mMutexHandle = ::CreateMutex(NULL, false, NULL);
			if (mMutexHandle == NULL)
				printf("Error: Can't create Mutex object\n");
		}
		// ---------------------------------------------------------------------
		//	~ImageWindow
		// ---------------------------------------------------------------------
		virtual ~ImageWindow()
		{
			if (mMutexHandle != NULL)
				::CloseHandle(mMutexHandle);
		}

		// Member Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// updateImage
		// ---------------------------------------------------------------------
		void	updateImage()
		{
			updateFPS();
			updateImageView();
		}

		// ---------------------------------------------------------------------
		// copyToClipboard
		// ---------------------------------------------------------------------
		bool	copyToClipboard()
		{
			if (getBitmapInfoPtr() == NULL)
			{
				printf("Error: getBitmapInfoPtr() == NULL\n");
				return false;
			}

			/*unsigned char	*buf = createDIB(true);

			::OpenClipboard(mWindowH);
			::EmptyClipboard();
			::SetClipboardData(CF_DIB, buf);
			::CloseClipboard();

			delete buf;
			*/
			return true;
		}
		// ---------------------------------------------------------------------
		// saveImageAs
		// ---------------------------------------------------------------------
		bool	saveImageAs()
		{
			const int	BUF_LEN = 512;
			OPENFILENAME	fileName;
			TCHAR	buf[BUF_LEN] = TEXT("Untitled.bmp");

			::ZeroMemory(&fileName, sizeof(fileName));
			fileName.lStructSize = sizeof(fileName);
			fileName.hwndOwner = mWindowH;
			fileName.lpstrFilter = TEXT("Bitmap File (*.bmp)\0*.bmp\0");
			fileName.nFilterIndex = 1;
			fileName.lpstrFile = buf;
			fileName.nMaxFile = BUF_LEN;
			fileName.lpstrTitle = TEXT("Save Image As");
			//fileName.Flags = OFN_EXTENSIONDIFFERENT;
			fileName.lpstrDefExt = TEXT("bmp");

			if (::GetSaveFileName(&fileName) == 0)
				return false;

			return saveToBitmapFile((const char *)fileName.lpstrFile);
		}
		// ---------------------------------------------------------------------
		// getDispScale
		// ---------------------------------------------------------------------
		double	getViewScale()
		{
			return mImageViewScale;
		}
		// ---------------------------------------------------------------------
		// setViewScale
		// ---------------------------------------------------------------------
		void	setViewScale(double inScale)
		{
			double	prevImageViewScale = mImageViewScale;

			if (inScale <= 1.0)
				inScale = 1.0;
			mImageViewScale = inScale;
			checkImageViewOffset();
			updateStatusBar();
		
			double	scale = mImageViewScale / 100.0;
			if (prevImageViewScale > mImageViewScale &&
				((int )(getWidth() * scale) <= mImageViewRect.right - mImageViewRect.left ||
					(int )(getHeight() * scale) <= mImageViewRect.bottom - mImageViewRect.top))
				updateImageView(true);
			else
				updateImageView();

			updateMouseCursor();
		}
		// ---------------------------------------------------------------------
		// calcWindowSizeFitScale
		// ---------------------------------------------------------------------
		double	calcWindowSizeFitScale()
		{
			double	imageRatio = (double )getHeight() / (double )getWidth();
			double	dispRatio = (double )mImageViewSize.cy / (double )mImageViewSize.cx;
			double	scale;

			if (imageRatio > dispRatio)
				scale = (double )mImageViewSize.cy / (double )getHeight();
			else
				scale = (double )mImageViewSize.cx / (double )getWidth();

			return scale * 100.0;
		}
		// ---------------------------------------------------------------------
		// fitViewScaleToWindowSize
		// ---------------------------------------------------------------------
		void	fitViewScaleToWindowSize()
		{
			setViewScale(calcWindowSizeFitScale());
		}
		// ---------------------------------------------------------------------
		// zoomIn
		// ---------------------------------------------------------------------
		void	zoomIn()
		{
			setViewScale(calcImageScale(IMAGE_ZOOM_STEP));
		}
		// ---------------------------------------------------------------------
		// zoomOut
		// ---------------------------------------------------------------------
		void	zoomOut()
		{
			setViewScale(calcImageScale(-IMAGE_ZOOM_STEP));
		}
		// ---------------------------------------------------------------------
		// isScrollable
		// ---------------------------------------------------------------------
		bool	isScrollable()
		{
				int	imageWidth = (int )(getWidth() * mImageViewScale / 100.0);
				int	imageHeight = (int )(getHeight() * mImageViewScale / 100.0);

				if (imageWidth > mImageViewSize.cx)
					return true;
				if (imageHeight > mImageViewSize.cy)
					return true;

				return false;
		}
		// ---------------------------------------------------------------------
		// setDrawOverlayFunc
		// ---------------------------------------------------------------------
		void	setDrawOverlayFunc(void (*inFunc)(HDC, void *), void *inFuncData)
		{
			mDrawOverlayFunc = inFunc;
			mOverlayFuncData = inFuncData;
		}

	protected:
		// Constatns -----------------------------------------------------------
		const static int	IMAGE_FILE_NAME_BUF_LEN		= 256;
		const static int	IMAGE_STR_BUF_SIZE			= 256;
		const static int	FPS_DATA_NUM				= 25;
		const static int	IMAGE_PALLET_SIZE_8BIT		= 256;
		const static int	IMAGE_ZOOM_STEP				= 1;
		const static int	MOUSE_WHEEL_STEP			= 60;

		enum MenuEventSubID
		{
			SIDM_PROPERTY = 2048,
			SIDM_ZOOMPANE,
			SIDM_FPS,
			SIDM_HISTOGRAM,
			SIDM_FREEZE,
			SIDM_SCROLL_TOOL,
			SIDM_ZOOM_TOOL,
			SIDM_INFO_TOOL,
			SIDM_ZOOM_IN,
			SIDM_ZOOM_OUT,
			SIDM_ACTUAL_SIZE,
			SIDM_FIT_WINDOW,
			SIDM_ADJUST_WINDOW_SIZE
		};

		// Member Variables ----------------------------------------------------
		HCURSOR				mArrowCursor;
		HCURSOR				mScrollCursor;
		HCURSOR				mZoomPlusCursor;
		HCURSOR				mZoomMinusCursor;
		HCURSOR				mInfoCursor;
		HFONT				mPixValueFont;

		int					mCursorMode;
		int					mMouseDownMode;
		bool				mIsMouseDragging;
		POINT				mMouseDownPos;

		HANDLE				mMutexHandle;

		RECT				mImageViewRect;
		SIZE				mImageViewSize;
		SIZE				mImageViewOffset;
		SIZE				mImageViewOffsetStart;
		double				mImageViewScale;
		double				mImagePrevScale;
		int					mFileNameIndex;

		void				(*mDrawOverlayFunc)(HDC, void *);
		void				*mOverlayFuncData;

		double				mFPSValue;
		double				mFPSData[FPS_DATA_NUM];
		int					mFPSDataCount;

		unsigned __int64	mFrequency;
		unsigned __int64	mPrevCount;


		// Member Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		//	onIDM_SAVE
		// ---------------------------------------------------------------------
		virtual bool	onIDM_SAVE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
//			saveImage();
			return true;
		}
		// ---------------------------------------------------------------------
		//	onIDM_SAVE_AS
		// ---------------------------------------------------------------------
		virtual bool	onIDM_SAVE_AS(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			saveImageAs();
			return true;
		}
		// ---------------------------------------------------------------------
		//	onIDM_COPY
		// ---------------------------------------------------------------------
		virtual bool	onIDM_COPY(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			copyToClipboard();
			return true;
		}
		// ---------------------------------------------------------------------
		//	onSIDM_PROPERTY
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_PROPERTY(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			//dumpBitmapInfo();
			return true;
		}
		// ---------------------------------------------------------------------
		//	onSIDM_ZOOMPANE
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_ZOOMPANE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onSIDM_FPS
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_FPS(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onSIDM_HISTOGRAM
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_HISTOGRAM(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onSIDM_FREEZE
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_FREEZE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		//	onSIDM_SCROLL_TOOL
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_SCROLL_TOOL(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			setCursorMode(CURSOR_MODE_SCROLL_TOOL);
			return true;
		}
		// ---------------------------------------------------------------------
		//	onSIDM_ZOOM_TOOL
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_ZOOM_TOOL(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			setCursorMode(CURSOR_MODE_ZOOM_TOOL);
			return true;
		}
		// ---------------------------------------------------------------------
		//	onSIDM_INFO_TOOL
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_INFO_TOOL(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			setCursorMode(CURSOR_MODE_INFO_TOOL);
			return true;
		}
		// ---------------------------------------------------------------------
		//	onSIDM_ZOOM_IN
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_ZOOM_IN(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			zoomIn();
			if (::GetKeyState(VK_CONTROL) < 0)
				updateWindowSize(true);
			return true;
		}
		// ---------------------------------------------------------------------
		//	onSIDM_ZOOM_OUT
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_ZOOM_OUT(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			zoomOut();
			if (::GetKeyState(VK_CONTROL) < 0)
				updateWindowSize(true);
			return true;
		}
		// ---------------------------------------------------------------------
		//	onSIDM_ACTUAL_SIZE
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_ACTUAL_SIZE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			if (::GetKeyState(VK_CONTROL) < 0 || isFullScreenMode() == true)
				setViewScale(100);
			else
			{
				updateWindowSize();
				updateImageView();
			}
			return true;
		}
		// ---------------------------------------------------------------------
		//	onSIDM_FIT_WINDOW
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_FIT_WINDOW(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			fitViewScaleToWindowSize();
			return true;
		}
		// ---------------------------------------------------------------------
		//	onIDM_ADJUST_WINDOW_SIZE
		// ---------------------------------------------------------------------
		virtual bool	onSIDM_ADJUST_WINDOW_SIZE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			if (isFullScreenMode() == false)
				updateWindowSize(true);
			return true;
		}
		// ---------------------------------------------------------------------
		// onWM_SIZE
		// ---------------------------------------------------------------------
		virtual bool	onWM_SIZE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			SDIWindow::onWM_SIZE(inMessage, inWParam, inLParam, outResult);

			updateImageViewRect();
			if (::GetKeyState(VK_SHIFT) < 0)
			{
				fitViewScaleToWindowSize();
				return true;
			}

			if (checkImageViewOffset())
				updateImageView();

			updateStatusBar();
			return true;
		}
		// ---------------------------------------------------------------------
		// onWM_PAINT
		// ---------------------------------------------------------------------
		virtual bool	onWM_PAINT(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			if (getBitmapInfoPtr() == NULL)
				return true;

			DWORD result = ::WaitForSingleObject(mMutexHandle, INFINITE);
			if (result != WAIT_OBJECT_0)
				return true;

			PAINTSTRUCT	paintstruct;
			HDC	hdc = ::BeginPaint(mWindowH, &paintstruct);
			drawImage(hdc);
			ReleaseMutex(mMutexHandle);
			EndPaint(mWindowH, &paintstruct);
			return true;
		}
		// ---------------------------------------------------------------------
		// onWM_SETCURSOR
		// ---------------------------------------------------------------------
		virtual bool	onWM_SETCURSOR(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return updateMouseCursor();
		}
		// ---------------------------------------------------------------------
		// onWM_KEYDOWN
		// ---------------------------------------------------------------------
		virtual bool	onWM_KEYDOWN(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			switch (inWParam)
			{
				case VK_SHIFT:
					updateMouseCursor();
					break;
			}
			return true;
		}
		// ---------------------------------------------------------------------
		// onWM_KEYUP
		// ---------------------------------------------------------------------
		virtual bool	onWM_KEYUP(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			switch (inWParam)
			{
				case VK_SHIFT:
					updateMouseCursor();
					break;
			}
			return true;
		}
		// ---------------------------------------------------------------------
		// onWM_CHAR
		// ---------------------------------------------------------------------
		virtual bool	onWM_CHAR(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			switch (inWParam)
			{
				case 's':
				case 'S':
//					saveBitmapFile();
					return true;
				case 'd':
				case 'D':
//					dumpBitmapInfo();
					return true;
				case '+':
					zoomIn();
					return true;
				case '-':
					zoomOut();
					return true;
				case '=':
					setViewScale(100);
					return true;
			}

			if (SDIWindow::onWM_CHAR(inMessage, inWParam, inLParam, outResult))
				return true;

			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_COMMAND
		// ---------------------------------------------------------------------
		virtual bool	onWM_COMMAND(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			switch (LOWORD(inWParam))
			{
				// IDM part
				case SDIWindow::IDM_SAVE:
					return onIDM_SAVE(inMessage, inWParam, inLParam, outResult);
				case SDIWindow::IDM_SAVE_AS:
					return onIDM_SAVE_AS(inMessage, inWParam, inLParam, outResult);
				case SDIWindow::IDM_COPY:
					return onIDM_COPY(inMessage, inWParam, inLParam, outResult);
				// SIDM part
				case SIDM_PROPERTY:
					return onSIDM_PROPERTY(inMessage, inWParam, inLParam, outResult);
				case SIDM_ZOOMPANE:
					return onSIDM_ZOOMPANE(inMessage, inWParam, inLParam, outResult);
				case SIDM_FPS:
					return onSIDM_FPS(inMessage, inWParam, inLParam, outResult);
				case SIDM_HISTOGRAM:
					return onSIDM_HISTOGRAM(inMessage, inWParam, inLParam, outResult);
				case SIDM_FREEZE:
					return onSIDM_FREEZE(inMessage, inWParam, inLParam, outResult);
				case SIDM_SCROLL_TOOL:
					return onSIDM_SCROLL_TOOL(inMessage, inWParam, inLParam, outResult);
				case SIDM_ZOOM_TOOL:
					return onSIDM_ZOOM_TOOL(inMessage, inWParam, inLParam, outResult);
				case SIDM_INFO_TOOL:
					return onSIDM_INFO_TOOL(inMessage, inWParam, inLParam, outResult);
				case SIDM_ZOOM_IN:
					return onSIDM_ZOOM_IN(inMessage, inWParam, inLParam, outResult);
				case SIDM_ZOOM_OUT:
					return onSIDM_ZOOM_OUT(inMessage, inWParam, inLParam, outResult);
				case SIDM_ACTUAL_SIZE:
					return onSIDM_ACTUAL_SIZE(inMessage, inWParam, inLParam, outResult);
				case SIDM_FIT_WINDOW:
					return onSIDM_FIT_WINDOW(inMessage, inWParam, inLParam, outResult);
				case SIDM_ADJUST_WINDOW_SIZE:
					return onSIDM_ADJUST_WINDOW_SIZE(inMessage, inWParam, inLParam, outResult);
			}

			if (SDIWindow::onWM_COMMAND(inMessage, inWParam, inLParam, outResult))
				return true;

			return false;
		}
		// ---------------------------------------------------------------------
		// onWM_MOUSEMOVE
		// ---------------------------------------------------------------------
		virtual bool	onWM_MOUSEMOVE(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			if (mIsMouseDragging == false)
				return false;

			POINT	currentPos;
			double	scale = mImageViewScale / 100.0;

			currentPos.x = (short )LOWORD(inLParam);
			currentPos.y = (short )HIWORD(inLParam);

			switch (mMouseDownMode)
			{
				case CURSOR_MODE_SCROLL_TOOL:
					mImageViewOffset = mImageViewOffsetStart;
					mImageViewOffset.cx -= (int )((currentPos.x - mMouseDownPos.x) / scale);
					mImageViewOffset.cy -= (int )((currentPos.y - mMouseDownPos.y) / scale);
					checkImageViewOffset();
					updateImageView();
					break;
				case CURSOR_MODE_ZOOM_TOOL:
					break;
				case CURSOR_MODE_INFO_TOOL:
					break;
			}

			return true;
		}
		// ---------------------------------------------------------------------
		// onWM_LBUTTONDOWN
		// ---------------------------------------------------------------------
		virtual bool	onWM_LBUTTONDOWN(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			POINT	localPos;
			int	x, y;
			double	scale;

			mMouseDownPos.x = (short )LOWORD(inLParam);
			mMouseDownPos.y = (short )HIWORD(inLParam);

			if ((inWParam & MK_CONTROL) == 0)
				mMouseDownMode = mCursorMode;
			else
				mMouseDownMode = CURSOR_MODE_SCROLL_TOOL;

			switch (mMouseDownMode)
			{
				case CURSOR_MODE_SCROLL_TOOL:
					mIsMouseDragging = true;
					mImageViewOffsetStart = mImageViewOffset;
					::SetCapture(mWindowH);
					break;
				case CURSOR_MODE_ZOOM_TOOL:
					localPos = mMouseDownPos;
					localPos.x -= mImageViewRect.left;
					localPos.y -= mImageViewRect.top;
					scale = mImageViewScale / 100.0;
					x = (int )(localPos.x / scale);
					y = (int )(localPos.y / scale);
					x += mImageViewOffset.cx;
					y += mImageViewOffset.cy;

					if ((inWParam & MK_SHIFT) == 0)
						scale = calcImageScale(IMAGE_ZOOM_STEP);
					else
						scale = calcImageScale(-IMAGE_ZOOM_STEP);

					mImageViewOffset.cx = x - (int )(localPos.x / (scale / 100.0));
					mImageViewOffset.cy = y - (int )(localPos.y / (scale / 100.0));
					setViewScale(scale);
					break;
				case CURSOR_MODE_INFO_TOOL:
					localPos = mMouseDownPos;
					localPos.x -= mImageViewRect.left;
					localPos.y -= mImageViewRect.top;
					scale = mImageViewScale / 100.0;
					x = (int )(localPos.x / scale);
					y = (int )(localPos.y / scale);
					x += mImageViewOffset.cx;
					y += mImageViewOffset.cy;

/*					int	v = mBitmapBits[getWidth() * y + x];
					char	buf[256];

					_itoa(v, buf, 2);
					printf("X:%.4d Y:%.4d VALUE:%.3d %.2X %s\n", x, y, v, v, buf);*/
					break;
			}
			return true;
		}
		// ---------------------------------------------------------------------
		// onWM_LBUTTONUP
		// ---------------------------------------------------------------------
		virtual bool	onWM_LBUTTONUP(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			switch (mMouseDownMode)
			{
				case CURSOR_MODE_SCROLL_TOOL:
					mIsMouseDragging = false;
					::ReleaseCapture();
					break;
				case CURSOR_MODE_ZOOM_TOOL:
					break;
				case CURSOR_MODE_INFO_TOOL:
					break;
			}
			return true;
		}
		// ---------------------------------------------------------------------
		// onWM_LBUTTONDBLCLK
		// ---------------------------------------------------------------------
		virtual bool	onWM_LBUTTONDBLCLK(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return onWM_LBUTTONDOWN(inMessage, inWParam, inLParam, outResult);
		}
		// ---------------------------------------------------------------------
		// onWM_MBUTTONDOWN
		// ---------------------------------------------------------------------
		virtual bool	onWM_MBUTTONDOWN(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			POINT	localPos;
			int	x, y;
			double	scale;

			localPos.x = (short )LOWORD(inLParam);
			localPos.y = (short )HIWORD(inLParam);

			localPos.x -= mImageViewRect.left;
			localPos.y -= mImageViewRect.top;
			scale = mImageViewScale / 100.0;
			x = (int )(localPos.x / scale);
			y = (int )(localPos.y / scale);
			x += mImageViewOffset.cx;
			y += mImageViewOffset.cy;

			if ((inWParam & (MK_SHIFT + MK_CONTROL)) == 0)
				scale = 100;
			else
			{
				if ((inWParam & MK_SHIFT) == 0)
					scale = 3000;
				else
					scale = calcWindowSizeFitScale();
			}

			if (mImageViewScale == scale)
				scale = mImagePrevScale;

			mImagePrevScale = mImageViewScale;

			mImageViewOffset.cx = x - (int )(localPos.x / (scale / 100.0));
			mImageViewOffset.cy = y - (int )(localPos.y / (scale / 100.0));
			setViewScale(scale);

			return true;
		}
		// ---------------------------------------------------------------------
		// onWM_MBUTTONDBLCLK
		// ---------------------------------------------------------------------
		virtual bool	onWM_MBUTTONDBLCLK(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			return onWM_MBUTTONDOWN(inMessage, inWParam, inLParam, outResult);
		}
		// ---------------------------------------------------------------------
		// onWM_MOUSEWHEEL
		// ---------------------------------------------------------------------
		virtual bool	onWM_MOUSEWHEEL(UINT inMessage, WPARAM inWParam, LPARAM inLParam, LRESULT *outResult)
		{
			POINT	mousePos;
			int	x, y, zDelta;
			double	scale;

			mousePos.x = (short )LOWORD(inLParam);
			mousePos.y = (short )HIWORD(inLParam);
			ScreenToClient(mWindowH, &mousePos);
			if (PtInRect(&mImageViewRect, mousePos) == 0)
				return false;

			zDelta = GET_WHEEL_DELTA_WPARAM(inWParam);

			mousePos.x -= mImageViewRect.left;
			mousePos.y -= mImageViewRect.top;
			scale = mImageViewScale / 100.0;
			x = (int )(mousePos.x / scale);
			y = (int )(mousePos.y / scale);
			x += mImageViewOffset.cx;
			y += mImageViewOffset.cy;

			if ((inWParam & (MK_SHIFT + MK_CONTROL)) == 0)
				scale = calcImageScale(zDelta / MOUSE_WHEEL_STEP);
			else
			{
				if ((inWParam & MK_SHIFT) == 0)
					scale = calcImageScale(zDelta / MOUSE_WHEEL_STEP * 4);
				else
					scale = calcImageScale(zDelta / MOUSE_WHEEL_STEP / 2);
			}

			mImageViewOffset.cx = x - (int )(mousePos.x / (scale / 100.0));
			mImageViewOffset.cy = y - (int )(mousePos.y / (scale / 100.0));
			setViewScale(scale);

			return true;
		}
		// ---------------------------------------------------------------------
		// updateWindowSize
		// ---------------------------------------------------------------------
		void	updateWindowSize(bool inKeepViewScale = false)
		{
			if (mWindowState != WINDOW_OPEN_STATE || getBitmapInfoPtr() == NULL)
				return;

			RECT	rect;
			int		imageWidth, imageHeight;

			//mImageSize.cx = mBitmapInfo->biWidth;
			//mImageSize.cy = abs(mBitmapInfo->biHeight);

			if (inKeepViewScale == false)
			{
				mImageViewScale = 100.0;
				imageWidth = getWidth();
				imageHeight = getHeight();
			}
			else
			{
				imageWidth = (int)(getWidth() * mImageViewScale / 100.0);
				imageHeight = (int)(getHeight() * mImageViewScale / 100.0);
			}

			int	displayWidth = ::GetSystemMetrics(SM_CXSCREEN);
			int	displayHEIGHT = ::GetSystemMetrics(SM_CYSCREEN);

			if (imageWidth > displayWidth)
				imageWidth = displayWidth;
			if (imageHeight > displayHEIGHT)
				imageHeight = displayHEIGHT;

			rect.left = 0;
			rect.top = 0;
			rect.right = imageWidth;
			rect.bottom = imageHeight;
			mImageViewRect = getViewRect(&rect);

			mImageViewSize.cx = mImageViewRect.right - mImageViewRect.left;
			mImageViewSize.cy = mImageViewRect.bottom - mImageViewRect.top;
			mImageViewOffset.cx = 0;
			mImageViewOffset.cy = 0;

			::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, TRUE);	// if menu is enabled
			::SetWindowPos(mWindowH, NULL, 0, 0,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOMOVE | SWP_NOOWNERZORDER);

			updateStatusBar();
		}
		// ---------------------------------------------------------------------
		// checkImageViewOffset
		// ---------------------------------------------------------------------
		bool	checkImageViewOffset()
		{
			int	limit;
			double	scale = mImageViewScale / 100.0;
			bool	update = false;
			SIZE	prevOffset = mImageViewOffset;

			limit = (int)(getWidth() - mImageViewSize.cx / scale);
			if (mImageViewOffset.cx > limit)
			{
				mImageViewOffset.cx = limit;
				update = true;
			}
			if (mImageViewOffset.cx < 0)
			{
				mImageViewOffset.cx = 0;
				update = true;
			}
			limit = (int)(getHeight() - mImageViewSize.cy / scale);
			if (mImageViewOffset.cy > limit)
			{
				mImageViewOffset.cy = limit;
				update = true;
			}
			if (mImageViewOffset.cy < 0)
			{
				mImageViewOffset.cy = 0;
				update = true;
			}

			if (prevOffset.cx == mImageViewOffset.cx &&
				prevOffset.cy == mImageViewOffset.cy)
				update = false;

			return update;
		}
		// ---------------------------------------------------------------------
		// updateMouseCursor
		// ---------------------------------------------------------------------
		bool	updateMouseCursor()
		{
			if (updateMousePixelReadout() == false)
				return false;

			switch (mCursorMode)
			{
				case CURSOR_MODE_SCROLL_TOOL:
					if (isScrollable())
						::SetCursor(mScrollCursor);
					else
						::SetCursor(mArrowCursor);
					break;
				case CURSOR_MODE_ZOOM_TOOL:
					if (::GetKeyState(VK_SHIFT) < 0)
						::SetCursor(mZoomMinusCursor);
					else
						::SetCursor(mZoomPlusCursor);
					break;
				case CURSOR_MODE_INFO_TOOL:
					::SetCursor(mInfoCursor);
					break;
			}

			return true;
		}
		// ---------------------------------------------------------------------
		// updateMouseCursor
		// ---------------------------------------------------------------------
		virtual void	setCursorMode(int inCursorMode)
		{
			mCursorMode = inCursorMode;

			if (mMenuH == NULL)
				return;

			switch (mCursorMode)
			{
				case CURSOR_MODE_SCROLL_TOOL:
					::CheckMenuItem(mMenuH, SIDM_SCROLL_TOOL, MF_CHECKED);
					::CheckMenuItem(mMenuH, SIDM_ZOOM_TOOL, MF_UNCHECKED);
					::CheckMenuItem(mMenuH, SIDM_INFO_TOOL, MF_UNCHECKED);
					break;
				case CURSOR_MODE_ZOOM_TOOL:
					::CheckMenuItem(mMenuH, SIDM_SCROLL_TOOL, MF_UNCHECKED);
					::CheckMenuItem(mMenuH, SIDM_ZOOM_TOOL, MF_CHECKED);
					::CheckMenuItem(mMenuH, SIDM_INFO_TOOL, MF_UNCHECKED);
					break;
				case CURSOR_MODE_INFO_TOOL:
					::CheckMenuItem(mMenuH, SIDM_SCROLL_TOOL, MF_UNCHECKED);
					::CheckMenuItem(mMenuH, SIDM_ZOOM_TOOL, MF_UNCHECKED);
					::CheckMenuItem(mMenuH, SIDM_INFO_TOOL, MF_CHECKED);
					break;
			}
		}
		// ---------------------------------------------------------------------
		// updateImageView
		// ---------------------------------------------------------------------
		void	updateImageView(bool inErase = false)
		{
			if (mWindowState != WINDOW_OPEN_STATE)
				return;
			::InvalidateRect(mWindowH, &mImageViewRect, inErase);
		}
		// ---------------------------------------------------------------------
		// calcImageScale
		// ---------------------------------------------------------------------
		double	calcImageScale(int inStep)
		{
			double	val, scale;

			val = log10(mImageViewScale) + inStep / 100.0;
			scale = pow(10, val);

			// 100% snap & 1% limit (just in case...)
			if (fabs(scale - 100.0) <= 1.0)
				scale = 100;
			if (scale <= 1.0)
				scale = 1.0;

			return scale;
		}
		// ---------------------------------------------------------------------
		// initFPS
		// ---------------------------------------------------------------------
		void	initFPS()
		{
			::QueryPerformanceFrequency((LARGE_INTEGER *)&mFrequency);
			::QueryPerformanceCounter((LARGE_INTEGER *)&mPrevCount);

			mFPSValue = 0;
			mFPSDataCount = 0;
		}
		// ---------------------------------------------------------------------
		// updateFPS
		// ---------------------------------------------------------------------
		void	updateFPS()
		{
			if (mWindowState != WINDOW_OPEN_STATE || getBitmapInfoPtr() == NULL)
				return;
		
			unsigned __int64	currentCount;
			::QueryPerformanceCounter((LARGE_INTEGER *)&currentCount);

			mFPSValue = 1.0 * (double )mFrequency / (double )(currentCount - mPrevCount);
			mPrevCount = currentCount;
		
			if (mFPSDataCount < FPS_DATA_NUM)
				mFPSDataCount++;
			else
				::MoveMemory(mFPSData, &(mFPSData[1]), sizeof(double) * (FPS_DATA_NUM - 1));	
			mFPSData[mFPSDataCount - 1] = mFPSValue;

			double	averageValue = 0;
			for (int i = 0; i < mFPSDataCount; i++)
				averageValue += mFPSData[i];
			averageValue /= mFPSDataCount;

	#ifdef _UNICODE
			wchar_t	buf[IMAGE_STR_BUF_SIZE];

			swprintf_s(buf, IMAGE_STR_BUF_SIZE, TEXT("FPS: %.1f (avg.=%.1f)"), mFPSValue, averageValue);
			SendMessage(mStatusbarH, SB_SETTEXT, (WPARAM )3, (LPARAM )buf);
	#else
			char	buf[IMAGE_STR_BUF_SIZE];

			sprintf_s(buf, IMAGE_STR_BUF_SIZE, TEXT("FPS: %.1f (avg.=%.1f)"), mFPSValue, averageValue);
			SendMessage(mStatusbarH, SB_SETTEXT, (WPARAM )3, (LPARAM )buf);
	#endif
		}
		// ---------------------------------------------------------------------
		// getPixelPointer
		// ---------------------------------------------------------------------
		unsigned char	*getPixelPointer(int inX, int inY)
		{
/*			if (inX < 0 || inX >= getWidth() ||
				inY < 0 || inY >= getHeight())
				return NULL;

			if (mBitmapInfo->biBitCount == 8)
				return &(mBitmapBits[getWidth() * inY + inX]);

			int	lineSize = getWidth() * 3;
			if (lineSize % 4 != 0)
				lineSize = lineSize + (4 - (lineSize % 4));

			if (mBitmapInfo->biHeight < 0)	// Topdown-up DIB
				return &(mBitmapBits[lineSize * inY+ (inX * 3)]);

			return &(mBitmapBits[lineSize * (getHeight() - inY - 1)+ (inX * 3)]);
*/
			return 0;
		}
		// ---------------------------------------------------------------------
		// updateMousePixelReadout
		// ---------------------------------------------------------------------
		bool	updateMousePixelReadout()
		{
			POINT	pos;
			int	x, y;
			bool	result;

			::GetCursorPos(&pos);
			::ScreenToClient(mWindowH, &pos);
			if (::PtInRect(&mImageViewRect, pos) == 0)
				result = false;
			else
				result = true;

			pos.x -= mImageViewRect.left;
			pos.y -= mImageViewRect.top;
			x = (int )(pos.x / (mImageViewScale / 100.0));
			y = (int )(pos.y / (mImageViewScale / 100.0));
			x += mImageViewOffset.cx;
			y += mImageViewOffset.cy;

			unsigned char	*pixelPtr = getPixelPointer(x, y);

	#ifdef _UNICODE
			wchar_t	buf[IMAGE_STR_BUF_SIZE];

			if (result == false || pixelPtr == NULL)
				swprintf_s(buf, IMAGE_STR_BUF_SIZE, TEXT(""));
			else
			{
/*				if (mBitmapInfo->biBitCount == 8)
					swprintf_s(buf, IMAGE_STR_BUF_SIZE, TEXT("%03d (%d,%d)"), pixelPtr[0], x, y);
				else
					swprintf_s(buf, IMAGE_STR_BUF_SIZE, TEXT("%03d %03d %03d (%d,%d)"),
								(int )pixelPtr[2], (int )pixelPtr[1], (int )pixelPtr[0], x, y);
*/			}

			SendMessage(mStatusbarH, SB_SETTEXT, (WPARAM )2, (LPARAM )buf);
	#else
			char	buf[IMAGE_STR_BUF_SIZE];

			if (result == false || pixelPtr == NULL)
				sprintf_s(buf, IMAGE_STR_BUF_SIZE, TEXT(""));
			else
			{
				if (mBitmapInfo->biBitCount == 8)
					sprintf_s(buf, IMAGE_STR_BUF_SIZE, TEXT("%03d (%d,%d)"), pixelPtr[0], x, y);
				else
					sprintf_s(buf, IMAGE_STR_BUF_SIZE, TEXT("%03d %03d %03d (%d,%d)"),
								(int )pixelPtr[2], (int )pixelPtr[1], (int )pixelPtr[0], x, y);
			}
			SendMessage(mStatusbarH, SB_SETTEXT, (WPARAM )2, (LPARAM )buf);
	#endif

			return result;
		}
		// ---------------------------------------------------------------------
		// updateImageViewRect
		// ---------------------------------------------------------------------
		void	updateImageViewRect()
		{
			if (mWindowState != WINDOW_OPEN_STATE || getBitmapInfoPtr() == NULL)
				return;

			RECT	rect = getViewRect();

			mImageViewRect = rect;
			mImageViewSize.cx = mImageViewRect.right - mImageViewRect.left;
			mImageViewSize.cy = mImageViewRect.bottom - mImageViewRect.top;
		}
		// ---------------------------------------------------------------------
		// updateStatusBar
		// ---------------------------------------------------------------------
		void	updateStatusBar()
		{
			RECT	rect;
			::GetClientRect(mWindowH, &rect);

			int	statusbarSize[] = {100, 300, rect.right - 200, rect.right};
			::SendMessage(mStatusbarH, SB_SETPARTS, (WPARAM )4, (LPARAM )(LPINT)statusbarSize);

		#ifdef _UNICODE
			wchar_t	buf[IMAGE_STR_BUF_SIZE];

			swprintf_s(buf, IMAGE_STR_BUF_SIZE, TEXT("Zoom: %.0f%%"), mImageViewScale);
			::SendMessage(mStatusbarH, SB_SETTEXT, (WPARAM )0, (LPARAM )buf);

			/*if (mBitmapInfo != NULL)
			{
				swprintf_s(buf, IMAGE_STR_BUF_SIZE,
					TEXT("Image: %dx%dx%d"),
					mBitmapInfo->biWidth, abs(mBitmapInfo->biHeight), mBitmapInfo->biBitCount);
				::SendMessage(mStatusbarH, SB_SETTEXT, (WPARAM )1, (LPARAM )buf);
			}*/

			swprintf_s(buf, IMAGE_STR_BUF_SIZE, TEXT("FPS: %.1f"), mFPSValue);
			::SendMessage(mStatusbarH, SB_SETTEXT, (WPARAM )3, (LPARAM )buf);
		#else
			char	buf[ZO_IMAGE_STR_BUF_SIZE];

			sprintf_s(buf, ZO_IMAGE_STR_BUF_SIZE, TEXT("Zoom: %.0f%%"), mImageViewScale);
			::SendMessage(mStatusbarH, SB_SETTEXT, (WPARAM )0, (LPARAM )buf);

			if (mBitmapInfo != NULL)
			{
				sprintf_s(buf, ZO_IMAGE_STR_BUF_SIZE,
					TEXT("Image: %dx%dx%d"),
					mBitmapInfo->biWidth, abs(mBitmapInfo->biHeight), mBitmapInfo->biBitCount);
				::SendMessage(mStatusbarH, SB_SETTEXT, (WPARAM )1, (LPARAM )buf);
			}

			sprintf_s(buf, ZO_IMAGE_STR_BUF_SIZE, TEXT("FPS: %.1f"), mFPSValue);
			::SendMessage(mStatusbarH, SB_SETTEXT, (WPARAM )3, (LPARAM )buf);
		#endif
		}

		// ---------------------------------------------------------------------
		// drawImage
		// ---------------------------------------------------------------------
		void	drawImage(HDC inHDC)
		{
			if (mImageViewScale == 100)
			{
				::SetDIBitsToDevice(inHDC,
					mImageViewRect.left, mImageViewRect.top,
					getWidth(), getHeight(),
					mImageViewOffset.cx, -1 * mImageViewOffset.cy,
					0, getHeight(),
					getBitmapImageBufPtr(), getBitmapInfoPtr(), DIB_RGB_COLORS);
			}
			else
			{
				double	scale = (mImageViewScale / 100.0);
				::SetStretchBltMode(inHDC, COLORONCOLOR);
				::StretchDIBits(inHDC,
					mImageViewRect.left, mImageViewRect.top,
					(int)(getWidth() * scale),
					(int)(getHeight() * scale),
					mImageViewOffset.cx, -1 * mImageViewOffset.cy,
					getWidth(), getHeight(),
					getBitmapImageBufPtr(), getBitmapInfoPtr(), DIB_RGB_COLORS, SRCCOPY);

				/*if (mImageViewScale >= 3000 && mBitmapInfo->biBitCount == 8)
				{
					int	numX = (int )ceil((mImageViewRect.right - mImageViewRect.left) / scale);
					int	numY = (int )ceil((mImageViewRect.bottom - mImageViewRect.top) / scale);
					int	x, y;

					HGDIOBJ	prevFont = ::SelectObject(inHDC, mPixValueFont);
					::SetBkMode(inHDC, TRANSPARENT);
					for (y = 0; y < numY; y++)
					{
						unsigned char	*pixelPtr = getPixelPointer(mImageViewOffset.cx, mImageViewOffset.cy + y);
						for (x = 0; x < numX; x++)
						{
							wchar_t	buf[IMAGE_STR_BUF_SIZE];
							unsigned char	pixelValue = pixelPtr[x];

							swprintf_s(buf, IMAGE_STR_BUF_SIZE, TEXT("%03d"), pixelValue);

							if (pixelValue > 0x80)
								::SetTextColor(inHDC, RGB(0x00, 0x00, 0x00));
							else
								::SetTextColor(inHDC, RGB(0xFF, 0xFF, 0xFF));
							::TextOut(inHDC,
								(int )((double )x * scale + scale * 0.5 - 10) + mImageViewRect.left,
								(int )((double )y * scale + scale * 0.5 - 5) + mImageViewRect.top,
								buf, 3);
						}
					}
					::SelectObject(inHDC, prevFont);*/
				}
			if (mDrawOverlayFunc != NULL)
				mDrawOverlayFunc(inHDC, mOverlayFuncData);
		}
		// ---------------------------------------------------------------------
		//	initBeforeCreateWindow
		// ---------------------------------------------------------------------
		virtual int	initBeforeCreateWindow()
		{
			mImageViewScale = 100;
			mImagePrevScale = 100;
			initFPS();
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

			mPixValueFont = ::CreateFont(
				10, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, ANSI_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				ANTIALIASED_QUALITY | PROOF_QUALITY,
				//PROOF_QUALITY,
				FIXED_PITCH | FF_MODERN,
				TEXT("Lucida Console"));
				//TEXT("Courier"));

			updateWindowSize();

			return -1;	// success (Win32 style result)
		}
		// ---------------------------------------------------------------------
		//	initMenu
		// ---------------------------------------------------------------------
		virtual void	initMenu()
		{
			mMenuH = ::CreateMenu();
			mPopupMenuH = ::CreateMenu();
			HMENU	popupSubMenuH = ::CreateMenu();
			HMENU	fileMenuH = ::CreateMenu();
			HMENU	editMenuH = ::CreateMenu();
			HMENU	viewMenuH = ::CreateMenu();
			HMENU	zoomMenuH = ::CreateMenu();
			HMENU	windowMenuH = ::CreateMenu();
			HMENU	helpMenuH = ::CreateMenu();

			::AppendMenu(mMenuH, MF_POPUP, (UINT_PTR )fileMenuH, TEXT("&File"));
			::AppendMenu(mMenuH, MF_POPUP, (UINT_PTR )editMenuH, TEXT("&Edit"));
			::AppendMenu(mMenuH, MF_POPUP, (UINT_PTR )viewMenuH, TEXT("&View"));
			::AppendMenu(mMenuH, MF_POPUP, (UINT_PTR )zoomMenuH, TEXT("&Zoom"));
			::AppendMenu(mMenuH, MF_POPUP, (UINT_PTR )windowMenuH, TEXT("&Window"));
			::AppendMenu(mMenuH, MF_POPUP, (UINT_PTR )helpMenuH, TEXT("&Help"));

			::AppendMenu(mPopupMenuH, MF_POPUP, (UINT_PTR )popupSubMenuH, TEXT("&RightClickMenu"));
			::AppendMenu(popupSubMenuH, MF_POPUP, (UINT_PTR )fileMenuH, TEXT("&File"));
			::AppendMenu(popupSubMenuH, MF_POPUP, (UINT_PTR )editMenuH, TEXT("&Edit"));
			::AppendMenu(popupSubMenuH, MF_POPUP, (UINT_PTR )viewMenuH, TEXT("&View"));
			::AppendMenu(popupSubMenuH, MF_POPUP, (UINT_PTR )zoomMenuH, TEXT("&Zoom"));
			::AppendMenu(popupSubMenuH, MF_POPUP, (UINT_PTR )windowMenuH, TEXT("&Window"));
			::AppendMenu(popupSubMenuH, MF_POPUP, (UINT_PTR )helpMenuH, TEXT("&Help"));

			::AppendMenu(fileMenuH, MF_ENABLED, SDIWindow::IDM_SAVE, TEXT("&Save Image"));
			::AppendMenu(fileMenuH, MF_ENABLED, SDIWindow::IDM_SAVE_AS, TEXT("Save Image &As..."));
			::AppendMenu(fileMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(fileMenuH, MF_ENABLED, SIDM_PROPERTY, TEXT("&Property"));
			::AppendMenu(fileMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(fileMenuH, MF_ENABLED, SDIWindow::IDM_CLOSE, TEXT("&Close"));

			::AppendMenu(editMenuH, MF_GRAYED, SDIWindow::IDM_UNDO, TEXT("&Undo"));
			::AppendMenu(editMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(editMenuH, MF_GRAYED, SDIWindow::IDM_CUT, TEXT("Cu&t"));
			::AppendMenu(editMenuH, MF_ENABLED, SDIWindow::IDM_COPY, TEXT("&Copy"));
			::AppendMenu(editMenuH, MF_GRAYED, SDIWindow::IDM_PASTE, TEXT("&Paste"));

			::AppendMenu(viewMenuH, MF_ENABLED, SDIWindow::IDM_MENUBAR, TEXT("&Menubar"));
			::AppendMenu(viewMenuH, MF_ENABLED, SDIWindow::IDM_TOOLBAR, TEXT("&Toolbar"));
			::AppendMenu(viewMenuH, MF_ENABLED, SDIWindow::IDM_STATUSBAR, TEXT("&Status Bar"));
			::AppendMenu(viewMenuH, MF_GRAYED, SIDM_ZOOMPANE, TEXT("&Zoom &Pane"));
			::AppendMenu(viewMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(viewMenuH, MF_ENABLED, SDIWindow::IDM_FULL_SCREEN, TEXT("&Full Screen"));
			::AppendMenu(viewMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(viewMenuH, MF_ENABLED, SIDM_FPS, TEXT("FPS"));
			::AppendMenu(viewMenuH, MF_GRAYED, SIDM_HISTOGRAM, TEXT("Histogram"));
			::AppendMenu(viewMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(viewMenuH, MF_GRAYED, SIDM_FREEZE, TEXT("Freeze"));
			::AppendMenu(viewMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(viewMenuH, MF_ENABLED, SIDM_SCROLL_TOOL, TEXT("Scroll Tool"));
			::AppendMenu(viewMenuH, MF_ENABLED, SIDM_ZOOM_TOOL, TEXT("Zoom Tool"));
			::AppendMenu(viewMenuH, MF_ENABLED, SIDM_INFO_TOOL, TEXT("Info Tool"));

			::AppendMenu(zoomMenuH, MF_ENABLED, SIDM_ZOOM_IN, TEXT("Zoom In"));
			::AppendMenu(zoomMenuH, MF_ENABLED, SIDM_ZOOM_OUT, TEXT("Zoom Out"));
			::AppendMenu(zoomMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(zoomMenuH, MF_ENABLED, SIDM_ACTUAL_SIZE, TEXT("Actual Size"));
			::AppendMenu(zoomMenuH, MF_ENABLED, SIDM_FIT_WINDOW, TEXT("Fit to Window"));
			::AppendMenu(zoomMenuH, MF_SEPARATOR, 0, NULL);
			::AppendMenu(zoomMenuH, MF_ENABLED, SIDM_ADJUST_WINDOW_SIZE, TEXT("Adjust Window Size"));

			::AppendMenu(windowMenuH, MF_GRAYED, SDIWindow::IDM_CASCADE_WINDOW, TEXT("&Cascade"));
			::AppendMenu(windowMenuH, MF_GRAYED, SDIWindow::IDM_TILE_WINDOW, TEXT("&Tile"));

			::AppendMenu(helpMenuH, MF_ENABLED, SDIWindow::IDM_ABOUT, TEXT("&About"));

			setCursorMode(mCursorMode);

			::CheckMenuItem(mMenuH, IDM_MENUBAR, MF_CHECKED);
			::CheckMenuItem(mMenuH, IDM_TOOLBAR, MF_CHECKED);
			::CheckMenuItem(mMenuH, IDM_STATUSBAR, MF_CHECKED);
		}
		// ---------------------------------------------------------------------
		//	initCursor
		// ---------------------------------------------------------------------
		virtual void	initCursor()
		{
			static const unsigned char andPlane[3][128] = {{	// Scroll
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
			},{	//	Zoom Plus
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
			},{	//	Zoom Minus
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
			}};

			static const unsigned char xorPlane[3][128] = {{	//	Scroll
				0x01, 0x80, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x05, 0xA0, 0x00,
				0x00, 0x01, 0x80, 0x00, 0x00, 0x31, 0x8C, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0xFD, 0xBF,
				0x00, 0x00, 0xFD, 0xBF, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x31, 0x8C, 0x00, 0x00, 0x01,
				0x80, 0x00, 0x00, 0x05, 0xA0, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00,
				0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			},{	//	Zoom Plus
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x10, 0x40, 0x00,
				0x00, 0x22, 0x20, 0x00, 0x00, 0x22, 0x20, 0x00, 0x00, 0x2F, 0xA0, 0x00, 0x00, 0x22, 0x20,
				0x00, 0x00, 0x22, 0x20, 0x00, 0x00, 0x10, 0x60, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x00,
				0x38, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			},{	//	Zoom Minus
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x10, 0x40, 0x00,
				0x00, 0x20, 0x20, 0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x2F, 0xA0, 0x00, 0x00, 0x20, 0x20,
				0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x10, 0x60, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x00,
				0x38, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			}};

			//	When we want to use color cursor, we will have to use
			//	CreateIconIndirect() or CreateIconFromResorceEx()
			mScrollCursor = ::CreateCursor(mModuleH, 0, 0, 32, 32, andPlane[0], xorPlane[0]);
			mArrowCursor =:: LoadCursor(NULL, IDC_ARROW);
			mZoomPlusCursor = ::CreateCursor(mModuleH, 0, 0, 32, 32, andPlane[1], xorPlane[1]);
			mZoomMinusCursor = ::CreateCursor(mModuleH, 0, 0, 32, 32, andPlane[2], xorPlane[2]);
			mInfoCursor = ::LoadCursor(NULL, IDC_CROSS);
		}
		// ---------------------------------------------------------------------
		//	getToolbarData
		// ---------------------------------------------------------------------
		virtual void	getToolbarData(int *outButtnNum,  const TBBUTTON **outTBUTTON)
		{
			static const TBBUTTON tbb[16] = {
				// iBitmap, idCommand, fsState, fsStyle, bReserved[2], dwData, iString
				{ MAKELONG(0, 0), SDIWindow::IDM_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(1, 0), SDIWindow::IDM_COPY, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG( 0, 0), 0,				TBSTATE_ENABLED, TBSTYLE_SEP, {0, 0}, 0, 0},
				{ MAKELONG(2, 0), SIDM_SCROLL_TOOL, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(3, 0), SIDM_ZOOM_TOOL, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(4, 0), SIDM_INFO_TOOL, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG( 0, 0), 0,				TBSTATE_ENABLED, TBSTYLE_SEP, {0, 0}, 0, 0},
				{ MAKELONG(5, 0), SDIWindow::IDM_FULL_SCREEN, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG( 0, 0), 0,				TBSTATE_ENABLED, TBSTYLE_SEP, {0, 0}, 0, 0},
				{ MAKELONG( 6, 0), SIDM_FREEZE,		TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, {0, 0}, 0, 0},
				{MAKELONG( 0, 0), 0,				TBSTATE_ENABLED, TBSTYLE_SEP, {0, 0}, 0, 0},
				{ MAKELONG(7, 0), SIDM_ZOOM_IN, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(8, 0), SIDM_ZOOM_OUT, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(9, 0), SIDM_ACTUAL_SIZE, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(10, 0), SIDM_FIT_WINDOW, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
				{ MAKELONG(11, 0), SIDM_ADJUST_WINDOW_SIZE, TBSTATE_ENABLED, TBSTYLE_BUTTON | BTNS_AUTOSIZE, { 0, 0 }, 0, 0 },
			};
			
			*outButtnNum = 16;
			*outTBUTTON = tbb;
		}
		// ---------------------------------------------------------------------
		//	getToolbarBitmapData
		// ---------------------------------------------------------------------
		virtual const unsigned char	*getToolbarBitmapData(int inIndex)
		{
			static const unsigned char data[13][128] = { {	// Save Icon
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
					0xF0, 0x0F, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x0F, 0x88, 0x88, 0x88, 0x88, 0x88,
					0x88, 0xF0, 0x0F, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x0F, 0x88, 0x88, 0x88, 0x88,
					0x88, 0x88, 0xF0, 0x0F, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x0F, 0x88, 0x88, 0x88,
					0x88, 0x88, 0x88, 0xF0, 0x0F, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x0F, 0x88, 0x8F,
					0xFF, 0xFF, 0xF8, 0x88, 0xF0, 0x0F, 0x88, 0x8F, 0xFF, 0xFF, 0xF8, 0x88, 0xF0, 0x0F, 0x88,
					0x8F, 0xFF, 0x88, 0xF8, 0x88, 0xF0, 0x0F, 0x88, 0x8F, 0xFF, 0x88, 0xF8, 0x8D, 0xF0, 0x0F,
					0x88, 0x8F, 0xFF, 0x88, 0xF8, 0xDF, 0xF0, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				}, {		// Copy Icon
					0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x0F, 0xBB, 0xBB, 0xBB, 0xBB, 0xF0, 0x00,
					0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xF0, 0x00, 0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xF0,
					0x00, 0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xF0, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB,
					0xFB, 0xBB, 0xF0, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xFF, 0xFB, 0xF0, 0x0F, 0xBF, 0xFF, 0xFF,
					0xFB, 0xFF, 0xFB, 0xF0, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xFF, 0xFB, 0xF0, 0x0F, 0xBF, 0xFF,
					0xFF, 0xFB, 0xFF, 0xFB, 0xF0, 0x0F, 0xBB, 0xBB, 0xBB, 0xBB, 0xFF, 0xFB, 0xF0, 0x0F, 0xFF,
					0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xF0, 0x00, 0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xF0, 0x00,
					0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFB, 0xF0, 0x00, 0x00, 0x0F, 0xBB, 0xBB, 0xBB, 0xBB, 0xF0,
					0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0
				}, {	//	Scroll Icon (Translate Icon)
					0x00, 0x00, 0x0F, 0xFD, 0xDF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xDB, 0xBD, 0xFF, 0x00,
					0x00, 0x00, 0x00, 0xFD, 0xBB, 0xBB, 0xDF, 0x00, 0x00, 0x00, 0x00, 0xFB, 0xDB, 0xBD, 0xBF,
					0x00, 0x00, 0x0F, 0xFF, 0xFD, 0xFB, 0xBF, 0xDF, 0xFF, 0xF0, 0xFF, 0xDB, 0xDF, 0xFB, 0xBF,
					0xFD, 0xBD, 0xFF, 0xFD, 0xBD, 0xFF, 0xFF, 0xFF, 0xFF, 0xDB, 0xDF, 0xDB, 0xBB, 0xBB, 0xFB,
					0xBF, 0xBB, 0xBB, 0xBD, 0xDB, 0xBB, 0xBB, 0xFB, 0xBF, 0xBB, 0xBB, 0xBD, 0xFD, 0xBD, 0xFF,
					0xFF, 0xFF, 0xFF, 0xDB, 0xDF, 0xFF, 0xDB, 0xDF, 0xFB, 0xBF, 0xFD, 0xBD, 0xFF, 0x0F, 0xFF,
					0xFD, 0xFB, 0xBF, 0xDF, 0xFF, 0xF0, 0x00, 0x00, 0xFB, 0xDB, 0xBD, 0xBF, 0x00, 0x00, 0x00,
					0x00, 0xFD, 0xBB, 0xBB, 0xDF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xDB, 0xBD, 0xFF, 0x00, 0x00,
					0x00, 0x00, 0x0F, 0xFD, 0xDF, 0xF0, 0x00, 0x00
				}, {	//	Zoom Icon
					0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x3C,
					0xEF, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF3, 0xBB, 0xCF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x3B,
					0xBB, 0x9F, 0x00, 0x0F, 0xFF, 0xFF, 0xF3, 0xBB, 0xB9, 0xFF, 0x00, 0xFE, 0xCB, 0xBB, 0xDB,
					0xBB, 0x9F, 0xF0, 0x0F, 0xEB, 0x9F, 0xFF, 0x9B, 0xB9, 0xFF, 0x00, 0x0F, 0xC9, 0xFF, 0xFF,
					0xF9, 0xCF, 0xF0, 0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFF, 0xBF, 0xF0, 0x00, 0x0F, 0xBF, 0xFF,
					0xFF, 0xFF, 0xBE, 0xF0, 0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFF, 0xBF, 0xF0, 0x00, 0x0F, 0xC9,
					0xFF, 0xFF, 0xF9, 0xCF, 0x00, 0x00, 0x0F, 0xEB, 0x9F, 0xFF, 0x9B, 0xEF, 0x00, 0x00, 0x00,
					0xFE, 0xCB, 0xBB, 0xC3, 0xF0, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				}, {	//	Cross Icon
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB0, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x0B, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB0, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB0,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB0, 0x00, 0x00, 0x00, 0x0B, 0xBB, 0xBB, 0xBB,
					0xBB, 0xBB, 0xBB, 0xB0, 0x0B, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xB0, 0x00, 0x00, 0x00,
					0x0B, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x0B, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB0, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x0B, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB0, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				}, {	//	Full Icon
					0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
					0xBF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0xFB, 0xFB, 0xBB, 0xBB, 0xBB, 0xFF,
					0xFF, 0xBF, 0xFB, 0xFB, 0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xBF, 0xFB, 0xFB, 0xFF, 0xFF, 0xFB,
					0xFF, 0xFF, 0xBF, 0xFB, 0xFB, 0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xBF, 0xFB, 0xFB, 0xFF, 0xFF,
					0xFB, 0xFF, 0xFF, 0xBF, 0xFB, 0xFB, 0xBB, 0xBB, 0xB9, 0xFF, 0xFF, 0xBF, 0xFB, 0xFB, 0xBB,
					0xBB, 0x9D, 0xDF, 0xDF, 0xBF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFD, 0xBD, 0xBF, 0xBF, 0xFB, 0xFF,
					0xFF, 0xFF, 0xFF, 0xDB, 0xBF, 0xBF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFD, 0xBB, 0xBF, 0xBF, 0xFB,
					0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0xFB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBF,
					0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
				}, {	//	Pause Icon
					0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x3C, 0xBB, 0xBB, 0xC3, 0xFF,
					0x00, 0x0F, 0xFD, 0xBB, 0xBB, 0xBB, 0xBB, 0xDF, 0xF0, 0x0F, 0xDB, 0xBB, 0xBB, 0xBB, 0xBB,
					0xBD, 0xF0, 0xF3, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0x3F, 0xFC, 0xBB, 0xBF, 0xFB, 0xBF,
					0xFB, 0xBB, 0xCF, 0xFB, 0xBB, 0xBF, 0xFB, 0xBF, 0xFB, 0xBB, 0xBF, 0xFB, 0xBB, 0xBF, 0xFB,
					0xBF, 0xFB, 0xBB, 0xBF, 0xFB, 0xBB, 0xBF, 0xFB, 0xBF, 0xFB, 0xBB, 0xBF, 0xFB, 0xBB, 0xBF,
					0xFB, 0xBF, 0xFB, 0xBB, 0xBF, 0xFC, 0xBB, 0xBF, 0xFB, 0xBF, 0xFB, 0xBB, 0xCF, 0xF3, 0xBB,
					0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0x3F, 0x0F, 0xDB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBD, 0xF0, 0x0F,
					0xFD, 0xBB, 0xBB, 0xBB, 0xBB, 0xDF, 0xF0, 0x00, 0xFF, 0x3C, 0xBB, 0xBB, 0xC3, 0xFF, 0x00,
					0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00
				}, {	// Zoom In Icon
					0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x3C,
					0xEF, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF3, 0xBB, 0xCF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x3B,
					0xBB, 0x9F, 0x00, 0x0F, 0xFF, 0xFF, 0xF3, 0xBB, 0xB9, 0xFF, 0x00, 0xFE, 0xCB, 0xBB, 0xDB,
					0xBB, 0x9F, 0xF0, 0x0F, 0xEB, 0x9F, 0xFF, 0x9B, 0xB9, 0xFF, 0x00, 0x0F, 0xC9, 0xFF, 0x8F,
					0xF9, 0xCF, 0xF0, 0x00, 0x0F, 0xBF, 0xFF, 0x8F, 0xFF, 0xBF, 0xF0, 0x00, 0x0F, 0xBF, 0x88,
					0x88, 0x8F, 0xBE, 0xF0, 0x00, 0x0F, 0xBF, 0xFF, 0x8F, 0xFF, 0xBF, 0xF0, 0x00, 0x0F, 0xC9,
					0xFF, 0x8F, 0xF9, 0xCF, 0x00, 0x00, 0x0F, 0xEB, 0x9F, 0xFF, 0x9B, 0xEF, 0x00, 0x00, 0x00,
					0xFE, 0xCB, 0xBB, 0xC3, 0xF0, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				}, {	//	Zoom Out Icon
					0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x3C,
					0xEF, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF3, 0xBB, 0xCF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x3B,
					0xBB, 0x9F, 0x00, 0x0F, 0xFF, 0xFF, 0xF3, 0xBB, 0xB9, 0xFF, 0x00, 0xFE, 0xCB, 0xBB, 0xDB,
					0xBB, 0x9F, 0xF0, 0x0F, 0xEB, 0x9F, 0xFF, 0x9B, 0xB9, 0xFF, 0x00, 0x0F, 0xC9, 0xFF, 0xFF,
					0xF9, 0xCF, 0xF0, 0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFF, 0xBF, 0xF0, 0x00, 0x0F, 0xBF, 0x88,
					0x88, 0x8F, 0xBF, 0xF0, 0x00, 0x0F, 0xBF, 0xFF, 0xFF, 0xFF, 0xBF, 0xF0, 0x00, 0x0F, 0xC9,
					0xFF, 0xFF, 0xF9, 0xCF, 0x00, 0x00, 0x0F, 0xEB, 0x9F, 0xFF, 0x9B, 0xEF, 0x00, 0x00, 0x00,
					0xFE, 0xCB, 0xBB, 0xC3, 0xF0, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				}, {	//	Actual Size Icon
					0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x3C,
					0xEF, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF3, 0xBB, 0xCF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x3B,
					0xBB, 0x9F, 0x00, 0x0F, 0xFF, 0xFF, 0xF3, 0xBB, 0xB9, 0xFF, 0x00, 0xFE, 0xCB, 0xBB, 0xDB,
					0xBB, 0x9F, 0xF0, 0x0F, 0xEB, 0x9F, 0xFF, 0x9B, 0xB9, 0xFF, 0x00, 0x0F, 0xCB, 0xBB, 0xBB,
					0xBB, 0xBB, 0xBB, 0xBB, 0x0F, 0xBB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0x0F, 0xBB, 0xFB,
					0xFF, 0xBF, 0xFF, 0xBF, 0xFB, 0x0F, 0xBB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0x0F, 0xCB,
					0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0x0F, 0xEB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0x00,
					0xFB, 0xFB, 0xFF, 0xBF, 0xFF, 0xBF, 0xFB, 0x00, 0x0B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB,
					0x00, 0x0B, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB
				}, {	//	Fit to Window Icon
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
					0xFF, 0x00, 0x00, 0xFB, 0xBB, 0xBB, 0xBB, 0xFD, 0xDF, 0x00, 0x00, 0xFB, 0xBB, 0xBB, 0xB9,
					0xDB, 0xDF, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0xBD, 0xFF, 0x0F, 0xBB, 0xFF, 0xDB, 0xBB,
					0xCB, 0xD9, 0xBF, 0x0F, 0xBB, 0xFD, 0xCE, 0xFE, 0xCC, 0xFB, 0xBF, 0x0F, 0xBB, 0xFB, 0xEF,
					0xBF, 0xEB, 0xFB, 0xBF, 0x0F, 0xBB, 0xFB, 0xFB, 0xBB, 0xFB, 0xFB, 0xBF, 0x0F, 0xBB, 0xFB,
					0xEF, 0xBF, 0xEB, 0xFB, 0xBF, 0x0F, 0xBB, 0xFD, 0xCE, 0xFE, 0xCD, 0xFB, 0xBF, 0x0F, 0xBB,
					0xFF, 0xDB, 0xBB, 0xDF, 0xFB, 0xBF, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
					0x00, 0xFB, 0xBB, 0xBB, 0xBB, 0xF0, 0x00, 0x00, 0x00, 0xFB, 0xBB, 0xBB, 0xBB, 0xF0, 0x00,
					0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00
				}, {	//	Adjust Window Icon
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF,
					0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDD, 0xF0, 0xFB, 0xBB, 0xBB, 0xBB, 0xBB, 0x9D,
					0xBD, 0xF0, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xDB, 0xDB, 0xF0, 0xFB, 0xFF, 0xFD, 0xBB, 0xBC,
					0xBD, 0x9B, 0xF0, 0xFB, 0xFF, 0xDC, 0xEF, 0xEC, 0xCF, 0xFB, 0xF0, 0xFB, 0xFF, 0xBE, 0xFB,
					0xFE, 0xBF, 0xFB, 0xF0, 0xFB, 0xFF, 0xBF, 0xBB, 0xBF, 0xBF, 0xFB, 0xF0, 0xFB, 0xFF, 0xBE,
					0xFB, 0xFE, 0xBF, 0xFB, 0xF0, 0xFB, 0xFF, 0xDC, 0xEF, 0xEC, 0xDF, 0xFB, 0xF0, 0xFB, 0xFF,
					0xFD, 0xBB, 0xBD, 0xFF, 0xFB, 0xF0, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xF0, 0xFB,
					0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xF0, 0xFB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xF0,
					0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0
				} };
			return data[inIndex];
		}
	};
};

#endif	// #ifdef VIW_IMAGE_WINDOW_H

