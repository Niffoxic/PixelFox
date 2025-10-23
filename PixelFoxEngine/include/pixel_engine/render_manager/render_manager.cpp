#include "pch.h"
#include "render_manager.h"

#include "pixel_engine/core/event/event_windows.h"
#include "pixel_engine/utilities/logger/logger.h"



_Use_decl_annotations_
pixel_engine::PERenderManager::PERenderManager(PEWindowsManager* windows)
    : m_pWindowsManager(windows)
{
}

_Use_decl_annotations_
bool pixel_engine::PERenderManager::OnInit()
{
    logger::warning(pixel_engine::logger_config::LogCategory::Render,
        "Attempting to initialize RenderManager");

    SubscribeToEvents();

    if (not CreateDeviceAndDeviceContext()) return false;
    if (not CreateSwapChain())              return false;
    if (not CreateRTV())                    return false;
    if (not CreateVertexShader())           return false;
    if (not CreatePixelShader())            return false;
    if (not CreateViewport())               return false;

    logger::success(pixel_engine::logger_config::LogCategory::Render,
        "initialized RenderManager");

    return true;
}

_Use_decl_annotations_
bool pixel_engine::PERenderManager::OnRelease()
{
    UnSubscribeToEvents();
	return true;
}

_Use_decl_annotations_
void pixel_engine::PERenderManager::OnLoopStart(float deltaTime)
{
}

void pixel_engine::PERenderManager::OnLoopEnd()
{
}

bool pixel_engine::PERenderManager::CreateDeviceAndDeviceContext()
{
    UINT createFlags = 0;
#if defined (DEBUG) || defined(_DEBUG)
    createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = 
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    D3D_FEATURE_LEVEL availableLevel = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_pDevice,
        &availableLevel,
        &m_pDeviceContext
    );

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to Create Device!");
        return false;
    }

    logger::success(pixel_engine::logger_config::LogCategory::Render,
        "Created Device and Context!");

    return true;
}

bool pixel_engine::PERenderManager::CreateSwapChain()
{
    Microsoft::WRL::ComPtr<IDXGIDevice> m_pDxgiDevice;
    HRESULT hr = m_pDevice.As(&m_pDxgiDevice);

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to Translate Device as a dxgi device!");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter> m_pAdapter;
    hr = m_pDxgiDevice->GetAdapter(&m_pAdapter);

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to Get Adapter whcih used to create device and context!");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory> m_pFactory;
    hr = m_pAdapter->GetParent(__uuidof(IDXGIFactory),
        reinterpret_cast<void**>(m_pFactory.GetAddressOf()));


    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to Get factory whcih used to create adapter!");
        return false;
    }

    //~ Creates Swapchain
    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount        = 1u;
    desc.OutputWindow       = m_pWindowsManager->GetWindowsHandle();
    desc.SampleDesc.Count   = 1u;
    desc.SampleDesc.Quality = 0u;
    desc.Windowed           = m_pWindowsManager->IsFullScreen() ? FALSE: TRUE;
    desc.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;
    desc.Flags              = 0u;
    desc.BufferDesc.Width   = m_pWindowsManager->GetWindowsWidth();
    desc.BufferDesc.Height  = m_pWindowsManager->GetWindowsHeight();
    desc.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferDesc.RefreshRate.Numerator   = 60u;
    desc.BufferDesc.RefreshRate.Denominator = 1u;
    

    hr = m_pFactory->CreateSwapChain(m_pDevice.Get(), &desc, m_pSwapchain.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to Create Swapchain");
        return false;
    }

    logger::success(pixel_engine::logger_config::LogCategory::Render,
        "Created Swapchain!");

    return true;
}

bool pixel_engine::PERenderManager::CreateRTV()
{
    return true;
}

bool pixel_engine::PERenderManager::CreateVertexShader()
{
    return true;
}

bool pixel_engine::PERenderManager::CreatePixelShader()
{
    return true;
}

bool pixel_engine::PERenderManager::CreateViewport()
{
    return true;
}

void pixel_engine::PERenderManager::SubscribeToEvents()
{
    auto token = EventQueue::Subscribe<WINDOW_RESIZE_EVENT>([](const WINDOW_RESIZE_EVENT& event)
    {
        logger::info(logger_config::LogCategory::Editor,
            "event: Recevied by RenderManager - Windows Resize Event");
            
    });
    m_eventTokens.push_back(token);

    token = EventQueue::Subscribe<FULL_SCREEN_EVENT>([](const FULL_SCREEN_EVENT& event)
    {
        logger::info(logger_config::LogCategory::Editor,
            "event: Recevied by RenderManager - Full Screen Event");
    });
    m_eventTokens.push_back(token);

    token = EventQueue::Subscribe<WINDOWED_SCREEN_EVENT>([](const WINDOWED_SCREEN_EVENT& event)
    {
        logger::info(logger_config::LogCategory::Editor,
            "event: Recevied by RenderManager - Windowed Screen Event");
    });
    m_eventTokens.push_back(token);
}

void pixel_engine::PERenderManager::UnSubscribeToEvents()
{
    for (int i = 0; i < m_eventTokens.size(); i++)
    {
        EventQueue::Unsubscribe(m_eventTokens[i]);
    }
}
