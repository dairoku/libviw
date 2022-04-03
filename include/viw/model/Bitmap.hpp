// =============================================================================
//  Bitmap.hpp
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
	\file		viw/model/Bitmap.h
	\author		Dairoku Sekiguchi
	\version	1.0.0
	\date		2014/02/22
	\brief		Header file for viw library types

	This file defines types for viw library
*/

#ifndef VIW_UTIL_BITMAP_H
#define VIW_UTIL_BITMAP_H

// Includes --------------------------------------------------------------------
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "viw/utils/ColorMap.hpp"
#include "viw/Exception.hpp"

// Namespace -------------------------------------------------------------------
namespace viw
{
 namespace model
 {
	// -------------------------------------------------------------------------
	// Bitmap class
	// -------------------------------------------------------------------------
	class	Bitmap
	{
	public:
		// Constructors and Destructor -----------------------------------------
		// ---------------------------------------------------------------------
		// ~Bitmap
		// ---------------------------------------------------------------------
		virtual ~Bitmap()
		{
			if (mAllocatedBitmapInfoPtr != NULL)
				delete mAllocatedBitmapInfoPtr;

			if (mAllocatedBitmapBitsPtr != NULL)
				delete mAllocatedBitmapBitsPtr;
		}

		// Member functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// getBitmapInfoPtr
		// ---------------------------------------------------------------------
		const BITMAPINFO	*getBitmapInfoPtr()
		{
			return (BITMAPINFO *)mBitmapInfoPtr;
		}
		// ---------------------------------------------------------------------
		// getBitmapInfoHeaderPtr
		// ---------------------------------------------------------------------
		const BITMAPINFOHEADER	*getBitmapInfoHeaderPtr()
		{
			return mBitmapInfoPtr;
		}
		// ---------------------------------------------------------------------
		// getColorPalettePtr
		// ---------------------------------------------------------------------
		RGBQUAD	*getColorPalettePtr()
		{
			char	*bufPtr = (char *)mBitmapInfoPtr;
			bufPtr += (sizeof(BITMAPINFOHEADER));
			return (RGBQUAD *)bufPtr;
		}
		// ---------------------------------------------------------------------
		// getBitmapInfoSize
		// ---------------------------------------------------------------------
		// Note: This size includes the RGBQuad part (The Color Pallet Part)
		size_t	getBitmapInfoSize()
		{
			return mBitmapInfoSize;
		}
		// ---------------------------------------------------------------------
		// getColorPalletNum
		// ---------------------------------------------------------------------
		int	getColorPalletNum()
		{
			return mColorPalletNum;
		}
		// ---------------------------------------------------------------------
		// getBitmapBitsPtr
		// ---------------------------------------------------------------------
		unsigned char	*getBitmapBitsPtr()
		{
			return mBitmapBitsPtr;
		}
		// ---------------------------------------------------------------------
		// isTopDown
		// ---------------------------------------------------------------------
		bool	isTopDown()
		{
			if (mBitmapInfoPtr == NULL)
				return false;

			if (mBitmapInfoPtr->biHeight < 0)
				return true;

			return false;
		}
		// ---------------------------------------------------------------------
		// getWidth
		// ---------------------------------------------------------------------
		int	getWidth()
		{
			if (mBitmapInfoPtr == NULL)
				return 0;

			return mBitmapInfoPtr->biWidth;
		}
		// ---------------------------------------------------------------------
		// getHeight
		// ---------------------------------------------------------------------
		int	getHeight()
		{
			if (mBitmapInfoPtr == NULL)
				return 0;

			return getAbsBitmapHeight(mBitmapInfoPtr);
		}
		// ---------------------------------------------------------------------
		// getBitmapBitsSize
		// ---------------------------------------------------------------------
		size_t	getBitmapBitsSize()
		{
			return mBitmapBitsSize;
		}
		// ---------------------------------------------------------------------
		// getBitmapLineOffset
		// ---------------------------------------------------------------------
		size_t	getBitmapLineOffset()
		{
			return mBitmapLineOffset;
		}
		// ---------------------------------------------------------------------
		// setBitmapInfo
		// ---------------------------------------------------------------------
		bool	setBitmapInfo(int inWidth, int inHeight, int inBitCount = 24)
		{
			bool	allocationIsNeeded = false;

			if (mBitmapInfoPtr == NULL)
				allocationIsNeeded = true;
			else
			{
				if (mBitmapInfoPtr->biBitCount != inBitCount)
					allocationIsNeeded = true;
			}

			if (allocationIsNeeded == true)
			{
				int	colorPalletNum = calColorPalletNum(inBitCount);

				if (mBitmapInfoPtr != NULL && mAllocatedBitmapInfoPtr == NULL)
				{
					if (mThrowsEx == false)
						return false;
					else
						throw ViwException(ViwException::PARAM_ERROR,
							"invalid parameter (can't change inBitCount)", VIW_EXCEPTION_LOCATION_MACRO, 0);
				}

				if (mAllocatedBitmapInfoPtr != NULL)
					delete mAllocatedBitmapInfoPtr;
				mAllocatedBitmapInfoPtr = NULL;
				if (allocateBitmapInfo(colorPalletNum) == false)
					return false;

				mBitmapInfoPtr->biSize = sizeof(BITMAPINFOHEADER);
				mBitmapInfoPtr->biWidth = inWidth;
				mBitmapInfoPtr->biHeight = inHeight;
				mBitmapInfoPtr->biPlanes = 1;
				mBitmapInfoPtr->biBitCount = inBitCount;
				mBitmapInfoPtr->biCompression = 0;
				mBitmapInfoPtr->biXPelsPerMeter = 3779;
				mBitmapInfoPtr->biYPelsPerMeter = 3779;
				mBitmapInfoPtr->biClrUsed = colorPalletNum;
				mBitmapInfoPtr->biClrImportant = 0;
				setBitmapBitsSize(mBitmapInfoPtr);
				setColorPallet(mBitmapInfoPtr);
			}

			if (mBitmapInfoPtr->biWidth != inWidth ||
				mBitmapInfoPtr->biHeight != inHeight)
			{
				mBitmapInfoPtr->biWidth = inWidth;
				mBitmapInfoPtr->biHeight = inHeight;
				setBitmapBitsSize(mBitmapInfoPtr);
			}

			return true;
		}
		// ---------------------------------------------------------------------
		// setBitmapBits
		// ---------------------------------------------------------------------
		bool	setBitmapBits(unsigned char *inImageBuffer, size_t inBufferSize,
								int inWidth, int inHeight, int inBitCount = 24)
		{
			if (setBitmapInfo(inWidth, inHeight, inBitCount) == false)
				return false;

			return setBitmapBits(inImageBuffer, inBufferSize);
		}
		// ---------------------------------------------------------------------
		// setBitmapBits
		// ---------------------------------------------------------------------
		bool	setBitmapBits(unsigned char *inImageBuffer, size_t inBufferSize)
		{
			if (inImageBuffer == mBitmapBitsPtr && inBufferSize == mBitmapBitsSize)
				return true;

			mBitmapLineOffset = calBitmapLineOffset(mBitmapInfoPtr);
			mBitmapBitsSize = mBitmapLineOffset * getHeight();

			if (inBufferSize != mBitmapBitsSize)
			{
				if (mThrowsEx == false)
					return false;
				else
					throw ViwException(ViwException::PARAM_ERROR,
					"invalid inBufferSize", VIW_EXCEPTION_LOCATION_MACRO, 0);
			}

			if (mAllocatedBitmapBitsPtr != NULL)
			{
				delete mAllocatedBitmapBitsPtr;
				mAllocatedBitmapBitsPtr = NULL;
			}

			mBitmapBitsPtr = inImageBuffer;
			return true;
		}
		// ---------------------------------------------------------------------
		// saveToFile
		// ---------------------------------------------------------------------
		bool	saveToFile(const char *inFileName)
		{
			LPCTSTR			fileName;
			bool			result;

			try
			{
				int wcharsize = ::MultiByteToWideChar(CP_ACP, 0, inFileName, -1, NULL, 0);
				fileName = new WCHAR[wcharsize];
				if (fileName == NULL)
					return false;
				::MultiByteToWideChar(CP_ACP, 0, inFileName, -1, (LPWSTR )fileName, wcharsize);

				result = saveToFile(fileName);

				delete fileName;
			}

			catch (ViwException &ex)
			{
				delete fileName;
				throw ex;
			}

			return result;
		}
		// ---------------------------------------------------------------------
		// saveToFile
		// ---------------------------------------------------------------------
		bool	saveToFile(const LPCTSTR inFileName)
		{
			HANDLE					fileHandle;
			BITMAPFILEHEADER		bmpFHeader;
			DWORD					sizeInBytes;
			BOOL					result;

			fileHandle = ::CreateFileW(inFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (fileHandle == INVALID_HANDLE_VALUE)
			{
				if (mThrowsEx == false)
					return NULL;
				else
					throw ViwException(ViwException::OS_ERROR,
					"::CreateFileW() returned NULL", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}

			bmpFHeader.bfType		= 0x4d42;
			bmpFHeader.bfOffBits	= (DWORD )(sizeof(BITMAPFILEHEADER) + mBitmapInfoSize);
			bmpFHeader.bfReserved1	= 0;
			bmpFHeader.bfReserved2	= 0;
			bmpFHeader.bfSize		= (DWORD )(bmpFHeader.bfOffBits + mBitmapBitsSize);

			result = ::WriteFile(fileHandle, &bmpFHeader, sizeof(BITMAPFILEHEADER), &sizeInBytes, NULL);
			if (result == 0 || sizeInBytes != sizeof(BITMAPFILEHEADER))
			{
				::CloseHandle(fileHandle);
				if (mThrowsEx == false)
					return false;
				else
					throw ViwException(ViwException::OS_ERROR,
					"::WriteFile() returned an error", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}

			result = ::WriteFile(fileHandle, mBitmapInfoPtr, (DWORD)mBitmapInfoSize, &sizeInBytes, NULL);
			if (result == 0 || sizeInBytes != mBitmapInfoSize)
			{
				::CloseHandle(fileHandle);
				if (mThrowsEx == false)
					return false;
				else
					throw ViwException(ViwException::OS_ERROR,
					"::WriteFile() returned an error", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}

			result = ::WriteFile(fileHandle, mBitmapBitsPtr, (DWORD)mBitmapBitsSize, &sizeInBytes, NULL);
			if (result == 0 || sizeInBytes != mBitmapBitsSize)
			{
				::CloseHandle(fileHandle);
				if (mThrowsEx == false)
					return false;
				else
					throw ViwException(ViwException::OS_ERROR,
					"::WriteFile() returned an error", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}
			::CloseHandle(fileHandle);

			return true;
		}
		// ---------------------------------------------------------------------
		// dump
		// ---------------------------------------------------------------------
		void	dump()
		{
			if (mBitmapInfoPtr != NULL)
				dumpBITMAPINFOHEADER(mBitmapInfoPtr);
			else
				printf("mBitmapInfoPtr == NULL\n");

			printf("\n");
			printf("mBitmapInfoSize:   %zu\n", mBitmapInfoSize);
			printf("mColorPalletNum:   %d\n", mColorPalletNum);
			printf("mBitmapBitsSize:   %zu\n", mBitmapBitsSize);
			printf("mBitmapLineOffset: %zu\n", mBitmapLineOffset);
		}
		// Static Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// createBitmap
		// ---------------------------------------------------------------------
		static Bitmap	*createBitmap(int inWidth, int inHeight, int inBitCount = 24, bool inThroswEx = false)
		{
			Bitmap	*bitmap = new Bitmap(inThroswEx);
			int		colorPalletNum = calColorPalletNum(inBitCount);
			
			try
			{
				if (bitmap->setBitmapInfo(inWidth, inHeight, inBitCount) == false)
				{
					delete bitmap;
					return NULL;
				}

				if (bitmap->allocateImageBuffer() == false)
				{
					delete bitmap;
					return NULL;
				}
			}

			catch (ViwException &ex)
			{
				delete bitmap;
				throw ex;
			}

			return bitmap;
		}
		// ---------------------------------------------------------------------
		// createBitmap
		// ---------------------------------------------------------------------
		static Bitmap	*createBitmap(unsigned char *inExternalBuffer, size_t inBufferSize,
										int inWidth, int inHeight, int inBitCount = 24, bool inThroswEx = false)
		{
			Bitmap	*bitmap = new Bitmap(inThroswEx);
			int		colorPalletNum = calColorPalletNum(inBitCount);

			try
			{
				if (bitmap->setBitmapInfo(inWidth, inHeight, inBitCount) == false)
				{
					delete bitmap;
					return NULL;
				}

				if (bitmap->setBitmapBits(inExternalBuffer, inBufferSize) == false)
				{
					delete bitmap;
					return NULL;
				}
			}

			catch (ViwException &ex)
			{
				delete bitmap;
				throw ex;
			}

			return bitmap;
		}
		// ---------------------------------------------------------------------
		// loadFromFile
		// ---------------------------------------------------------------------
		static Bitmap	*loadFromFile(const char *inFileName, bool inThrowsEx = false)
		{
			LPCTSTR			fileName;
			Bitmap			*bitmap;

			try
			{
				int wcharsize = ::MultiByteToWideChar(CP_ACP, 0, inFileName, -1, NULL, 0);
				fileName = new WCHAR[wcharsize];
				::MultiByteToWideChar(CP_ACP, 0, inFileName, -1, (LPWSTR )fileName, wcharsize);

				bitmap = loadFromFile(fileName, inThrowsEx);

				delete fileName;
			}

			catch (ViwException &ex)
			{
				delete fileName;
				throw ex;
			}

			return bitmap;
		}
		// ---------------------------------------------------------------------
		// loadFromFile
		// ---------------------------------------------------------------------
		static Bitmap	*loadFromFile(const LPCTSTR inFileName, bool inThrowsEx = false)
		{
			HANDLE					fileHandle;
			BITMAPFILEHEADER		bmpFHeader;
			BITMAPINFOHEADER		bmpInfo;
			DWORD					sizeInBytes;
			BOOL					result;

			fileHandle = ::CreateFileW(inFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (fileHandle == INVALID_HANDLE_VALUE)
			{
				if (inThrowsEx == false)
					return NULL;
				else
					throw ViwException(ViwException::OS_ERROR,
					"::CreateFile() returned NULL", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}

			result = ::ReadFile(fileHandle, &bmpFHeader, sizeof(BITMAPFILEHEADER), &sizeInBytes, NULL);
			if (result == 0 || sizeInBytes != sizeof(BITMAPFILEHEADER))
			{
				::CloseHandle(fileHandle);
				if (inThrowsEx == false)
					return NULL;
				else
					throw ViwException(ViwException::OS_ERROR,
					"::ReadFile() returned an error", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}
			
			// Check bmpHeader
			if (bmpFHeader.bfType != 0x4d42 || bmpFHeader.bfReserved1 != 0 || bmpFHeader.bfReserved2 != 0)
			{
				dumpBITMAPFILEHEADER(&bmpFHeader);
				::CloseHandle(fileHandle);
				if (inThrowsEx == false)
					return NULL;
				else
					throw ViwException(ViwException::FILE_FORMAT_ERROR,
					"bmpFHeader.bfType != 0x4d42 || bmpFHeader.bfReserved1 != 0 || bmpFHeader.bfReserved2 != 0",
					VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}
			
			result = ::ReadFile(fileHandle, &bmpInfo, sizeof(BITMAPINFOHEADER), &sizeInBytes, NULL);
			if (result == 0 || sizeInBytes != sizeof(BITMAPINFOHEADER))
			{
				::CloseHandle(fileHandle);
				if (inThrowsEx == false)
					return NULL;
				else
					throw ViwException(ViwException::OS_ERROR,
					"::ReadFile() returned an error", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}
			
			// Check mBmpInfo (Dosen't support OS/2 type)
			if (bmpInfo.biSize != sizeof(BITMAPINFOHEADER) ||
				//bmpInfo.biPlanes != 1 || bmpInfo.biCompression != 0 || bmpInfo.biSizeImage != 0)
				bmpInfo.biPlanes != 1 || bmpInfo.biCompression != 0)
			{
				dumpBITMAPINFOHEADER(&bmpInfo);
				::CloseHandle(fileHandle);
				if (inThrowsEx == false)
					return NULL;
				else
					throw ViwException(ViwException::FILE_FORMAT_ERROR,
					"bmpInfo.biSize != sizeof(BITMAPINFOHEADER) || bmpInfo.biPlanes != 1 || bmpInfo.biCompression != 0 || bmpInfo.biSizeImage != 0",
					VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}
			
			int	rgbQuadNum = calColorPalletNum(&bmpInfo);			
			DWORD	offBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * rgbQuadNum;
			if (bmpFHeader.bfOffBits < offBits)
			{
				::CloseHandle(fileHandle);
				if (inThrowsEx == false)
					return NULL;
				else
					throw ViwException(ViwException::FILE_FORMAT_ERROR,
					"bmpFHeader.bfOffBits < offBits",
					VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}
			Bitmap	*bitmap = new Bitmap(inThrowsEx);
			if (bitmap == NULL)
			{
				::CloseHandle(fileHandle);
				if (inThrowsEx == false)
					return NULL;
				else
					throw ViwException(ViwException::MEMORY_ERROR,
					"new Bitmap(inThrowsEx) returned NULL", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}

			try
			{
				if (!bitmap->allocateBitmapInfo(rgbQuadNum))
				{
					::CloseHandle(fileHandle);
					delete bitmap;
					return NULL;
				}
			}

			catch (ViwException &ex)
			{
				::CloseHandle(fileHandle);
				delete bitmap;
				throw ex;
			}

			*bitmap->mBitmapInfoPtr = bmpInfo;
			if (bitmap->mColorPalletNum != 0)
			{
				unsigned char	*colorPalletPtr = (unsigned char *)bitmap->mBitmapInfoPtr;
				colorPalletPtr += sizeof(BITMAPINFOHEADER);
				size_t	colorPalletSize = sizeof(RGBQUAD) * bitmap->mColorPalletNum;
				result = ::ReadFile(fileHandle, colorPalletPtr, (DWORD)colorPalletSize, &sizeInBytes, NULL);
				if (result == 0 || sizeInBytes != colorPalletSize)
				{
					::CloseHandle(fileHandle);
					delete bitmap;
					if (inThrowsEx == false)
						return NULL;
					else
						throw ViwException(ViwException::OS_ERROR,
						"::ReadFile() returned an error", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
				}
				if (bmpFHeader.bfOffBits != offBits)
				{
					printf("WARNING: bmpFHeader.bfOffBits != offBits:%d, %d\n", bmpFHeader.bfOffBits, offBits);
					DWORD filePointer = ::SetFilePointer(fileHandle, (bmpFHeader.bfOffBits - offBits), NULL, FILE_CURRENT);
					if (filePointer == INVALID_SET_FILE_POINTER)
					{
						::CloseHandle(fileHandle);
						delete bitmap;
						if (inThrowsEx == false)
							return NULL;
						else
							throw ViwException(ViwException::OS_ERROR,
							"::SetFilePointer() returned an error", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
					}
				}
			}
			bitmap->mBitmapLineOffset = calBitmapLineOffset(bitmap->mBitmapInfoPtr);
			bitmap->mBitmapBitsSize = bitmap->mBitmapLineOffset * getAbsBitmapHeight(bitmap->mBitmapInfoPtr);
			bitmap->mBitmapBitsPtr = new unsigned char[bitmap->mBitmapBitsSize];
			if (bitmap->mBitmapBitsPtr == NULL)
			{
				::CloseHandle(fileHandle);
				delete bitmap;
				if (inThrowsEx == false)
					return NULL;
				else
					throw ViwException(ViwException::MEMORY_ERROR,
					"new unsigned char[bitmap->mBitmapBitsSize] returned NULL", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}
			
			result = ::ReadFile(fileHandle, bitmap->mBitmapBitsPtr, (DWORD)bitmap->mBitmapBitsSize, &sizeInBytes, NULL);
			if (result == 0 || sizeInBytes != bitmap->mBitmapBitsSize)
			{
				::CloseHandle(fileHandle);
				delete bitmap;
				if (inThrowsEx == false)
					return NULL;
				else
					throw ViwException(ViwException::OS_ERROR,
					"::ReadFile() returned an error", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}
			::CloseHandle(fileHandle);

			return bitmap;
		}
		// ---------------------------------------------------------------------
		// calBitmapBitsSize
		// ---------------------------------------------------------------------
		static size_t	calBitmapBitsSize(int inWidth, int inHeight, int inBitCount)
		{
			return calBitmapLineOffset(inWidth, inBitCount) * abs(inHeight);
		}
		// ---------------------------------------------------------------------
		// calBitmapBitsSize
		// ---------------------------------------------------------------------
		static size_t	calBitmapBitsSize(const BITMAPINFOHEADER *inBmpInfo)
		{
			return calBitmapLineOffset(inBmpInfo) * getAbsBitmapHeight(inBmpInfo);
		}
		// ---------------------------------------------------------------------
		// calBitmapLineOffset
		// ---------------------------------------------------------------------
		static size_t	calBitmapLineOffset(const BITMAPINFOHEADER *inBmpInfo)
		{
			return calBitmapLineOffset(inBmpInfo->biWidth, inBmpInfo->biBitCount);
		}
		// ---------------------------------------------------------------------
		// calBitmapLineOffset
		// ---------------------------------------------------------------------
		static size_t	calBitmapLineOffset(int inWidth, int inBitCount)
		{
			int	lineOffset;
			lineOffset = inWidth * inBitCount;
			lineOffset /= 8;
			if (lineOffset % 4 != 0)
				lineOffset = (lineOffset / 4 + 1) * 4;
			return lineOffset;
		}
		// ---------------------------------------------------------------------
		// calColorPalletNum
		// ---------------------------------------------------------------------
		static int	calColorPalletNum(int inBitCount)
		{
			int	rgbQuadNum = 0;	// bitCount = 24 and others
			switch (inBitCount)
			{
			case 1:
				rgbQuadNum = 2;
				break;
			case 4:
				rgbQuadNum = 16;
				break;
			case 8:
				rgbQuadNum = 256;
				break;
			}
			return rgbQuadNum;
		}
		// ---------------------------------------------------------------------
		// calColorPalletNum
		// ---------------------------------------------------------------------
		static int	calColorPalletNum(const BITMAPINFOHEADER *inBmpInfoHeader)
		{
			return calColorPalletNum(inBmpInfoHeader->biBitCount);
		}
		// ---------------------------------------------------------------------
		// calColorPalletNum
		// ---------------------------------------------------------------------
		static int	calColorPalletNum(const BITMAPINFO *inBmpInfo)
		{
			return calColorPalletNum(inBmpInfo->bmiHeader.biBitCount);
		}
		// ---------------------------------------------------------------------
		// setColorPallet
		// ---------------------------------------------------------------------
		static void	setColorPallet(BITMAPINFOHEADER *inBmpInfoHeader,
			utils::ColorMap::ColorMapIndex inIndex = utils::ColorMap::CMIndex_GrayScale)
		{
			setColorPallet((BITMAPINFO *)inBmpInfoHeader, inIndex);
		}
		// ---------------------------------------------------------------------
		// setColorPallet
		// ---------------------------------------------------------------------
		static void	setColorPallet(BITMAPINFO *inBmpInfo,
						utils::ColorMap::ColorMapIndex inIndex = utils::ColorMap::CMIndex_GrayScale)
		{
			int	num = calColorPalletNum(inBmpInfo->bmiHeader.biBitCount);
			if (num == 0)
				return;

			if (num == 2)
			{
				inBmpInfo->bmiColors[0].rgbRed = 0;
				inBmpInfo->bmiColors[0].rgbGreen = 0;
				inBmpInfo->bmiColors[0].rgbBlue = 0;
				inBmpInfo->bmiColors[1].rgbRed = 255;
				inBmpInfo->bmiColors[1].rgbGreen = 255;
				inBmpInfo->bmiColors[1].rgbBlue = 255;
				return;
			}

			if (num == 16)
			{
				return;
			}

			utils::ColorMap::getColorMap(inIndex, num, (unsigned char *)inBmpInfo->bmiColors);
		}
		// ---------------------------------------------------------------------
		// setBitmapBitsSize
		// ---------------------------------------------------------------------
		static void	setBitmapBitsSize(BITMAPINFOHEADER *inBmpInfo)
		{
			inBmpInfo->biSizeImage = (DWORD)(calBitmapLineOffset(inBmpInfo) * getAbsBitmapHeight(inBmpInfo));
		}
		// ---------------------------------------------------------------------
		// getAbsBitmapHeight
		// ---------------------------------------------------------------------
		static int	getAbsBitmapHeight(const BITMAPINFO *inBmpInfo)
		{
			return getAbsBitmapHeight((BITMAPINFOHEADER *)inBmpInfo);
		}
		// ---------------------------------------------------------------------
		// getAbsBitmapHeight
		// ---------------------------------------------------------------------
		static int	getAbsBitmapHeight(const BITMAPINFOHEADER *inBmpInfoHeader)
		{
			return abs(inBmpInfoHeader->biHeight);
		}
		// ---------------------------------------------------------------------
		// dumpBITMAPFILEHEADER
		// ---------------------------------------------------------------------
		static void	dumpBITMAPFILEHEADER(const BITMAPFILEHEADER *inBmpFHeader)
		{
			printf("\nDump BITMAPFILEHEADER\n");
			printf("biSize:           %d\n", inBmpFHeader->bfType);
			printf("biWidth:          %d\n", inBmpFHeader->bfSize);
			printf("biHeight:         %d\n", inBmpFHeader->bfReserved1);
			printf("biPlanes:         %d\n", inBmpFHeader->bfReserved2);
			printf("biBitCount:       %d\n", inBmpFHeader->bfOffBits);
		}
		// ---------------------------------------------------------------------
		// dumpBITMAPINFOHEADER
		// ---------------------------------------------------------------------
		static void	dumpBITMAPINFOHEADER(const BITMAPINFOHEADER *inBmpInfo)
		{
			printf("\nDump BITMAPINFOHEADER\n");
			printf("biSize:           %d\n", inBmpInfo->biSize);
			printf("biWidth:          %d\n", inBmpInfo->biWidth);
			printf("biHeight:         %d\n", inBmpInfo->biHeight);
			printf("biPlanes:         %d\n", inBmpInfo->biPlanes);
			printf("biBitCount:       %d\n", inBmpInfo->biBitCount);
			printf("biCompression:    %d\n", inBmpInfo->biCompression);
			printf("biSizeImage:      %d\n", inBmpInfo->biSizeImage);
			printf("biXPelsPerMeter:  %d\n", inBmpInfo->biXPelsPerMeter);
			printf("biYPelsPerMeter:  %d\n", inBmpInfo->biYPelsPerMeter);
			printf("biClrUsed:        %d\n", inBmpInfo->biClrUsed);
			printf("biClrImportant:   %d\n", inBmpInfo->biClrImportant);
		}

	protected:
		// Member variables ----------------------------------------------------
		BITMAPINFOHEADER	*mBitmapInfoPtr;
		BITMAPINFOHEADER	*mAllocatedBitmapInfoPtr;
		size_t				mBitmapInfoSize;
		int					mColorPalletNum;
		unsigned char		*mBitmapBitsPtr;
		unsigned char		*mAllocatedBitmapBitsPtr;
		size_t				mBitmapBitsSize;
		size_t				mBitmapLineOffset;
		bool				mThrowsEx;

		// Constructors and Destructor -----------------------------------------
		// ---------------------------------------------------------------------
		// Bitmap
		// ---------------------------------------------------------------------
		Bitmap(bool inThrowsEx)
		{
			mBitmapInfoPtr = NULL;
			mAllocatedBitmapInfoPtr = NULL;
			mBitmapInfoSize = 0;
			mColorPalletNum = 0;

			mBitmapBitsPtr = NULL;
			mAllocatedBitmapBitsPtr = NULL;
			mBitmapBitsSize = 0;
			mBitmapLineOffset = 0;
			mThrowsEx = inThrowsEx;
		}

		bool	allocateBitmapInfo(int inColorPalletNum = 0)
		{
			mColorPalletNum = inColorPalletNum;
			mBitmapInfoSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * mColorPalletNum;

			mAllocatedBitmapInfoPtr = (BITMAPINFOHEADER *)(new unsigned char[mBitmapInfoSize]);
			if (mAllocatedBitmapInfoPtr == NULL)
			{
				if (mThrowsEx == false)
					return false;
				else
					throw ViwException(ViwException::MEMORY_ERROR,
					"new unsigned char[mBitmapInfoSize] returned NULL", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}

			mBitmapInfoPtr = mAllocatedBitmapInfoPtr;
			::ZeroMemory(mBitmapInfoPtr, mBitmapInfoSize);
			return true;
		}

		bool	allocateImageBuffer()
		{
			mBitmapLineOffset = calBitmapLineOffset(mBitmapInfoPtr);
			mBitmapBitsSize = mBitmapLineOffset * getHeight();
			
			mAllocatedBitmapBitsPtr = new unsigned char[mBitmapBitsSize];
			if (mAllocatedBitmapBitsPtr == NULL)
			{
				if (mThrowsEx == false)
					return false;
				else
					throw ViwException(ViwException::MEMORY_ERROR,
					"new unsigned char[mBitmapBitsSize] returned NULL", VIW_EXCEPTION_LOCATION_MACRO, ::GetLastError());
			}

			mBitmapBitsPtr = mAllocatedBitmapBitsPtr;
			::ZeroMemory(mBitmapBitsPtr, mBitmapBitsSize);
			return true;
		}
	};
 };
};

#endif	// #ifdef VIW_UTIL_BITMAP_H

