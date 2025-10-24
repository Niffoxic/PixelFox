#include "pch.h"
#include "PixelEngine.h"

#include "pixel_engine/exceptions/base_exception.h"
#include "pixel_engine/core/event/event_queue.h"

#include <sstream>

pixel_engine::PixelEngine::PixelEngine(PIXEL_ENGINE_CONSTRUCT_DESC const* desc)
{
	CreateManagers(desc);
	CreateUtilities();
	SetManagerDependency();
}

pixel_engine::PixelEngine::~PixelEngine()
{
	if (not m_dependecyResolver.Shutdown())
	{
		logger::error("Failure Detected at the time of deleting managers!");
	}
	logger::close();
}

_Use_decl_annotations_
bool pixel_engine::PixelEngine::Init(PIXEL_ENGINE_INIT_DESC const* desc)
{
	if (not m_dependecyResolver.Init())
	{
		logger::error("Failed to initialize manager!");
		THROW_MSG("Failed to initialize manager!");
		return false;
	}

	if (not InitApplication(desc))
	{
		logger::error("Failed to initialize application!");
		THROW_MSG("Failed to initialize application!");
		return false;
	}

	return true;
}

_Use_decl_annotations_
HRESULT pixel_engine::PixelEngine::Execute(PIXEL_ENGINE_EXECUTE_DESC const* desc)
{
	m_clock.reset_clock();
	logger::info("Starting Game Loop!");
	BeginPlay();
	while (true)
	{
		float dt = m_clock.tick();
		if (PEWindowsManager::ProcessMessage())
		{
			logger::warning("Closing Application!");
			m_dependecyResolver.Shutdown();
			logger::close();
			return S_OK;
		}
	
		m_dependecyResolver.UpdateLoopStart(dt);
		Tick(dt);
		m_dependecyResolver.UpdateLoopEnd();

#if defined(DEBUG) || defined(_DEBUG)
		static float passed				= 0.0f;
		static int   frame				= 0;
		static float avg_frames			= 0.0f;
		static float last_time_elapsed	= 0.0f;

		frame++;
		passed += dt;

		if (passed >= 1.0f)
		{
			avg_frames	     += frame;
			last_time_elapsed = m_clock.time_elapsed();

			std::string message =
				"Time Elapsed: "								+ 
				std::to_string(last_time_elapsed)				+
				" Frame Rate: "									+ 
				std::to_string(frame)							+
				" per second (Avg = "							+ 
				std::to_string(avg_frames / last_time_elapsed)	+
				")";

			m_pWindowsManager->SetWindowMessageOnTitle(message);

			passed = 0.0f;
			frame  = 0;
		}
#endif
		EventQueue::DispatchAll();
	}
	return S_OK;
}

bool pixel_engine::PixelEngine::CreateManagers(PIXEL_ENGINE_CONSTRUCT_DESC const* desc)
{
	m_pWindowsManager = std::make_unique<PEWindowsManager>(desc->WindowsDesc);
	m_pRenderManager  = std::make_unique<PERenderManager>(m_pWindowsManager.get());

	return true;
}

void pixel_engine::PixelEngine::CreateUtilities()
{
	LOGGER_CREATE_DESC cfg{};
	cfg.TerminalName			= "PixelFox Logger";
	cfg.EnableTerminal			= true;
	cfg.EnableAnsiTrueColor		= true;
	cfg.DuplicateToDebugger		= true;
	cfg.ShowTimestamps			= true;
	cfg.ShowThreadId			= true;
	cfg.ShowFileAndLine			= true;
	cfg.ShowFunction			= true;
	cfg.UseUtcTimestamps		= false;
	cfg.UseRelativeTimestamps	= false;
	cfg.MinimumLevel			= logger_config::LogLevel::Trace;

	logger::init(cfg);
}

void pixel_engine::PixelEngine::SetManagerDependency()
{
	m_dependecyResolver.Register(m_pWindowsManager.get());
	m_dependecyResolver.Register(m_pRenderManager.get());

	m_dependecyResolver.AddDependency(
		m_pRenderManager.get(),
		m_pWindowsManager.get()
	);
}

