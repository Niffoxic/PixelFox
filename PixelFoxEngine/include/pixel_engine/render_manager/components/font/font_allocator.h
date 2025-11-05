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
		Texture* GetGlyph(char c);

		_NODISCARD _Use_decl_annotations_
		fox::vector<Texture*> GetGlyphs(const std::string& text);

	private:
		void SetGlyphOrder(const std::string& glyphs);

	private:
		TileSet* m_tileSet{ nullptr };
		fox::unordered_map<char, int> m_fonts;
	};
}
