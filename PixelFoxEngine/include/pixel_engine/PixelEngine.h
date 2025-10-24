#pragma once
#include "PixelFoxEngineAPI.h"#

#include "pixel_engine/core/dependency/dependency_resolver.h"
#include "pixel_engine/render_manager/render_manager.h"
#include "pixel_engine/window_manager/windows_manager.h"

#include "pixel_engine/utilities/logger/logger.h"
#include "pixel_engine/utilities/clock/clock.h"

#include <memory>
#include <sal.h>

namespace pixel_engine
{
	typedef struct _PIXEL_ENGINE_CONSTRUCT_DESC
	{
		WINDOW_CREATE_DESC const* WindowsDesc;
	} PFE_API PIXEL_ENGINE_CONSTRUCT_DESC;

	//~ data to send to application (init time)
	typedef struct _PIXEL_ENGINE_INIT_DESC
	{

	} PFE_API PIXEL_ENGINE_INIT_DESC;

	typedef struct _PIXEL_ENGINE_EXECUTE_DESC
	{

	} PFE_API PIXEL_ENGINE_EXECUTE_DESC;

	class PFE_API PixelEngine
	{
	public:
		PixelEngine(PIXEL_ENGINE_CONSTRUCT_DESC const* desc);
		~PixelEngine();

		_NODISCARD _Check_return_  _Success_(return != false)
		bool	Init(PIXEL_ENGINE_INIT_DESC const* desc);

		_Success_(return == S_OK)
		HRESULT Execute(PIXEL_ENGINE_EXECUTE_DESC const* desc);

	protected:
		//~ Application Must Implement them
		virtual bool InitApplication(PIXEL_ENGINE_INIT_DESC const* desc) = 0;
		virtual void BeginPlay()										 = 0;
		virtual void Tick(float deltaTime)								 = 0;
		virtual void Release()											 = 0;

	private:
		bool CreateManagers(PIXEL_ENGINE_CONSTRUCT_DESC const* desc);

		void CreateUtilities();
		void SetManagerDependency();

	private:
		DependencyResolver				  m_dependecyResolver{};
		
		std::unique_ptr<GameClock>		  m_pClock		   { nullptr };
		std::unique_ptr<PEWindowsManager> m_pWindowsManager{ nullptr };
		std::unique_ptr<PERenderManager>  m_pRenderManager { nullptr };
	};
} // namespace pixel_engine
