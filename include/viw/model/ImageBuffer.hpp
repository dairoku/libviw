// =============================================================================
//  ImageBuffer.hpp
//
//  MIT License
//
//  Copyright (c) 2015-2018 Dairoku Sekiguchi
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
	\file		viw/model/ImageBuffer.h
	\author		Dairoku Sekiguchi
	\version	1.0.0
	\date		2015/02/22
	\brief		Header file for viw library types

	This file defines types for viw library
*/

#ifndef VIW_MODEL_IMAGEBUFFER_H
#define VIW_MODEL_IMAGEBUFFER_H

// Includes --------------------------------------------------------------------
#include <Windows.h>
#include <stdio.h>
#include "viw/Exception.hpp"

// Namespace -------------------------------------------------------------------
namespace viw
{
 namespace model
 {
	// -------------------------------------------------------------------------
	// ImageBuffer class
	// -------------------------------------------------------------------------
	template <typename ImageBufferType> class	ImageBuffer
	{
	public:
		// Enum ----------------------------------------------------------------
		enum BufferFormat
		{
			BUFFER_FORMAT_NOT_SPECIFIED				= 0,
			BUFFER_FORMAT_ANY,
			BUFFER_FORMAT_NAITIVE_COLOR,
			BUFFER_FORMAT_NAITIVE_COLOR_ALPHA,

			BUFFER_FORMAT_MONO						= 1024,

			BUFFER_FORMAT_RGB						= 2048,
			BUFFER_FORMAT_RGBA,
			BUFFER_FORMAT_BGR,
			BUFFER_FORMAT_BGRA
		};

		// Constructors and Destructor -----------------------------------------
		// ---------------------------------------------------------------------
		// ImageBuffer
		// ---------------------------------------------------------------------
		ImageBuffer(bool inThroswEx = false)
		{
			mThrowsEx				= inThroswEx;

			mAllocatedImageBuffer	= NULL;
			mExternalImageBuffer	= NULL;

			mFormat					= BUFFER_FORMAT_NOT_SPECIFIED;
			mWidth					= 0;
			mHeight					= 0;
			mOnePixelCount			= 0;
			mImageBufferPixelCount	= 0;
			mImageBufferSize		= 0;
			mIsBottomUp				= false;
			mIsImageModified		= false;
		}
		// ---------------------------------------------------------------------
		// ~ImageBuffer
		// ---------------------------------------------------------------------
		virtual ~ImageBuffer()
		{
			if (mAllocatedImageBuffer != NULL)
				delete mAllocatedImageBuffer;
		}

		// Member functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// updateImageBufferPtr
		// ---------------------------------------------------------------------
		bool	updateImageBufferPtr(ImageBufferType *inImagePtr)
		{
			if (mAllocatedImageBuffer != NULL)
			{
				delete mAllocatedImageBuffer;
				mAllocatedImageBuffer = NULL;
			}

			mExternalImageBuffer = inImagePtr;
			imageBufferModified();
		}
		// ---------------------------------------------------------------------
		// setImageBufferPtr
		// ---------------------------------------------------------------------
		bool	setImageBufferPtr(int inWidth, int inHeight, ImageBufferType *inImagePtr, BufferFormat inFormat, bool inIsBottomUp = false)
		{
			inFormat = checkBufferFormat(inFormat);
			int	onePixelCount = obtainOnePixelCount(inFormat);
			if (onePixelCount == 0)
			{
				if (mThrowsEx == false)
					return false;
				else
					throw ViwException(ViwException::PARAM_ERROR,
					"Invalid BufferFormat ()", VIW_EXCEPTION_LOCATION_MACRO, 0);
			}

			if (mAllocatedImageBuffer != NULL)
			{
				delete mAllocatedImageBuffer;
				mAllocatedImageBuffer = NULL;
			}

			mWidth = inWidth;
			mHeight = inHeight;
			mFormat = inFormat;
			mIsBottomUp = inIsBottomUp;
			mExternalImageBuffer = inImagePtr;
			mOnePixelCount = onePixelCount;
			mImageBufferPixelCount = mWidth * mHeight * mOnePixelCount;
			mImageBufferSize = mImageBufferPixelCount * sizeof(ImageBufferType);

			parameterModified();
			imageBufferModified();
			return true;
		}
		// ---------------------------------------------------------------------
		// allocateImageBuffer
		// ---------------------------------------------------------------------
		bool	allocateImageBuffer(int inWidth, int inHeight, BufferFormat inFormat, bool inIsBottomUp = false)
		{
			inFormat = checkBufferFormat(inFormat);
			int	onePixelCount = obtainOnePixelCount(inFormat);
			if (onePixelCount == 0)
			{
				if (mThrowsEx == false)
					return false;
				else
					throw ViwException(ViwException::PARAM_ERROR,
						"Invalid BufferFormat ()", VIW_EXCEPTION_LOCATION_MACRO, 0);
			}

			if (mAllocatedImageBuffer != NULL)
			{
				if (mWidth == inWidth && mHeight == inHeight && mFormat == inFormat)
					return true;

				delete mAllocatedImageBuffer;
				mAllocatedImageBuffer = NULL;
			}

			mWidth = inWidth;
			mHeight = inHeight;
			mFormat = inFormat;
			mIsBottomUp = inIsBottomUp;
			mOnePixelCount = onePixelCount;
			mImageBufferPixelCount = mWidth * mHeight * mOnePixelCount;
			mImageBufferSize = mImageBufferPixelCount * sizeof(ImageBufferType);
			mExternalImageBuffer = NULL;

			mAllocatedImageBuffer = new ImageBufferType[mImageBufferPixelCount];
			if (mAllocatedImageBuffer == NULL)
			{
				if (mThrowsEx == false)
					return false;
				else
					throw ViwException(ViwException::MEMORY_ERROR,
						"mAllocatedImageBuffer == NULL", VIW_EXCEPTION_LOCATION_MACRO, 0);
			}

			parameterModified();
			return true;
		}
		// ---------------------------------------------------------------------
		// setMonoImageBufferPtr
		// ---------------------------------------------------------------------
		bool	setMonoImageBufferPtr(int inWidth, int inHeight, ImageBufferType *inImagePtr, BufferFormat inFormat, bool inIsBottomUp = false)
		{
			return setImageBufferPtr(inWidth, inHeight, inImagePtr, BUFFER_FORMAT_MONO, inIsBottomUp);
		}
		// ---------------------------------------------------------------------
		// setColorImageBufferPtr
		// ---------------------------------------------------------------------
		bool	setColorImageBufferPtr(int inWidth, int inHeight, ImageBufferType *inImagePtr, BufferFormat inFormat, bool inIsBottomUp = false)
		{
			return setImageBufferPtr(inWidth, inHeight, inImagePtr, BUFFER_FORMAT_NAITIVE_COLOR, inIsBottomUp);
		}
		// ---------------------------------------------------------------------
		// allocateMonoImageBuffer
		// ---------------------------------------------------------------------
		bool	allocateMonoImageBuffer(int inWidth, int inHeight, bool inIsBottomUp = false)
		{
			return allocateImageBuffer(inWidth, inHeight, BUFFER_FORMAT_MONO, inIsBottomUp);
		}
		// ---------------------------------------------------------------------
		// allocateColorImageBuffer
		// ---------------------------------------------------------------------
		bool	allocateColorImageBuffer(int inWidth, int inHeight, bool inIsBottomUp = false)
		{
			return allocateImageBuffer(inWidth, inHeight, BUFFER_FORMAT_NAITIVE_COLOR, inIsBottomUp);
		}
		// ---------------------------------------------------------------------
		// copyIntoImageBuffer
		// ---------------------------------------------------------------------
		bool	copyIntoImageBuffer(int inWidth, int inHeight, const ImageBufferType *inImagePtr, BufferFormat inFormat, bool inIsBottomUp = false)
		{
			if (allocateImageBuffer(inWidth, inHeight, inFormat, inIsBottomUp) == false)
				return false;

			::CopyMemory(mAllocatedImageBuffer, inImagePtr, mImageBufferSize);

			parameterModified();
			imageBufferModified();
			return true;
		}
		// ---------------------------------------------------------------------
		// flipImageBuffer
		// ---------------------------------------------------------------------
		bool	flipImageBuffer()
		{
		}
		// ---------------------------------------------------------------------
		// markAsImageModified
		// ---------------------------------------------------------------------
		void	markAsImageModified()
		{
			mIsImageModified = true;
		}
		// ---------------------------------------------------------------------
		// getBufferFormat
		// ---------------------------------------------------------------------
		BufferFormat	getBufferFormat()
		{
			return mFormat;
		}
		// ---------------------------------------------------------------------
		// getWidth
		// ---------------------------------------------------------------------
		int	getWidth()
		{
			return mWidth;
		}
		// ---------------------------------------------------------------------
		// getHeight
		// ---------------------------------------------------------------------
		int	getHeight()
		{
			return mHeight;
		}
		// ---------------------------------------------------------------------
		// getOnePixelCount
		// ---------------------------------------------------------------------
		int	getOnePixelCount()
		{
			return mOnePixelCount;
		}
		// ---------------------------------------------------------------------
		// getImageBufferPixelCount
		// ---------------------------------------------------------------------
		int	getImageBufferPixelCount()
		{
			return mImageBufferPixelCount;
		}
		// ---------------------------------------------------------------------
		// getImageBufferSize
		// ---------------------------------------------------------------------
		size_t	getImageBufferSize()
		{
			return mImageBufferSize;
		}
		// ---------------------------------------------------------------------
		// isBottomUp
		// ---------------------------------------------------------------------
		bool	isBottomUp()
		{
			return mIsBottomUp;
		}
		// ---------------------------------------------------------------------
		// isImageModified
		// ---------------------------------------------------------------------
		bool	isImageModified()
		{
			return mIsImageModified;
		}
		// ---------------------------------------------------------------------
		// getImageBufferPtr
		// ---------------------------------------------------------------------
		ImageBufferType	*getImageBufferPtr()
		{
			if (mAllocatedImageBuffer == NULL)
				return mExternalImageBuffer;
			return mAllocatedImageBuffer;
		}
		// ---------------------------------------------------------------------
		// getImageBufferLinePtr
		// ---------------------------------------------------------------------
		ImageBufferType	*getImageBufferLinePtr(int inY)
		{
			ImageBufferType	*bufferPtr = getImageBufferPtr();

			bufferPtr += getWidth() * getOnePixelCount() * inY;

			return bufferPtr;
		}

		// Static Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// obtainNativeColorFormat
		// ---------------------------------------------------------------------
		static BufferFormat	obtainNativeColorFormat()
		{
			return BUFFER_FORMAT_BGR;
		}
		// ---------------------------------------------------------------------
		// obtainNativeColorAlphaFormat
		// ---------------------------------------------------------------------
		static BufferFormat	obtainNativeColorAlphaFormat()
		{
			return BUFFER_FORMAT_BGRA;
		}
		// ---------------------------------------------------------------------
		// checkBufferFormat
		// ---------------------------------------------------------------------
		static BufferFormat	checkBufferFormat(BufferFormat inFormat)
		{
			if (inFormat == BUFFER_FORMAT_NAITIVE_COLOR)
				return obtainNativeColorFormat();
			if (inFormat == BUFFER_FORMAT_NAITIVE_COLOR_ALPHA)
				return obtainNativeColorAlphaFormat();
			return inFormat;
		}
		// ---------------------------------------------------------------------
		// obtainOnePixelCount
		// ---------------------------------------------------------------------
		static int	obtainOnePixelCount(BufferFormat inFormat)
		{
			switch (inFormat)
			{
				case BUFFER_FORMAT_MONO:
					return 1;
				case BUFFER_FORMAT_RGB:
				case BUFFER_FORMAT_BGR:
					return 3;
				case BUFFER_FORMAT_RGBA:
				case BUFFER_FORMAT_BGRA:
					return 4;
			}
			return 0;
		}

	protected:
		// Member variables ----------------------------------------------------
		bool				mIsBitmapBitsDirectMapMode;
		ImageBufferType		*mAllocatedImageBuffer;
		ImageBufferType		*mExternalImageBuffer;
		bool				mThrowsEx;

		BufferFormat		mFormat;
		int					mWidth;
		int					mHeight;
		bool				mIsBottomUp;
		int					mOnePixelCount;
		int					mImageBufferPixelCount;
		size_t				mImageBufferSize;

		// Member functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// clearIsImageModifiedFlag
		// ---------------------------------------------------------------------
		void	clearIsImageModifiedFlag()
		{
			mIsImageModified = false;
		}
		// ---------------------------------------------------------------------
		// parameterModified
		// ---------------------------------------------------------------------
		virtual void	parameterModified()
		{
			markAsImageModified();
		}
		// ---------------------------------------------------------------------
		// imageBufferModified
		// ---------------------------------------------------------------------
		virtual void	imageBufferModified()
		{
			markAsImageModified();
		}

	private:
		// Member variables ----------------------------------------------------
		volatile bool		mIsImageModified;
	};
 };
};

#endif	// #ifdef VIW_MODEL_IMAGEBUFFER_H

