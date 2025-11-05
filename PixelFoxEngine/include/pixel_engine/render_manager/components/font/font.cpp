#include "pch.h"
#include "font.h"

#include "font_allocator.h"

_Use_decl_annotations_
void pixel_engine::PEFont::SetText(const std::string& text)
{
	if (m_text == text) return;

	m_text = text;
	m_ppFontTextures.clear();

	auto textures = FontGenerator::Instance().GetGlyphs(text);

	FVector2D startPosition = m_position;

	for (auto* tex : textures)
	{
		if (!tex) continue;

		FONT_POSITION fp{};
		fp.startPosition = startPosition;
		fp.sampledTexture = tex;

		m_ppFontTextures.push_back(fp);
		startPosition.x += tex->GetWidth();
	}
}

std::string pixel_engine::PEFont::GetText() const
{
	return m_text;
}

void pixel_engine::PEFont::SetPosition(const FVector2D& pos)
{
	FVector2D delta = pos - m_position;
	m_position		= pos;

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
