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

#include <string>
#include <cctype>

#include "pixel_engine/core/interface/interface_singleton.h"
#include "pixel_engine/render_manager/components/texture/allocator/tileset_allocator.h"
#include "pixel_engine/utilities/logger/logger.h"

#include "fox_math/vector.h"

namespace pixel_engine
{
	class PFE_API FontGenerator final : public ISingleton<FontGenerator>
	{
		friend ISingleton<FontGenerator>;

	public:
		FontGenerator();

		_NODISCARD _Use_decl_annotations_
		Texture* GetGlyph(char c, int px=8);

		_NODISCARD _Use_decl_annotations_
		fox::vector<Texture*> GetGlyphs(const std::string& text, int px = 8);

	private:
		void SetGlyphOrder(const std::string& glyphs);

	private:
		TileSet* m_tileSet_64{ nullptr };
		TileSet* m_tileSet_32{ nullptr };
		TileSet* m_tileSet_16{ nullptr };
		TileSet* m_tileSet_8{ nullptr };
		fox::unordered_map<char, int> m_fonts;
	};
}
