// =============================================================================
//  BitmapBuffer.hpp
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

