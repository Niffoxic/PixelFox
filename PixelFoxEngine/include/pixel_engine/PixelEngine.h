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
		_In_ WINDOW_CREATE_DESC const* WindowsDesc;
	} PFE_API PIXEL_ENGINE_CONSTRUCT_DESC;

	typedef struct _PIXEL_ENGINE_INIT_DESC
	{

	} PFE_API PIXEL_ENGINE_INIT_DESC;

	typedef struct _PIXEL_ENGINE_EXECUTE_DESC
	{

	} PFE_API PIXEL_ENGINE_EXECUTE_DESC;

	class PFE_API PixelEngine
	{
	public:
		 PixelEngine(_In_ PIXEL_ENGINE_CONSTRUCT_DESC const* desc);
		~PixelEngine();

		_NODISCARD _Check_return_ _Success_(return != false)
		bool Init(_In_ PIXEL_ENGINE_INIT_DESC const* desc);

		_Success_(return == S_OK)
		HRESULT Execute(_In_ PIXEL_ENGINE_EXECUTE_DESC const* desc);

	protected:
		//~ Application Must Implement them
		_NODISCARD _Check_return_
		virtual bool InitApplication(_In_ PIXEL_ENGINE_INIT_DESC const* desc) = 0;
		
		virtual void BeginPlay()					 = 0;
		virtual void Tick     (_In_ float deltaTime) = 0;
		virtual void Release ()						 = 0;

	private:
		_NODISCARD _Check_return_
		bool CreateManagers(_In_ PIXEL_ENGINE_CONSTRUCT_DESC const* desc);

		void CreateUtilities	 ();
		void SetManagerDependency();
		void SubscribeToEvents	 ();

	protected:
		std::unique_ptr<GameClock>		  m_pClock{ nullptr };
		std::unique_ptr<PEWindowsManager> m_pWindowsManager{ nullptr };
		std::unique_ptr<PERenderManager>  m_pRenderManager{ nullptr };

	private:
		bool			   m_bEnginePaused	  { false };
		DependencyResolver m_dependecyResolver{};
	};
} // namespace pixel_engine
