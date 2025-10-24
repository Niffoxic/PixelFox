#include "pch.h"
#include "render_manager.h"

#include "pixel_engine/core/event/event_windows.h"
#include "pixel_engine/utilities/logger/logger.h"

// TODO: Upload to GPU
// via m_pDeviceContext->UpdateSubresource
// (m_pBackBuffer.Get(), 0, nullptr, cpuBytes, 0, 0);

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
    const float clear[4] = { 0.72f, 0.23f, 0.62f, 1.0f };
    m_pDeviceContext->ClearRenderTargetView(m_pRTV.Get(), clear);
}

void pixel_engine::PERenderManager::OnLoopEnd()
{
    m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);
    m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0u);
    m_pDeviceContext->Draw(3, 0);
    m_pSwapchain->Present(1u, 0u);
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
    Microsoft::WRL::ComPtr<ID3D11Texture2D> back_texture{ nullptr };
    HRESULT hr = m_pSwapchain->GetBuffer(0,
        __uuidof(ID3D11Texture2D), reinterpret_cast<void**>
        (back_texture.GetAddressOf()));

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to acquire back buffer texture");
        return false;
    }

    hr = m_pDevice->CreateRenderTargetView(
        back_texture.Get(),
        nullptr,
        m_pRTV.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create RTV");
        return false;
    }

    //~ Set states once since only 1 texture will be rendering pixels
    m_pDeviceContext->OMSetRenderTargets(1, m_pRTV.GetAddressOf(), nullptr);
    m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
    m_pDeviceContext->RSSetState(nullptr);
    m_pDeviceContext->IASetInputLayout(nullptr);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return true;
}

bool pixel_engine::PERenderManager::CreateVertexShader()
{
    //~ Fullscreen quad
    static const char* kVS = R"(
        struct VSOut {
            float4 pos : SV_Position;
        };
        VSOut VS(uint vertexID : SV_VertexId)
        {
            VSOut o;
            float2 tex = float2((vertexID << 1) & 2, vertexID & 2);
            o.pos = float4(tex * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
            return o;
        }
    )";

    Microsoft::WRL::ComPtr<ID3DBlob> vsblob, err;

    HRESULT hr = D3DCompile(kVS, strlen(kVS),
        nullptr, nullptr, nullptr, "VS", "vs_5_0",
        0, 0,
        vsblob.GetAddressOf(), err.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create vertex blob");
        return false;
    }

    hr = m_pDevice->CreateVertexShader(vsblob->GetBufferPointer(),
        vsblob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create vertex shader");
        return false;
    }
    logger::success(pixel_engine::logger_config::LogCategory::Render,
        "Created Vertex Shader!");

    m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);

    return true;
}

bool pixel_engine::PERenderManager::CreatePixelShader()
{
    const char* psSrc = R"(
    ByteAddressBuffer buf : register(t0);
    struct VSOut { float4 pos : SV_Position; };
    float4 PS(VSOut i) : SV_Target
    {
        int px = clamp(int(i.pos.x), 0, WIDTH-1);
        int py = clamp(int(i.pos.y), 0, HEIGHT-1);
        uint pixelIndex = py * WIDTH + px;

        uint offset = pixelIndex * 3;
        uint inner = offset & 3;
        uint base  = offset & ~3;

        uint data;
        if (inner == 0)
        {
            data = buf.Load(base) & 0x00FFFFFF;
        }
        else
        {
            uint lo = buf.Load(base);
            uint hi = buf.Load(base + 4);
            data = ((lo >> (inner * 8)) | (hi << ((4 - inner) * 8))) & 0x00FFFFFF;
        }

        float r = ( data        & 0xFF) / 255.0;
        float g = ((data >>  8) & 0xFF) / 255.0;
        float b = ((data >> 16) & 0xFF) / 255.0;
        return float4(r,g,b,1.0);
    }
)";

    D3D_SHADER_MACRO macros[] =
    {
        { "WIDTH",  std::to_string(m_pWindowsManager->GetWindowsWidth()).c_str() },
        { "HEIGHT", std::to_string(m_pWindowsManager->GetWindowsHeight()).c_str() },
        { nullptr, nullptr }
    };

    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> psBlob, err;
    HRESULT hr = D3DCompile(
        psSrc, strlen(psSrc),
        nullptr,
        macros,
        nullptr,
        "PS", "ps_5_0",
        flags, 0,
        psBlob.GetAddressOf(),
        err.GetAddressOf()
    );

    if (FAILED(hr))
    {
        if (err) 
        {
            // TODO: figure out how to print error message
            logger::error(pixel_engine::logger_config::LogCategory::Render,
                "error message occured");
        }
        else
        {
            logger::error(pixel_engine::logger_config::LogCategory::Render,
                "PS compile failed (no error blob)");
        }
        return false;
    }

    hr = m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create pixel shader");
        return false;
    }

    logger::success(pixel_engine::logger_config::LogCategory::Render,
        "Created Pixel Shader!");

    //~ configure backbuffer as a texture
    const size_t dataSize = 
        static_cast<size_t>(m_pWindowsManager->GetWindowsWidth()) *
        static_cast<size_t>(m_pWindowsManager->GetWindowsHeight()) * 3u;

    m_PaddedDataSize = ((dataSize + 3u) / 4u) * 4u;

    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth      = static_cast<UINT>(m_PaddedDataSize);
    desc.Usage          = D3D11_USAGE_DEFAULT;
    desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0u;
    desc.MiscFlags      = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    m_pBackBuffer.Reset();
    hr = m_pDevice->CreateBuffer(&desc, nullptr, m_pBackBuffer.GetAddressOf());
    if (FAILED(hr)) 
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create shader resource on backbuffer");
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
    srvd.ViewDimension         = D3D11_SRV_DIMENSION_BUFFEREX;
    srvd.Format                = DXGI_FORMAT_R32_TYPELESS;
    srvd.BufferEx.FirstElement = 0;
    srvd.BufferEx.NumElements  = static_cast<UINT>(m_PaddedDataSize / 4u);
    srvd.BufferEx.Flags        = D3D11_BUFFEREX_SRV_FLAG_RAW;

    m_pSRV.Reset();
    hr = m_pDevice->CreateShaderResourceView(
         m_pBackBuffer.Get(),
         &srvd,
         m_pSRV.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create shader resource view outta backbuffer");
        return false;
    }

    //~ Bind PS and SRV
    m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0u);

    ID3D11ShaderResourceView* srvs[] = { m_pSRV.Get() };
    m_pDeviceContext->PSSetShaderResources(0u, 1u, srvs);

    logger::success(
        pixel_engine::logger_config::LogCategory::Render,
        "Initialized and Configured Pixel Shader!");

#pragma region TEST_REGION
    const UINT w = m_pWindowsManager->GetWindowsWidth();
    const UINT h = m_pWindowsManager->GetWindowsHeight();
    const size_t testDataSize = size_t(w) * size_t(h) * 3u;
    const size_t padded = (testDataSize + 3u) & ~size_t(3);

    std::vector<unsigned char> cpu(padded, 0);
    for (UINT y = 0; y < h; ++y)
    {
        for (UINT x = 0; x < w; ++x)
        {
            size_t i = (size_t(y) * w + x) * 3u;
            cpu[i + 0] = (unsigned char)(255.0f * (float)x / (float)w);
            cpu[i + 1] = (unsigned char)(255.0f * (float)y / (float)h);
            cpu[i + 2] = 101;                                           
        }
    }
    m_pDeviceContext->UpdateSubresource(m_pBackBuffer.Get(), 0, nullptr, cpu.data(), 0, 0);
#pragma endregion
    return true;
}

bool pixel_engine::PERenderManager::CreateViewport()
{
    m_Viewport.Width    = static_cast<FLOAT>(m_pWindowsManager->GetWindowsWidth());
    m_Viewport.Height   = static_cast<FLOAT>(m_pWindowsManager->GetWindowsHeight());
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;
    m_Viewport.TopLeftX = 0.0f;
    m_Viewport.TopLeftY = 0.0f;

    m_pDeviceContext->RSSetViewports    (1, &m_Viewport);
    m_pDeviceContext->OMSetRenderTargets(1, m_pRTV.GetAddressOf(), nullptr);

    logger::success(
        pixel_engine::logger_config::LogCategory::Render,
        "Initialized and Configured Viewport");

    return true;
}

void pixel_engine::PERenderManager::SubscribeToEvents()
{
    auto token = EventQueue::Subscribe<WINDOW_RESIZE_EVENT>([](const WINDOW_RESIZE_EVENT& event)
    {
        logger::info(logger_config::LogCategory::Render,
            "event: Recevied by RenderManager - Windows Resize Event");
    });
    m_eventTokens.push_back(token);

    token = EventQueue::Subscribe<FULL_SCREEN_EVENT>([](const FULL_SCREEN_EVENT& event)
    {
        logger::info(logger_config::LogCategory::Render,
            "event: Recevied by RenderManager - Full Screen Event");
    });
    m_eventTokens.push_back(token);

    token = EventQueue::Subscribe<WINDOWED_SCREEN_EVENT>([](const WINDOWED_SCREEN_EVENT& event)
    {
        logger::info(logger_config::LogCategory::Render,
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
