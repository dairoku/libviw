// =============================================================================
//  DisplayBuffer.hpp
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
	\file		viw/model/DisplayBuffer.h
	\author		Dairoku Sekiguchi
	\version	1.0.0
	\date		2015/07/20
	\brief		Header file for viw library types

	This file defines types for viw library
*/

#ifndef VIW_MODEL_DISPLAYBUFFER_H
#define VIW_MODEL_DISPLAYBUFFER_H

// Includes --------------------------------------------------------------------
#include <Windows.h>
#include <stdio.h>
#include "viw/Exception.hpp"
#include "viw/Model/ImageBuffer.hpp"

// Namespace -------------------------------------------------------------------
namespace viw
{
 namespace model
 {
	// -------------------------------------------------------------------------
	// DisplayBuffer class
	// -------------------------------------------------------------------------
	 template <typename ImageBufferType> class	DisplayBuffer : public ImageBuffer<ImageBufferType>
	{
	public:
		// Enum ----------------------------------------------------------------
		enum DisplayMapMode
		{
			DISPLAY_MAP_NOT_SPECIFIED					= 0,
			DISPLAY_MAP_ANY,
			DISPLAY_MAP_NONE,
			DISPLAY_MAP_DIRECT,

			DISPLAY_MAP_PARTIAL							= 1024,

			DISPLAY_MAP_LUT_1D							= 2048,

			DISPLAY_MAP_LUT_3D							= 4096
		};

		// Constructors and Destructor -----------------------------------------
		// ---------------------------------------------------------------------
		// DisplayImageBuffer
		// ---------------------------------------------------------------------
		DisplayBuffer(bool inThroswEx = false)
			: ImageBuffer(inThroswEx)
		{

			if (typeid(ImageBufferType) == typeid(unsigned char))
			{
				mMapMode = DISPLAY_MAP_NONE;
				mUseParentBuffer = true;
			}
			else
			{
				mMapMode = DISPLAY_MAP_NOT_SPECIFIED;
				mUseParentBuffer = false;
			}

			mDisplayBuffer = NULL;

			mDisplayFormat = BUFFER_FORMAT_NOT_SPECIFIED;
			mDisplayWidth = 0;
			mDisplayHeight = 0;
			mDisplayIsBottomUp = false;

			mIsBufferUpdateNeeded = false;
		}
		// ---------------------------------------------------------------------
		// ~DisplayBuffer
		// ---------------------------------------------------------------------
		virtual ~DisplayBuffer()
		{
			if (mDisplayBuffer != NULL)
				delete mDisplayBuffer;
		}

		// Member functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// updateDisplayBuffer
		// ---------------------------------------------------------------------
		virtual void	updateDisplayBuffer()
		{
			if (mUseParentBuffer == true)
				return;

			if (mDisplayBuffer == NULL)
				return;

			switch (mMapMode)
			{
				case DISPLAY_MAP_DIRECT:
				default:	// DISPLAY_MAP_DIRECT
					displayMapDirect();
					break;
			}

			clearIsImageModifiedFlag();
			clearIsBufferUpdateNeededFlag();
		}
		// ---------------------------------------------------------------------
		// getDisplayBufferPtr
		// ---------------------------------------------------------------------
		const unsigned char	*getDisplayBufferPtr()
		{	
			return allocateDisplayBuffer();
		}
		// ---------------------------------------------------------------------
		// getDisplayBufferSize
		// ---------------------------------------------------------------------
		size_t	getDisplayBufferSize()
		{
			if (mUseParentBuffer == true)
				return getImageBufferSize();

			return mDisplayBufferSize;
		}
		// ---------------------------------------------------------------------
		// getDisplayMapMode
		// ---------------------------------------------------------------------
		DisplayMapMode	getDisplayMapMode()
		{
			return mMapMode;
		}
		// ---------------------------------------------------------------------
		// setDisplayMapMode
		// ---------------------------------------------------------------------
		bool	setDisplayMapMode(DisplayMapMode inMapMode)
		{
			return false;
		}

		// ---------------------------------------------------------------------
		// setAsBufferUpdateNeeded
		// ---------------------------------------------------------------------
		void	setAsBufferUpdateNeeded()
		{
			mIsBufferUpdateNeeded = true;
		}

		// ---------------------------------------------------------------------
		// clearIsBufferUpdateNeededFlag
		// ---------------------------------------------------------------------
		void	clearIsBufferUpdateNeededFlag()
		{
			mIsBufferUpdateNeeded = false;
		}

		// ---------------------------------------------------------------------
		// isBufferUpdateNeeded
		// ---------------------------------------------------------------------
		bool	isBufferUpdateNeeded()
		{
			return mIsBufferUpdateNeeded;
		}

		// Static Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// obtainNativeColorFormat
		// ---------------------------------------------------------------------
		static BufferFormat	obtainNativeColorFormat()
		{
		}


	protected:
		// Member variables ----------------------------------------------------
		DisplayMapMode		mMapMode;

		bool				mUseParentBuffer;
		unsigned char		*mDisplayBuffer;

		BufferFormat		mDisplayFormat;
		int					mDisplayWidth;
		int					mDisplayHeight;
		bool				mDisplayIsBottomUp;
		size_t				mDisplayBufferSize;

		bool				mIsBufferUpdateNeeded;

		// Member functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// allocateDisplayBuffer
		// ---------------------------------------------------------------------
		unsigned char	*allocateDisplayBuffer()
		{
			if (mUseParentBuffer == true)
				return getImageBufferPtr();

			if (mAllocatedImageBuffer == NULL && mExternalImageBuffer == NULL)
				return NULL;

			//if (mWidth != mDisplayWidth && mHeight != mDisplayHeight && mFormat != mDisplayFormat)
			if (mWidth != mDisplayWidth || mHeight != mDisplayHeight || mFormat != mDisplayFormat)
			{
				if (mDisplayBuffer != NULL)
					delete mDisplayBuffer;

				mMapMode = DISPLAY_MAP_DIRECT;
				mDisplayWidth = mWidth;
				mDisplayHeight = mHeight;
				mDisplayFormat = mFormat;
				mDisplayBufferSize = mImageBufferPixelCount * sizeof(unsigned char);

				mDisplayBuffer = new unsigned char[mImageBufferPixelCount];
				if (mDisplayBuffer == NULL)
				{
					if (mThrowsEx == false)
						return false;
					else
						throw ViwException(ViwException::MEMORY_ERROR,
						"mDisplayBuffer == NULL", VIW_EXCEPTION_LOCATION_MACRO, 0);
				}
				setAsBufferUpdateNeeded();
			}

			if (isBufferUpdateNeeded() || isImageModified())
				updateDisplayBuffer();

			return mDisplayBuffer;
		}
		// ---------------------------------------------------------------------
		// displayMapDirect
		// ---------------------------------------------------------------------
		void	displayMapDirect()
		{
			ImageBufferType	*srcPtr = getImageBufferPtr();
			unsigned char	*dstPtr = mDisplayBuffer;

			for (int y = 0; y < mDisplayWidth; y++)
				for (int x = 0; x < mDisplayWidth * mOnePixelCount; x++, dstPtr++, srcPtr++)
					*dstPtr = (unsigned char)(*srcPtr);
		}
	};
 };
};

#endif	// #ifdef VIW_MODEL_DISPLAYBUFFER_H

