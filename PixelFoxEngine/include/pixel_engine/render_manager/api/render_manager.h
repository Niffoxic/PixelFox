#pragma once
#include "PixelFoxEngineAPI.h"
#include "pixel_engine/system_manager/interface/interface_manager.h"


namespace pixel_engine
{
	typedef struct _RENDERER_CREATE_DESC
	{

	} RENDER_CREATE_DESC;

	class PFE_API RenderManager final: public IManager
	{
	public:
		RenderManager() = default;
		explicit RenderManager(const RENDER_CREATE_DESC& desc){}
	};
}
