// =============================================================================
//  Exception.hpp
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
	\file		viw/Exception.h
	\author		Dairoku Sekiguchi
	\version	1.0.0
	\date		2014/12/30
	\brief		Header file for aile library types

	This file defines exceptionss for viw library
*/

#ifndef VIW_EXCEPTION_HPP
#define VIW_EXCEPTION_HPP

// Includes --------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

// Macros ----------------------------------------------------------------------
#define VIW_EXCEPT_AT_STRINGIFY(x)		#x
#define VIW_EXCEPT_AT_TOSTRING(x)		VIW_EXCEPT_AT_STRINGIFY(x)
#define VIW_EXCEPTION_AT				__FILE__ ":" VIW_EXCEPT_AT_TOSTRING(__LINE__)
#define	VIW_EXCEPTION_LOCATION_MACRO	__FUNCTION__ "  (" VIW_EXCEPTION_AT ")"


// Namespace -------------------------------------------------------------------
namespace viw
{
	// -------------------------------------------------------------------------
	// Throwable interface class
	// -------------------------------------------------------------------------
	class	Throwable
	{
	public:
		//	member functions
		virtual int				getExceptionCode()	const = 0;
		virtual int				getOSErrorCode()	const = 0;
		virtual const char		*getExceptionName()	const = 0;
		virtual const char		*getDescription()	const = 0;
		virtual const char		*getLocation()		const = 0;
	};

	// -------------------------------------------------------------------------
	// Exception class
	// -------------------------------------------------------------------------
	class	Exception : public Throwable
	{
	public:
		// Destructor ----------------------------------------------------------
	virtual						~Exception() {};

		// Constatns -----------------------------------------------------------
		static const int		MESSAGE_BUFFER_SIZE				= 128;
		enum ExceptionCode
		{
			OS_ERROR			= 1,
			MEMORY_ERROR,
			PARAM_ERROR,
			INVALID_OPERATION_ERROR,
			INTERNAL_ERROR,

			SUB_CLASS_ERROR		= 32768
		};

		// Member Functions ----------------------------------------------------
		virtual int				getExceptionCode() const { return mExceptionCode; };
		virtual int				getOSErrorCode() const { return mOSErrorCode; };
		virtual const char		*getExceptionName() const { return mExceptionName; };
		virtual const char		*getDescription() const { return mDescription; };
		virtual const char		*getLocation() const { return mLocation; };
		virtual void			dump()
		{
			printf("Catch %s\n",
					getExceptionName());
			printf("Desc=%s\n",
					getDescription());
			printf("ExCode=%d (OSError=%d) in\n",
					getExceptionCode(), getOSErrorCode());
			printf("%s\n",
					getLocation());
		};

	protected:
		// Constructors ---------------------------------------------------------
								Exception(const char *inExceptionName, int inExceptionCode, const char *inDescription,
											const char *inLocation, int inOSErrorCode = 0)
								{
									set(inExceptionName, inExceptionCode, inDescription,
										inLocation, inOSErrorCode);
								}
								Exception(const Throwable &inEx)
								{
									set(inEx.getExceptionName(), inEx.getExceptionCode(), inEx.getDescription(),
										inEx.getLocation(), inEx.getOSErrorCode());
								}
								Exception(const Exception &inEx)
								{
									set(inEx.getExceptionName(), inEx.getExceptionCode(), inEx.getDescription(),
										inEx.getLocation(), inEx.getOSErrorCode());
								}

		// Member Functions ----------------------------------------------------
		virtual void			set(const char *inExceptionName, int inExceptionCode, const char *inDescription,
									const char *inLocation, int inOSErrorCode)
		{
			setExceptionName(inExceptionName);
			setExceptionCode(inExceptionCode);
			setDescription(inDescription);
			setLocation(inLocation);
			setOSErrorCode(inOSErrorCode);
		};

		virtual void			setExceptionCode(int inExceptionCode) { mExceptionCode = inExceptionCode; };
		virtual void			setOSErrorCode(int inOSErrorCode) { mOSErrorCode = inOSErrorCode; };
		virtual void			setExceptionName(const char *inExceptionName)
		{
			setMessageToBuffer(inExceptionName, mExceptionName);
		};
		virtual void			setDescription(const char *inDescription)
		{
			setMessageToBuffer(inDescription, mDescription);
		};
		virtual void			setLocation(const char *inLocation)
		{
			setMessageToBuffer(inLocation, mLocation);
		};
		virtual void			setMessageToBuffer(const char *inMessage, char *inBuf)
		{
			size_t	len;
	
			len = ::strlen(inMessage);
			if (len >= MESSAGE_BUFFER_SIZE)
				len = MESSAGE_BUFFER_SIZE - 1;
			::memcpy(inBuf, inMessage, len);
			inBuf[len] = 0;
		};

	private:
		// Member Variables ----------------------------------------------------
		int						mExceptionCode, mOSErrorCode;
		char					mExceptionName[MESSAGE_BUFFER_SIZE];
		char					mDescription[MESSAGE_BUFFER_SIZE];
		char					mLocation[MESSAGE_BUFFER_SIZE];
	};
};

// Namespace -------------------------------------------------------------------
namespace viw
{
	// -------------------------------------------------------------------------
	// Exception class
	// -------------------------------------------------------------------------
	class	ViwException : public viw::Exception
	{
	public:
		// Constructors and Destructor -----------------------------------------
		ViwException(int inExceptionCode, const char *inDescription,
			const char *inLocation, int inOSErrorCode = 0)
			: Exception("ViwException", inExceptionCode, inDescription,
			inLocation, inOSErrorCode)
		{
		}
		ViwException(const ViwException &inEx)
			: Exception(inEx)
		{
		}
		virtual					~ViwException() {};

		// Constatns -----------------------------------------------------------
		enum ExceptionCode
		{
			// a bit dirty redifintion of constants, which are defined in the parent class...
			OS_ERROR = 1,
			MEMORY_ERROR,
			PARAM_ERROR,
			INVALID_OPERATION_ERROR,
			INTERNAL_ERROR,

			FILE_FORMAT_ERROR = 32768
		};
	};
};

#endif	// VIW_EXCEPTION_HPP
