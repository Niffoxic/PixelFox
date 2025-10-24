#pragma once
#include "PixelFoxEngineAPI.h"
#include "core/unordered_map.h"

#include "pixel_engine/core/interface/interface_frame.h"
#include "pixel_engine/core/event/event_queue.h"
#include "pixel_engine/window_manager/windows_manager.h"

#include "api/render_api.h"
#include <memory>

namespace pixel_engine
{
	class PFE_API PERenderManager final: public IFrameObject
	{
	public:
		 PERenderManager(_In_ PEWindowsManager* windows);
		~PERenderManager() = default;

		//~ interface implementation
		_NODISCARD _Check_return_ bool Initialize() override;
		_NODISCARD _Check_return_ bool Release() override;

		_NODISCARD _Check_return_
		std::string GetObjectName() const override { return "RenderManager"; }

		void OnFrameBegin(_In_ float deltaTime) override;
		void OnFrameEnd  () override;

	private:
		void SubscribeToEvents  ();
		void UnSubscribeToEvents();

	private:
		PEWindowsManager*			 m_pWindowsManager{ nullptr };
		std::unique_ptr<PERenderAPI> m_pRenderAPI	  { nullptr };
		fox::vector<SubToken>		 m_eventTokens	  {};
	};
}
