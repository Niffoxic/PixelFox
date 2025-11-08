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

	slice.scaledBy = { 0.25f, 0.25f };
	m_tileSet_8 = TileSetAllocator::Instance().BuildTexture(slice);
	slice.scaledBy = { 0.5f, 0.5f };
	m_tileSet_16 = TileSetAllocator::Instance().BuildTexture(slice);
	slice.scaledBy = { 1.f, 1.f };
	m_tileSet_32 = TileSetAllocator::Instance().BuildTexture(slice);
	slice.scaledBy = { 2.f, 2.f };
	m_tileSet_64 = TileSetAllocator::Instance().BuildTexture(slice);
	

	if (!m_tileSet_8 || !m_tileSet_16 || !m_tileSet_32 || !m_tileSet_64)
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
Texture* FontGenerator::GetGlyph(char c, int px)
{
	if (!m_fonts.contains(c)) return nullptr;

	TileSet* ts = nullptr;
	switch (px)
	{
	case 8:  ts = m_tileSet_8;  break;
	case 16: ts = m_tileSet_16; break;
	case 32: ts = m_tileSet_32; break;
	case 64: ts = m_tileSet_64; break;
	default: ts = m_tileSet_32; break;
	}

	if (!ts) return nullptr;

	return ts->GetSlice(m_fonts[c]);
}

_Use_decl_annotations_
fox::vector<Texture*> FontGenerator::GetGlyphs(const std::string& text, int px)
{
	fox::vector<Texture*> result;
	result.reserve(text.size());

	for (char c : text)
	{
		Texture* tex = GetGlyph(c);
		if (not tex) continue;
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
