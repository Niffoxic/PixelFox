#pragma once

#include "PixelFoxEngineAPI.h"
#include "base_exception.h"

#include <windows.h>

namespace pixel_engine
{
	class PFE_API WinException final : public BaseException
	{
	public:
		WinException(
			_In_z_ const char*	file,
			_In_   int			line,
			_In_z_ const char*	function,
			_In_   DWORD		hr = ::GetLastError()
		)
			:	BaseException(file, line, function, "None"),
				m_nLastError(hr)
		{
			LPVOID buffer = nullptr;
			DWORD size = FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER	|
				FORMAT_MESSAGE_FROM_SYSTEM		|
				FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				m_nLastError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				reinterpret_cast<LPSTR>(&buffer),
				0,
				nullptr
			);

			if (size && buffer)
			{
				m_szErrorMessage = static_cast<char*>(buffer);
				LocalFree(buffer);
			}
			else m_szErrorMessage = "UnRecognized Error Spotted (you're screw my boy)";

		}

		_NODISCARD _Ret_z_ _Ret_valid_ _Check_return_
		const char* what() const noexcept override
		{
			if (m_szWhatBuffer.empty())
			{
				m_szWhatBuffer =
					"[WinException] "	 + m_szErrorMessage				 +
					"\nOn File Path: "	 + m_szFilePath				     +
					"\nAt Line Number: " + std::to_string(m_nLineNumber) +
					"\nFunction: "		 + m_szFunctionName;
			}
			return m_szWhatBuffer.c_str();
		}

	private:
		DWORD m_nLastError{};
	};
} // namespace pixel_engine

#define THROW_WIN() \
    throw pixel_engine::WinException(__FILE__, __LINE__, __FUNCTION__)

#define THROW_WIN_IF_FAILS(_hr_expr) \
    do { HRESULT _hr_internal = (_hr_expr); if (FAILED(_hr_internal)) { \
        throw pixel_engine::WinException(__FILE__, __LINE__, __FUNCTION__, static_cast<DWORD>(_hr_internal)); \
    } } while(0)
