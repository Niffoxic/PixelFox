#pragma once
#include "PixelFoxEngineAPI.h"
#include "core/unordered_map.h"

#include "pixel_engine/core/interface/interface_frame.h"
#include "pixel_engine/core/event/event_queue.h"
#include "pixel_engine/window_manager/windows_manager.h"
#include "pixel_engine/utilities/clock/clock.h"

#include "api/render_api.h"
#include <memory>

namespace pixel_engine
{
	class PFE_API PERenderManager final: public IFrameObject
	{
	public:
		 PERenderManager(
			 _In_ PEWindowsManager* windows,
			 _In_ GameClock* clock);

		~PERenderManager();

		//~ interface implementation
		_NODISCARD _Check_return_ bool Initialize() override;
		_NODISCARD _Check_return_ bool Release() override;

		_NODISCARD _Check_return_
		std::string GetObjectName() const override { return "RenderManager"; }

		void OnFrameBegin(_In_ float deltaTime) override;
		void OnFrameEnd  () override;

	private:
		bool InitializeCamera2D ();
		bool InitializeRenderAPI();

		void SubscribeToEvents  ();
		void UnSubscribeToEvents();

		//~ Helpers
		void SafeCloseEvent_(HANDLE& h);

		//~ tests
		void HandleCameraInput(float deltaTime);

	private:
		PEWindowsManager*	  m_pWindowsManager { nullptr };
		GameClock*			  m_pClock			{ nullptr };
		fox::vector<SubToken> m_eventTokens	    {};

		//~ Manage render api
		std::unique_ptr<PERenderAPI> m_pRenderAPI{ nullptr };
		std::unique_ptr<Camera2D> m_pCamera		 { nullptr };

		HANDLE m_handleStartEvent{ nullptr };
		HANDLE m_handleEndEvent  { nullptr };
		HANDLE m_handleThread    { nullptr };
	};
}
