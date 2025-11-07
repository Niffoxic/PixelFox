// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com


/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#include "pch.h"
#include "font.h"

#include "font_allocator.h"

_Use_decl_annotations_
void pixel_engine::PEFont::SetText(const std::string& text)
{
	if (m_text == text) return;

	m_text = text;
	m_ppFontTextures.clear();

	auto& fg = FontGenerator::Instance();

	auto tex			    = fg.GetGlyph('A', m_nPx);
	int width			    = tex->GetWidth();
	FVector2D startPosition = m_position;

	float lastWidth = 0.0f;

	for (char c : text)
	{
		if (c == ' ')
		{
			const float base = (lastWidth > 0.0f) ? lastWidth : width;
			startPosition.x += base * 0.5f;
			continue;
		}

		Texture* tex = fg.GetGlyph(c, m_nPx);
		if (!tex) continue;

		FONT_POSITION fp{};
		fp.startPosition  = startPosition;
		fp.sampledTexture = tex;
		m_ppFontTextures.push_back(fp);

		lastWidth = static_cast<float>(tex->GetWidth());
		startPosition.x += lastWidth;
	}
}

std::string pixel_engine::PEFont::GetText() const
{
	return m_text;
}

void pixel_engine::PEFont::SetPosition(const FVector2D& pos)
{
	FVector2D delta = pos - m_position;
	m_position = pos;

	for (auto& fp : m_ppFontTextures)
		fp.startPosition += delta;
}

FVector2D pixel_engine::PEFont::GetPosition() const
{
	return m_position;
}

void pixel_engine::PEFont::SetScale(const FVector2D& scale)
{
	m_scale = scale;
}

FVector2D pixel_engine::PEFont::GetScale() const
{
	return m_scale;
}

const fox::vector<pixel_engine::FONT_POSITION>& pixel_engine::PEFont::GetFontTextures() const
{
	return m_ppFontTextures;
}
