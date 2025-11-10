#include "pch.h"
#include "file_system.h"

using namespace pixel_engine;

bool PEFileSystem::OpenForRead(const std::string& path)
{
	n_handle = CreateFile(
		path.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	m_bReadMode = true;
	return n_handle != INVALID_HANDLE_VALUE;
}

bool PEFileSystem::OpenForWrite(const std::string& path)
{
	//~ Separate File and Directory
	auto file = SplitPathFile(path);

	//~ Create Directory
	CreateDirectories(file.DirectoryNames);

	//~ try to open or else creates it
	n_handle = CreateFile(
		path.c_str(),
		GENERIC_WRITE,
		0,
		nullptr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	m_bReadMode = false;
	return n_handle != INVALID_HANDLE_VALUE;
}

void PEFileSystem::Close()
{
	if (n_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(n_handle);
		n_handle = INVALID_HANDLE_VALUE;
		m_bReadMode = false;
	}
}

bool PEFileSystem::ReadBytes(void* dest, size_t size) const
{
	if (!m_bReadMode || n_handle == INVALID_HANDLE_VALUE) return false;

	DWORD bytesRead = 0;
	return ReadFile(n_handle, dest, static_cast<DWORD>(size), &bytesRead, nullptr) && bytesRead == size;
}

bool PEFileSystem::WriteBytes(const void* data, size_t size) const
{
	if (m_bReadMode || n_handle == INVALID_HANDLE_VALUE) return false;

	DWORD bytesWritten = 0;
	return WriteFile(n_handle, data, static_cast<DWORD>(size), &bytesWritten, nullptr) && bytesWritten == size;
}

bool PEFileSystem::ReadUInt32(uint32_t& value) const
{
	return ReadBytes(&value, sizeof(uint32_t));
}

bool PEFileSystem::WriteUInt32(uint32_t value) const
{
	return WriteBytes(&value, sizeof(uint32_t));
}

bool PEFileSystem::ReadString(std::string& outStr) const
{
	uint32_t len;
	if (!ReadUInt32(len)) return false;

	std::string buffer(len, '\0');
	if (!ReadBytes(buffer.data(), len)) return false;

	outStr = std::move(buffer);

	return true;
}

bool PEFileSystem::WriteString(const std::string& str) const
{
	uint32_t len = static_cast<uint32_t>(str.size());
	return WriteUInt32(len) && WriteBytes(str.data(), len);
}

bool PEFileSystem::WritePlainText(const std::string& str) const
{
	if (m_bReadMode || n_handle == INVALID_HANDLE_VALUE) return false;

	DWORD bytesWritten = 0;
	std::string line = str + "\n";
	return WriteFile(n_handle, line.c_str(), static_cast<DWORD>(line.size()), &bytesWritten, nullptr);
}

uint64_t PEFileSystem::GetFileSize() const
{
	if (n_handle == INVALID_HANDLE_VALUE) return 0;

	LARGE_INTEGER size{};
	if (!::GetFileSizeEx(n_handle, &size)) return 0;

	return static_cast<uint64_t>(size.QuadPart);
}

bool PEFileSystem::IsOpen() const
{
	return n_handle != INVALID_HANDLE_VALUE;
}

DIRECTORY_AND_FILE_NAME PEFileSystem::SplitPathFile(const std::string& fullPath)
{
	//~ Supports both '/' and '\\'
	size_t lastSlash = fullPath.find_last_of("/\\");
	if (lastSlash == std::string::npos)
	{
		//~ No folder, only filename
		return { "", fullPath };
	}

	return
	{
		fullPath.substr(0, lastSlash),
		fullPath.substr(lastSlash + 1)
	};
}

bool PEFileSystem::IsPathExists(const std::string& path)
{
	DWORD attr = GetFileAttributes(path.c_str());
	return (attr != INVALID_FILE_ATTRIBUTES);
}

bool PEFileSystem::IsDirectory(const std::string& path)
{
	DWORD attr = GetFileAttributes(path.c_str());
	return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool PEFileSystem::IsFile(const std::string& path)
{
	DWORD attr = GetFileAttributes(path.c_str());
	return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}
