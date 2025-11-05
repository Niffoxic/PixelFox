#pragma once

#include "PixelFoxEngineAPI.h"
#include <string>

#include "pixel_engine/core/interface/interface_object.h"
#include "pixel_engine/render_manager/components/texture/resource/texture.h"

#include "fox_math/vector.h"
#include "core/vector.h"

namespace pixel_engine
{
	typedef struct _FONT_POSITION
	{
		FVector2D startPosition;
		Texture*  sampledTexture;
	} FONT_POSITION;

	class PFE_API PEFont final: public PEIObject
	{
	public:


	public:
		PEFont() = default;
		~PEFont() override = default;

		//~ object interface
		_NODISCARD _Check_return_
		std::string GetObjectName() const override { return "Font"; }

		_NODISCARD _Check_return_
		bool Initialize() override { return true; }
		bool Release() override { return true; }

		//~ font config
		void SetText(_In_ const std::string& text);
		std::string GetText() const;

		void SetPosition(const FVector2D& pos);
		FVector2D GetPosition() const;

		void SetScale(const FVector2D& scale);
		FVector2D GetScale() const;

		const fox::vector<FONT_POSITION>& GetFontTextures() const;

	private:
		FVector2D   m_position{ 0, 0 };
		FVector2D   m_scale   { 0, 0 };
		std::string m_text    { "" };
		fox::vector<FONT_POSITION> m_ppFontTextures;
	};
} // namespace
