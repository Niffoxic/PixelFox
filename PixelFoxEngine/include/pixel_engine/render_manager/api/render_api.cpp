// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#include "pch.h"
#include "render_api.h"

#include "pixel_engine/utilities/logger/logger.h" 
#include "core/vector.h"

#include "fox_math/math.h"
#include "fox_math/matrix.h"
#include "fox_math/transform.h"
#include "fox_math/vector.h"
#include <cmath>
#include <array>

#include "pixel_engine/render_manager/render_queue/render_queue.h"

_Use_decl_annotations_
pixel_engine::PERenderAPI::PERenderAPI(const CONSTRUCT_RENDER_API_DESC* desc)
{
    m_handleStartEvent = desc->StartEvent;
    m_handleExitEvent  = desc->ExitEvent;

    m_handlePresentEvent = CreateEvent(
        nullptr,
        FALSE,
        FALSE,
        nullptr
    );

    m_handlePresentDoneEvent = CreateEvent(
        nullptr,
        FALSE,
        FALSE,
        nullptr
    );
}

pixel_engine::PERenderAPI::~PERenderAPI()
{
    if (m_handlePresentEvent)     CloseHandle(m_handlePresentEvent);
    if (m_handlePresentDoneEvent) CloseHandle(m_handlePresentDoneEvent);
}

_Use_decl_annotations_
bool pixel_engine::PERenderAPI::Init(const INIT_RENDER_API_DESC* desc)
{
    m_pCamera = desc->Camera;
    m_pClock = std::make_unique<GameClock>();

    if (not InitializeDirectX(desc))   return false;
    if (not InitializeRenderAPI(desc)) return false;

    return true;
}

DWORD pixel_engine::PERenderAPI::Execute()
{
    logger::info("[RenderThread] Waiting for Start Event...");
    WaitForSingleObject(m_handleStartEvent, INFINITE);
    logger::info("[RenderThread] Start Event received: render loop begin.");

    const HANDLE waits[2] = { m_handleExitEvent, m_handlePresentEvent };
    using clock = std::chrono::steady_clock;
    auto lastReport = clock::now();
    uint64_t framesSinceReport = 0;

    m_pClock->ResetTime();
    while (true)
    {
        float dt = m_pClock->Tick();

        bool showFPS = PERenderQueue::Instance().IsShowFPS();
        if (showFPS && !m_bShowingFPS)
        {
            m_bShowingFPS = true;
            auto pos = PERenderQueue::Instance().GetFPSPosition();
            int px   = PERenderQueue::Instance().GetFPSPx();
            m_fps.SetPosition(pos);
            m_fps.SetPx(px);
            PERenderQueue::Instance().AddFont(&m_fps);
        }
        if (!showFPS && m_bShowingFPS)
        {
            PERenderQueue::Instance().RemoveFont(&m_fps);
            m_bShowingFPS = false;
        } 

        if (m_bShowingFPS)
        {
            m_nFrameCount++;
            m_nTimeElapsed += dt;

            if (m_nTimeElapsed >= 1.0f)
            {
                m_nLastFps = m_nFrameCount;
                m_nFrameCount = 0;
                m_nTimeElapsed = 0.0f;

                m_fps.SetText(std::format("Render Thread FPS: {}", m_nLastFps));
            }
        }

        if (WaitForSingleObject(m_handleExitEvent, 0) == WAIT_OBJECT_0)
        {
            logger::info("[RenderThread] Exit signal received — shutting down.");
            SetEvent(m_handlePresentDoneEvent);
            return 0u;
        }

        CleanFrame();
        WriteFrame();

        PresentFrame();
        SetEvent(m_handlePresentDoneEvent);

        //const DWORD flag = WaitForMultipleObjects(2, waits, FALSE, INFINITE);
        //if (flag == WAIT_OBJECT_0) // exit
        //{
        //    SetEvent(m_handlePresentDoneEvent);
        //    return 0u;
        //}
        //else if (flag == WAIT_OBJECT_0 + 1)
        //{
        //    PresentFrame();
        //    SetEvent(m_handlePresentDoneEvent);
        //}
        //else if (flag == WAIT_FAILED)
        //{
        //    const DWORD err = GetLastError();
        //    logger::info("[RenderThread] WaitForMultipleObjects WAIT_FAILED (GetLastError=0x{:08X}) — aborting.", err);
        //    SetEvent(m_handlePresentDoneEvent);
        //    return 0u;
        //}
        //else
        //{
        //    logger::info("[RenderThread] Unexpected WaitForMultipleObjects result (0x{:X}) — aborting.", flag);
        //    SetEvent(m_handlePresentDoneEvent);
        //    return 0u;
        //}
    }
}

void pixel_engine::PERenderAPI::WaitForPresent()
{
    SetEvent(m_handlePresentEvent);
    WaitForSingleObject(m_handlePresentDoneEvent, INFINITE);
}

void pixel_engine::PERenderAPI::CleanFrame()
{
    const float clear[4] = { 1.f, 1.0f, 1.0f, 1.0f };
    m_pDeviceContext->ClearRenderTargetView(m_pRTV.Get(), clear);

    if (m_pRaster2D) m_pRaster2D->Clear({ 237, 237, 199 });
}

void pixel_engine::PERenderAPI::WriteFrame()
{
    PERenderQueue::Instance().Update();
    PERenderQueue::Instance().Render(m_pRaster2D.get());

    m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);
    m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0u);
    m_pDeviceContext->Draw(3, 0);
}

void pixel_engine::PERenderAPI::PresentFrame()
{
    m_pRaster2D->Present(m_pDeviceContext.Get(), m_pCpuImageBuffer.Get());
    m_pSwapchain->Present(0u, 0u);
}

_Use_decl_annotations_
bool pixel_engine::PERenderAPI::InitializeDirectX(const INIT_RENDER_API_DESC* desc)
{
    if (not CreateDeviceAndDeviceContext(desc)) return false;
    if (not CreateSwapChain(desc))              return false;
    if (not CreateRTV(desc))                    return false;
    if (not CreateVertexShader(desc))           return false;
    if (not CreatePixelShader(desc))            return false;
    if (not CreateViewport(desc))               return false;

    return true;
}

_Use_decl_annotations_
bool pixel_engine::PERenderAPI::CreateDeviceAndDeviceContext(const INIT_RENDER_API_DESC* desc)
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

_Use_decl_annotations_
bool pixel_engine::PERenderAPI::CreateSwapChain(const INIT_RENDER_API_DESC* desc)
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
    DXGI_SWAP_CHAIN_DESC sdesc{};
    sdesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sdesc.BufferCount                        = 1u;
    sdesc.OutputWindow                       = desc->WindowsHandle;
    sdesc.SampleDesc.Count                   = 1u;
    sdesc.SampleDesc.Quality                 = 0u;
    sdesc.Windowed                           = FALSE;
    sdesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
    sdesc.Flags                              = 0u;
    sdesc.BufferDesc.Width                   = desc->Width;
    sdesc.BufferDesc.Height                  = desc->Height;
    sdesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sdesc.BufferDesc.RefreshRate.Numerator   = 60u;
    sdesc.BufferDesc.RefreshRate.Denominator = 1u;

    hr = m_pFactory->CreateSwapChain(m_pDevice.Get(),
        &sdesc, m_pSwapchain.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to Create Swapchain");
        return false;
    }

    logger::success(pixel_engine::logger_config::LogCategory::Render,
        "Created Swapchain!");

    if (desc->FullScreen) m_pSwapchain->SetFullscreenState(TRUE, nullptr);
    else                  m_pSwapchain->SetFullscreenState(FALSE,  nullptr);
    
    return true;
}

_Use_decl_annotations_
bool pixel_engine::PERenderAPI::CreateRTV(const INIT_RENDER_API_DESC* desc)
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

_Use_decl_annotations_
bool pixel_engine::PERenderAPI::CreateVertexShader(const INIT_RENDER_API_DESC* desc)
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

_Use_decl_annotations_
bool pixel_engine::PERenderAPI::CreatePixelShader(const INIT_RENDER_API_DESC* desc)
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

    std::string wStr = std::to_string(desc->Width);
    std::string hStr = std::to_string(desc->Height);
    D3D_SHADER_MACRO macros[] = 
    {
        { "WIDTH",  wStr.c_str() },
        { "HEIGHT", hStr.c_str() },
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
        static_cast<size_t>(desc->Width) *
        static_cast<size_t>(desc->Height) * 3u;

    m_PaddedDataSize = ((dataSize + 3u) / 4u) * 4u;

    D3D11_BUFFER_DESC bdesc{};
    bdesc.ByteWidth = static_cast<UINT>(m_PaddedDataSize);
    bdesc.Usage = D3D11_USAGE_DEFAULT;
    bdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bdesc.CPUAccessFlags = 0u;
    bdesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    m_pCpuImageBuffer.Reset();
    hr = m_pDevice->CreateBuffer(&bdesc, nullptr, m_pCpuImageBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create shader resource on backbuffer");
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
    srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    srvd.Format = DXGI_FORMAT_R32_TYPELESS;
    srvd.BufferEx.FirstElement = 0;
    srvd.BufferEx.NumElements = static_cast<UINT>(m_PaddedDataSize / 4u);
    srvd.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;

    m_pSRV.Reset();
    hr = m_pDevice->CreateShaderResourceView(
        m_pCpuImageBuffer.Get(),
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
    return true;
}

_Use_decl_annotations_
bool pixel_engine::PERenderAPI::CreateViewport(const INIT_RENDER_API_DESC* desc)
{
    m_Viewport.Width = static_cast<FLOAT>(desc->Width);
    m_Viewport.Height = static_cast<FLOAT>(desc->Height);
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;
    m_Viewport.TopLeftX = 0.0f;
    m_Viewport.TopLeftY = 0.0f;

    m_pDeviceContext->RSSetViewports(1, &m_Viewport);
    m_pDeviceContext->OMSetRenderTargets(1, m_pRTV.GetAddressOf(), nullptr);

    logger::success(
        pixel_engine::logger_config::LogCategory::Render,
        "Initialized and Configured Viewport");

    return true;
}

_Use_decl_annotations_
bool pixel_engine::PERenderAPI::InitializeRenderAPI(const INIT_RENDER_API_DESC* desc)
{
    if (not InitializeRaster2D(desc))    return false;
    if (not InitializeRenderQueue(desc)) return false;
    return true;
}

_Use_decl_annotations_
bool pixel_engine::PERenderAPI::InitializeRaster2D(const INIT_RENDER_API_DESC* desc)
{
    PFE_RASTER_CONSTRUCT_DESC rasterDesc{};
    rasterDesc.EnableBoundCheck = true;
    rasterDesc.Viewport = 
    {
        0,
        0,
        static_cast<UINT>(desc->Width),
        static_cast<UINT>(desc->Height)
    };
    m_pRaster2D = std::make_unique<PERaster2D>(&rasterDesc);

    PFE_RASTER_INIT_DESC rasterInit{};
    rasterInit.WorkerCount = 8u;
    m_pRaster2D->Init(&rasterInit);

    return true;
}

_Use_decl_annotations_
bool pixel_engine::PERenderAPI::InitializeRenderQueue(const INIT_RENDER_API_DESC* desc)
{
    PFE_RENDER_QUEUE_CONSTRUCT_DESC renderDesc{};
    renderDesc.pCamera = m_pCamera;
    renderDesc.ScreenHeight = desc->Height;
    renderDesc.ScreenWidth = desc->Width;
    renderDesc.TilePx = 32;

    PERenderQueue::Init(renderDesc);
    return true;
}
