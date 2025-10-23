#include "pch.h"
#include "PixelEngine.h"

#include "pixel_engine/exceptions/base_exception.h"
#include "pixel_engine/core/event/event_queue.h"


// TODO: Create Timer(clock).

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
	logger::info("Starting Game Loop!");
	BeginPlay();
	while (true)
	{
		if (PEWindowsManager::ProcessMessage())
		{
			logger::warning("Closing Application!");
			m_dependecyResolver.Shutdown();
			logger::close();
			return S_OK;
		}
		logger::info("running!");
	
		m_dependecyResolver.UpdateLoopStart(0.f);
		Tick(0.0f);
		m_dependecyResolver.UpdateLoopEnd();

		EventQueue::DispatchAll();
	}
	return S_OK;
}

bool pixel_engine::PixelEngine::CreateManagers(PIXEL_ENGINE_CONSTRUCT_DESC const* desc)
{
	m_pWindowsManager = std::make_unique<PEWindowsManager>(desc->WindowsDesc);
	m_pRenderManager  = std::make_unique<PERenderManager>(desc->RenderDesc);

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

