// =============================================================================
//  BitmapBuffer.hpp
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
	\file		viw/model/BitmapBuffer.h
	\author		Dairoku Sekiguchi
	\version	1.0.0
	\date		2015/07/20
	\brief		Header file for viw library types

	This file defines types for viw library
*/

#ifndef VIW_MODEL_BITMAPBUFFER_H
#define VIW_MODEL_BITMAPBUFFER_H

// Includes --------------------------------------------------------------------
#include <Windows.h>
#include <stdio.h>
#include "viw/Exception.hpp"
#include "viw/Model/DisplayBuffer.hpp"
#include "viw/Model/Bitmap.hpp"

// Namespace -------------------------------------------------------------------
namespace viw
{
 namespace model
 {
	// -------------------------------------------------------------------------
	// ImageBuffer class
	// -------------------------------------------------------------------------
	template <typename ImageBufferType> class	BitmapBuffer : public DisplayBuffer<ImageBufferType>
	{
	public:
		// Constructors and Destructor -----------------------------------------
		// ---------------------------------------------------------------------
		// BitmapBuffer
		// ---------------------------------------------------------------------
		BitmapBuffer(bool inThroswEx = false)
			: DisplayBuffer(inThroswEx)
		{
			mBitmap	= NULL;
		}
		// ---------------------------------------------------------------------
		// ~BitmapBuffer
		// ---------------------------------------------------------------------
		virtual ~BitmapBuffer()
		{
			if (mBitmap != NULL)
				delete mBitmap;
		}

		// Member functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// getBitmapInfoPtr
		// ---------------------------------------------------------------------
		const BITMAPINFO	*getBitmapInfoPtr()
		{
			if (updateBitmapInfoPtr() == false)
				return NULL;
				
			return mBitmap->getBitmapInfoPtr();
		}
		// ---------------------------------------------------------------------
		// getBitmapInfoHeaderPtr
		// ---------------------------------------------------------------------
		const BITMAPINFOHEADER	*getBitmapInfoHeaderPtr()
		{
			if (updateBitmapInfoPtr() == false)
				return NULL;

			return mBitmap->getBitmapInfoHeaderPtr();
		}
		// ---------------------------------------------------------------------
		// getBitmapInfoSize
		// ---------------------------------------------------------------------
		size_t	getBitmapInfoSize()
		{
			BITMAPINFOHEADER	*header = getBitmapInfoHeaderPtr();
			if (header == NULL)
				return 0;

			return mBitmap->getBitmapInfoSize();
		}
		// ---------------------------------------------------------------------
		// getColorIndexNum
		// ---------------------------------------------------------------------
		int	getColorIndexNum()
		{
			BITMAPINFOHEADER	*header = getBitmapInfoHeaderPtr();
			if (header == NULL)
				return 0;

			return mBitmap->getColorIndexNum();
		}
		// ---------------------------------------------------------------------
		// getBitmapImageBufPtr
		// ---------------------------------------------------------------------
		const unsigned char	*getBitmapImageBufPtr()
		{
			if (updateBitmapInfoPtr() == false)
				return NULL;

			unsigned char	*imageBufferPtr = allocateDisplayBuffer();
			if (imageBufferPtr == NULL)
				return NULL;

			if (mBitmap->setBitmapBits(imageBufferPtr, getDisplayBufferSize()) == false)
				return NULL;
			
			return imageBufferPtr;
		}
		// ---------------------------------------------------------------------
		// saveToBitmapFile
		// ---------------------------------------------------------------------
		bool	saveToBitmapFile(const char *inFileName)
		{
			const unsigned char	*imageBufferPtr = getBitmapImageBufPtr();
			if (imageBufferPtr == NULL)
				return false;

			return mBitmap->saveToFile(inFileName);
		}
		// ---------------------------------------------------------------------
		// saveToBitmapFile
		// ---------------------------------------------------------------------
		bool	saveToBitmapFile(const LPCTSTR inFileName)
		{
			const unsigned char	*imageBufferPtr = getBitmapImageBufPtr();
			if (imageBufferPtr == NULL)
				return false;

			return mBitmap->saveToFile(inFileName);
		}
		// ---------------------------------------------------------------------
		// loadFromBitmapFile
		// ---------------------------------------------------------------------
		bool	loadFromBitmapFile(const char *inFileName)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// loadFromBitmapFile
		// ---------------------------------------------------------------------
		bool	loadFromBitmapFile(const LPCTSTR inFileName)
		{
			return false;
		}
		// ---------------------------------------------------------------------
		// cloneBitmap
		// ---------------------------------------------------------------------
		Bitmap	*cloneBitmap()
		{
			return NULL;
		}
		// ---------------------------------------------------------------------
		// allocateBitmap
		// ---------------------------------------------------------------------
		bool	allocateBitmap()
		{
			if (mBitmap != NULL)
				return true;

			int	bitCount = obtainBitmapBitCount(mFormat);
			if (bitCount == 0)
			{
				if (mThrowsEx == false)
					return false;
				else
					throw ViwException(ViwException::INVALID_OPERATION_ERROR,
						"unsupported format", VIW_EXCEPTION_LOCATION_MACRO, 0);
			}

			mBitmap = Bitmap::createBitmap(mWidth, mHeight, bitCount, mThrowsEx);
			if (mBitmap == NULL)
				return false;

			return true;
		}
	
		// Static Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// obtainBitmapHeight
		// ---------------------------------------------------------------------
		static int	obtainBitmapHeight(int inHeight, bool inIsBottomUp)
		{
			if (inIsBottomUp == false)
				return -1 * inHeight;

			return inHeight;
		}
		// ---------------------------------------------------------------------
		// obtainBitmapBitCount
		// ---------------------------------------------------------------------
		static int	obtainBitmapBitCount(BufferFormat inFormat)
		{
			switch (inFormat)
			{
				case BUFFER_FORMAT_MONO:
					return 8;
				//case BUFFER_FORMAT_RGB:
				case BUFFER_FORMAT_BGR:
					return 24;
				//case BUFFER_FORMAT_RGBA:
				//case BUFFER_FORMAT_BGRA:
				//	return 4;
			}

			return 0;
		}

	protected:
		// Member variables ----------------------------------------------------
		Bitmap				*mBitmap;

		// Member functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// parameterModified
		// ---------------------------------------------------------------------
		//virtual void	parameterModified()
		//{
		//	ImageBuffer::parameterModified();
		//	updateBitmapInfoPtr();
		//}
		// ---------------------------------------------------------------------
		// updateBitmapInfoPtr
		// ---------------------------------------------------------------------
		const bool	updateBitmapInfoPtr()
		{
			//unsigned char	*displayBuffer = getDisplayBufferPtr();
			//if (displayBuffer == NULL)
			//	return false;

			if (mBitmap == NULL)
			{
				if (allocateBitmap() == false)
					return false;
				return true;
			}

			int	bitCount = obtainBitmapBitCount(mFormat);
			int height = obtainBitmapHeight(mHeight, mIsBottomUp);

			return mBitmap->setBitmapInfo(mWidth, height, bitCount);
		}

	};
 };
};

#endif	// #ifdef VIW_MODEL_BITMAPBUFFER_H

