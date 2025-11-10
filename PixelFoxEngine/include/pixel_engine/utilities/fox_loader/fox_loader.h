#pragma once

#include "PixelFoxEngineAPI.h"

#include <string>
#include <unordered_map>
#include <sstream>
#include "pixel_engine/utilities/filesystem/file_system.h"

#include "core/unordered_map.h"


namespace pixel_engine
{
	class PFE_API PEFoxLoader
	{
	public:
		void Load(const std::string& filePath);
		void Save(const std::string& filePath);

		PEFoxLoader&	   operator=(const std::string& value);
		const PEFoxLoader& operator[](const std::string& key) const;
		
		PEFoxLoader& GetOrCreate(const std::string& key);

		auto begin() { return m_childerns.begin(); }
		auto end  () { return m_childerns.end(); }
		
		auto begin() const { return m_childerns.begin(); }
		auto end  () const { return m_childerns.end(); }

		const std::string& GetValue() const   { return m_szValue; }
		void SetValue(const std::string& val) { m_szValue = val; }

		bool Contains(const std::string& key) const;

		//~ Helpers
		std::string ToFormattedString(int indent = 0) const;
		void FromStream(std::istream& input);

		float AsFloat() const;
		int   AsInt() const;
		bool  AsBool() const;
		bool  IsValid() const;
		void  Clear();

	private:
		void Serializer (std::ostream& output, int indent) const;
		void ParserBlock(std::istream& input);

	private:
		std::string m_szValue;
		fox::unordered_map<std::string, PEFoxLoader> m_childerns;
		PEFileSystem m_fileSystem{};
	};
} // namespace pixel_engine