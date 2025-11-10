#pragma once

#include "PixelFoxEngineAPI.h"
#include <windows.h>
#include <string>


namespace pixel_engine
{
	typedef struct PFE_FILE_PATH_DESC
	{
		std::string DirectoryNames;
		std::string FileName;
	} DIRECTORY_AND_FILE_NAME;

	class PFE_API PEFileSystem
	{
	public:
		 PEFileSystem() = default;
		~PEFileSystem() = default;

		PEFileSystem(const PEFileSystem&)		= default;
		PEFileSystem(PEFileSystem&&)			= default;

		PEFileSystem& operator=(const PEFileSystem&) = default;
		PEFileSystem& operator=(PEFileSystem&&)		 = default;

		bool OpenForRead (const std::string& path);
		bool OpenForWrite(const std::string& path);
		void Close();

		bool ReadBytes (void* dest, size_t size)	   const;
		bool WriteBytes(const void* data, size_t size) const;

		bool ReadUInt32 (uint32_t& value) const;
		bool WriteUInt32(uint32_t value) const;

		bool ReadString		(std::string& outStr)	 const;
		bool WriteString	(const std::string& str) const;
		bool WritePlainText	(const std::string& str) const;

		uint64_t GetFileSize() const;
		bool	 IsOpen() const;

		//~ Utility
		static bool IsPathExists(const std::string& path);
		static bool IsDirectory	(const std::string& path);
		static bool IsFile		(const std::string& path);

		static DIRECTORY_AND_FILE_NAME SplitPathFile(const std::string& fullPath);

		template<typename... Args>
		static bool DeleteFiles(Args&&... args);

		template<typename... Args>
		static bool CreateDirectories(Args&&... args);

	private:
		HANDLE n_handle = INVALID_HANDLE_VALUE;
		bool m_bReadMode = false;
	};

	template<typename ...Args>
	inline bool PEFileSystem::DeleteFiles(Args&& ...args)
	{
		bool allSuccess = true;

		auto tryDelete = [&](const auto& path)
			{
				if (!DeleteFile(path.c_str())) allSuccess = false;
			};

		(tryDelete(std::forward<Args>(args)), ...);
		return allSuccess;
	}

	template<typename ...Args>
	inline bool PEFileSystem::CreateDirectories(Args&& ...args)
	{
		bool allSuccess = true;

		auto tryCreate = [&](const auto& pathStr)
			{
				std::string current;
				for (size_t i = 0; i < pathStr.length(); ++i)
				{
					wchar_t ch = pathStr[i];
					current += ch;

					if (ch == L'\\' || ch == L'/')
					{
						if (!current.empty() && !IsPathExists(current))
						{
							if (!CreateDirectory(current.c_str(),
								nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
							{
								allSuccess = false;
								return;
							}
						}
					}
				}

				// Final directory (if its not ending with a slash)
				if (!IsPathExists(current))
				{
					if (!CreateDirectory(current.c_str(), 
						nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
						allSuccess = false;
				}
			};

		(tryCreate(std::forward<Args>(args)), ...);
		return allSuccess;
	}
} // namespace pixel_engine

