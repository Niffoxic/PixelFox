#pragma once
#include "PixelFoxEngineAPI.h"
#include "pixel_engine/core/interface/interface_manager.h"
#include "core/unordered_map.h"
#include "pixel_engine/core/event/event_queue.h"



namespace pixel_engine
{
	typedef struct _RENDERER_CREATE_DESC
	{

	} RENDER_CREATE_DESC;

	class PFE_API PERenderManager final: public IManager
	{
	public:
		PERenderManager() = default;
		explicit PERenderManager(RENDER_CREATE_DESC const* desc){}

		//~ interface implementation
		_NODISCARD _Check_return_ bool OnInit() override;
		_NODISCARD _Check_return_ bool OnRelease() override;

		_NODISCARD _Check_return_ __forceinline
			std::string GetManagerName() const override { return "RenderManager"; }

		void OnLoopStart(_In_ float deltaTime) override;
		void OnLoopEnd() override;

	private:
		void SubscribeToEvents();
		void UnSubscribeToEvents();

	private:
		fox::vector<SubToken> m_eventTokens{};
	};
}
