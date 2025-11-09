#include "pch.h"
#include "fox_loader.h"

#include <algorithm>
#include <windows.h>
#include <string>
#include <sstream>
#include <iostream>

#include "pixel_engine/utilities/logger/logger.h"

using namespace pixel_engine;


void PEFoxLoader::Load(const std::string& filePath)
{
	if (!m_fileSystem.OpenForRead(filePath))
		return;

	uint64_t fileSize = m_fileSystem.GetFileSize();
	if (fileSize == 0)
	{
		m_fileSystem.Close();
		return;
	}

	std::string content(fileSize, '\0');
	m_fileSystem.ReadBytes(&content[0], fileSize);
	m_fileSystem.Close();

	std::istringstream iss(content);
	FromStream(iss);
}

void PEFoxLoader::Save(const std::string& filepath)
{
	if (!m_fileSystem.OpenForWrite(filepath))
		return;

	std::ostringstream oss;
	Serializer(oss, 0);

	if (!m_fileSystem.WritePlainText(oss.str()))
	{
		logger::error("Failed to Save: {}", filepath);
	}
	m_fileSystem.Close();
}

PEFoxLoader& PEFoxLoader::operator=(const std::string& value)
{
	m_szValue = value;
	return *this;
}

const PEFoxLoader& PEFoxLoader::operator[](const std::string& key) const
{
	if (m_childerns.contains(key))
	{
		return m_childerns.at(key);
	}
	static const PEFoxLoader invalidNode;  
	return invalidNode;
}

PEFoxLoader& PEFoxLoader::GetOrCreate(const std::string& key)
{
	return m_childerns[key];
}

bool PEFoxLoader::Contains(const std::string& key) const
{
	return m_childerns.contains(key);
}

std::string PEFoxLoader::ToFormattedString(int indent) const
{
	std::ostringstream oss;
	Serializer(oss, indent);
	return oss.str();
}

void PEFoxLoader::FromStream(std::istream& input)
{
	m_childerns.clear();
	m_szValue.clear();

	ParserBlock(input);
}

float PEFoxLoader::AsFloat() const
{
	if (!IsValid()) return 0.0f;

	try 
	{
		return std::stof(m_szValue);
	}
	catch (...)
	{
		return 0.0f;
	}
}

int PEFoxLoader::AsInt() const
{
	if (!IsValid()) return 0;

	try 
	{
		return std::stoi(m_szValue);
	}
	catch (...)
	{
		return 0;
	}
}

bool PEFoxLoader::AsBool() const
{
	if (!IsValid()) return false;

	std::string val = m_szValue;
	std::transform(val.begin(), val.end(), val.begin(), ::tolower);
	return (val == "true" || val == "1");
}


bool PEFoxLoader::IsValid() const

{
	return !m_szValue.empty() || !m_childerns.empty();
}

void PEFoxLoader::Clear()
{
	m_szValue.clear();
	m_childerns.clear();
}

void PEFoxLoader::Serializer(std::ostream& output, int indent) const
{
	const std::string indentStr(indent, '\t');

	if (!m_childerns.empty())
	{
		output << "{\n";
		bool first = true;
		for (const auto& [key, child] : m_childerns)
		{
			if (!first) output << ",\n";
			first = false;

			output << indentStr << '\t' << "\"" << key << "\": ";
			child.Serializer(output, indent + 1);
		}
		output << '\n' << indentStr << '}';
	}
	else
	{
		// Leaf node
		output << "\"" << m_szValue << "\"";
	}
}

void PEFoxLoader::ParserBlock(std::istream& input)
{
	auto skipWhitespace = [&](std::istream& in)
	{
		while (std::isspace(in.peek())) in.get();
	};

	auto readQuotedString = [&](std::istream& in) -> std::string 
	{
		skipWhitespace(in);
		if (in.get() != '"') return {};
		std::string result;
		char c;
		while (in.get(c))
		{
			if (c == '"') break;
			result += c;
		}
		return result;
	};

	skipWhitespace(input);
	if (input.peek() != '{') return;
	input.get(); // take '{'

	while (true)
	{
		skipWhitespace(input);
		if (input.peek() == '}')
		{
			input.get(); // take '}'
			break;
		}

		std::string key = readQuotedString(input);

		skipWhitespace(input);
		if (input.get() != ':') return;

		skipWhitespace(input);
		if (input.peek() == '{')
		{
			PEFoxLoader child;
			child.ParserBlock(input);
			m_childerns[key] = std::move(child);
		}
		else if (input.peek() == '"')
		{
			std::string value = readQuotedString(input);
			m_childerns[key].SetValue(value);
		}

		skipWhitespace(input);
		if (input.peek() == ',')
		{
			input.get(); //~ take ','
			continue;
		}
		else if (input.peek() == '}')
		{
			continue; //~ will be taken later
		}
		else
		{
			break; //~ invalid!
		}
	}
}