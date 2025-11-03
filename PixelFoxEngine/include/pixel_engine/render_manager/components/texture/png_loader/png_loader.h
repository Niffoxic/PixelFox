#pragma once

#include "PixelFoxEngineAPI.h"

#include "core/unordered_map.h"
#include "core/vector.h"

#include "pixel_engine/utilities/logger/logger.h"
#include "pixel_engine/core/interface/interface_singleton.h"
#include "pixel_engine/render_manager/components/texture/texture.h"

namespace pixel_engine
{
	class PFE_API PNGLoader final: public ISingleton<PNGLoader>
	{
		friend class ISingleton<PNGLoader>;
	public:
		PNGLoader() = default;
		std::unique_ptr<Texture> LoadTexture(const std::string& path);
	};
} // namespace pixel_engine
