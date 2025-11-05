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
#include "font_allocator.h"

using namespace pixel_engine;

FontGenerator::FontGenerator()
{
	PFE_CREATE_TILE_SET_FROM_SLICE slice{};
	slice.tileFilePath = "assets/font.png";
	slice.tileSize    = 32;
	slice.sliceWidth  = 32;
	slice.sliceHeight = 32;
	slice.scaledBy = { 1, 1 };

	m_tileSet = TileSetAllocator::Instance().BuildTexture(slice);
	if (!m_tileSet)
	{
		logger::error("FontGenerator: failed to load '{}'", slice.tileFilePath);
		return;
	}

	static constexpr const char* kGlyphs =
		R"(!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)";

	SetGlyphOrder(kGlyphs);
	logger::debug("FontGenerator initialized successfully with {} glyphs", static_cast<int>(m_fonts.size()));
}

_Use_decl_annotations_
Texture* FontGenerator::GetGlyph(char c)
{
	if (!m_tileSet) return nullptr;

	if (!m_fonts.contains(c)) return nullptr;

	return m_tileSet->GetSlice(m_fonts[c]);
}

_Use_decl_annotations_
fox::vector<Texture*> FontGenerator::GetGlyphs(const std::string& text)
{
	fox::vector<Texture*> result;
	result.reserve(text.size());

	for (char c : text)
	{
		Texture* tex = GetGlyph(c);

		if (not tex)
		{
			tex = m_tileSet->GetSlice(39); // Error Display
		}

		result.push_back(tex);
	}

	return result;
}

void FontGenerator::SetGlyphOrder(const std::string& glyphs)
{
	m_fonts.clear();
	int index = 0;
	for (char c : glyphs)
		m_fonts[c] = index++;
}
