// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once
#include "PixelFoxEngineAPI.h"
#include <stdexcept>
#include <string>

namespace pixel_engine
{
	class PFE_API BaseException: public std::exception
	{
	public:
		BaseException(
			_In_z_     const char* file,
			_In_	   const int   line,
			_In_z_     const char* function,
			_In_opt_z_ const char* message
		) : m_szFilePath(file),  m_nLineNumber(line),
			m_szFunctionName(function)
		{
			if (message) m_szErrorMessage = message;
			else m_szErrorMessage		  = "No error message provided";
		}

		_NODISCARD _Ret_z_ _Ret_valid_ _Check_return_
		virtual const char* what() const noexcept override
		{
			if (m_szWhatBuffer.empty())
			{
				m_szWhatBuffer =
					"[BaseException] "	 + m_szErrorMessage				 +
					"\nOn File Path: "	 + m_szFilePath					 +
					"\nAt Line Number: " + std::to_string(m_nLineNumber) +
					"\nFunction: "		 + m_szFunctionName;
								
			}
			return m_szWhatBuffer.c_str();
		}
		
	protected:
		std::string			m_szFilePath;
		std::string			m_szFunctionName;
		std::string			m_szErrorMessage;
		mutable std::string m_szWhatBuffer;
		int					m_nLineNumber;
	};
}

#define THROW() \
	throw pixel_engine::BaseException(__FILE__, __LINE__, __FUNCTION__)

#define THROW_MSG(msg) \
	throw pixel_engine::BaseException(__FILE__, __LINE__, __FUNCTION__, msg)
