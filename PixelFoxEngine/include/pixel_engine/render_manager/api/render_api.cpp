#include "pch.h"
#include "render_api.h"

#include "pixel_engine/utilities/logger/logger.h" 
#include "core/vector.h"

#include "fox_math/math.h"
#include <cmath>

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

bool pixel_engine::PERenderAPI::Init(const INIT_RENDER_API_DESC* desc)
{
    if (not InitializeDirectX(desc))   return false;
    if (not InitializeRenderAPI(desc)) return false;

    if (desc->Clock) m_pClock = desc->Clock;

    return true;
}

DWORD pixel_engine::PERenderAPI::Execute()
{
    logger::info("[RenderThread] Waiting for Start Event...");
    WaitForSingleObject(m_handleStartEvent, INFINITE);
    logger::info("[RenderThread] Start Event received: render loop begin.");

    const HANDLE waits[2] = { m_handleExitEvent, m_handlePresentEvent };

    while (true)
    {
        if (WaitForSingleObject(m_handleExitEvent, 0) == WAIT_OBJECT_0)
        {
            logger::info("[RenderThread] Exit signal received — shutting down.");
            SetEvent(m_handlePresentDoneEvent);
            return 0u;
        }

        CleanFrame();
        const float dt = m_pClock ? m_pClock->DeltaTime() : 0.0f;
        WriteFrame(dt);

        const DWORD flag = WaitForMultipleObjects(2, waits, FALSE, INFINITE);
        if (flag == WAIT_OBJECT_0) // exit
        {
            SetEvent(m_handlePresentDoneEvent);
            return 0u;
        }
        else if (flag == WAIT_OBJECT_0 + 1)
        {
            PresentFrame();
            SetEvent(m_handlePresentDoneEvent);
        }
        else if (flag == WAIT_FAILED)
        {
            const DWORD err = GetLastError();
            logger::info("[RenderThread] WaitForMultipleObjects WAIT_FAILED (GetLastError=0x{:08X}) — aborting.", err);
            SetEvent(m_handlePresentDoneEvent);
            return 0u;
        }
        else
        {
            logger::info("[RenderThread] Unexpected WaitForMultipleObjects result (0x{:X}) — aborting.", flag);
            SetEvent(m_handlePresentDoneEvent);
            return 0u;
        }
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

    //~ TODO: Replace with PESwapchain
    m_imageBuffer->ClearImageBuffer({ 255, 255, 255 });
}

void pixel_engine::PERenderAPI::WriteFrame(float deltaTime)
{
    TestImageUpdate(deltaTime);
    m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);
    m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0u);
    m_pDeviceContext->Draw(3, 0);
}

void pixel_engine::PERenderAPI::PresentFrame()
{
    m_pSwapchain->Present(1u, 0u);
}

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

bool pixel_engine::PERenderAPI::InitializeRenderAPI(const INIT_RENDER_API_DESC* desc)
{
    if (not CreateImageBuffer(desc)) return false;
    return true;
}

bool pixel_engine::PERenderAPI::CreateImageBuffer(const INIT_RENDER_API_DESC* desc)
{
    PE_IMAGE_BUFFER_DESC imageDesc{};
    imageDesc.Height = desc->Height;
    imageDesc.Width  = desc->Width;
    m_imageBuffer    = std::make_unique<PEImageBuffer>(imageDesc);

    if (m_imageBuffer->Empty())
    {
        logger::error("Failed to create render api image buffer!");
        return false;
    }

    return true;
}

// ================== I M TESTING HERE ===========================

void pixel_engine::PERenderAPI::TestImageUpdate(float deltaTime)
{
    InitCameraOnce();

    struct RGB { uint8_t r, g, b; };

    auto InBounds = [](int x, int y, int W, int H) noexcept
    {
        return (uint32_t)x < (uint32_t)W && (uint32_t)y < (uint32_t)H;
    };

    auto Plot = [&](int x, int y, RGB c) noexcept 
    {
        if (InBounds(x, y,  (int)m_imageBuffer->Width(),
                            (int)m_imageBuffer->Height()))
            m_imageBuffer->WriteAt(y, x, { c.r,c.g,c.b });
    };

    auto WorldToPixel = [&](const FVector2D& w) noexcept
    {
        const FVector2D s = m_camera.WorldToScreen(w);
        return std::pair<int, int>{ (int)std::lround(s.x), (int)std::lround(s.y) };
    };

    auto DotW = [&](const FVector2D& w, RGB c) noexcept 
    {
        auto [x, y] = WorldToPixel(w); Plot(x, y, c);
    };

    auto CrossW = [&](const FVector2D& w, int half, RGB c) noexcept 
    {
        auto [x, y] = WorldToPixel(w);
        for (int i = -half; i <= half; ++i) { Plot(x + i, y, c); Plot(x, y + i, c); }
    };

    auto CrossScreen = [&](int cx, int cy, int half, RGB c) noexcept
    {
        const int W = (int)m_imageBuffer->Width();
        const int H = (int)m_imageBuffer->Height();
        for (int i = -half; i <= half; ++i)
        {
            if ((uint32_t)(cx + i) < (uint32_t)W) m_imageBuffer->WriteAt((uint32_t)cy, (uint32_t)(cx + i), { c.r,c.g,c.b });
            if ((uint32_t)(cy + i) < (uint32_t)H) m_imageBuffer->WriteAt((uint32_t)(cy + i), (uint32_t)cx, { c.r,c.g,c.b });
        }
    };

    auto BoxW = [&](const FVector2D& w, int half, RGB c) noexcept 
    {
        auto [x, y] = WorldToPixel(w);
        const int l = x - half, r = x + half, t = y - half, b = y + half;
        for (int i = l; i <= r; ++i) { Plot(i, t, c); Plot(i, b, c); }
        for (int j = t; j <= b; ++j) { Plot(l, j, c); Plot(r, j, c); }
    };

    auto Line = [&](int x0, int y0, int x1, int y1, RGB c) noexcept 
    {
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        for (;;) {
            Plot(x0, y0, c);
            if (x0 == x1 && y0 == y1) break;
            int e2 = err << 1;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    };

    auto LineW = [&](const FVector2D& A, const FVector2D& B, RGB c) noexcept 
    {
        const auto a = WorldToPixel(A);
        const auto b = WorldToPixel(B);
        Line(a.first, a.second, b.first, b.second, c);
    };

    auto RectOutlineW = [&](FVector2D minP, FVector2D maxP, RGB c) noexcept 
    {
        FVector2D p0{ minP.x, minP.y }, p1{ maxP.x, minP.y };
        FVector2D p2{ maxP.x, maxP.y }, p3{ minP.x, maxP.y };
        LineW(p0, p1, c); LineW(p1, p2, c); LineW(p2, p3, c); LineW(p3, p0, c);
    };

    auto hash2i = [](int x, int y) noexcept -> uint32_t
    {
        uint32_t h = 2166136261u;
        h ^= (uint32_t)x + 0x9e3779b9u + (h << 6) + (h >> 2);
        h ^= (uint32_t)y + 0x85ebca6bu + (h << 6) + (h >> 2);
        h *= 16777619u;
        return h;
    };

    // animated follow target
    static FVector2D sFollow{ 0.f, 0.f };
    static bool sBound = false;
    
    if (!sBound) 
    {
        m_camera.SetFollowTarget(&sFollow);
        m_camera.SetFollowSmoothing(0.20f);
        m_camera.SetZoom(60.0f);
        m_camera.SetRotation(0.f);
        sBound = true;
    }

    static float t = 0.f; t += deltaTime;
    sFollow.x = 6.0f * std::cos(t * 0.8f);
    sFollow.y = 4.0f * std::sin(t * 1.1f);

    m_camera.OnFrameBegin(deltaTime);

    // Build 32x32 tile once
    static bool sInit = false;
    static uint8_t sTile[32][32][3]; // RGB
    if (!sInit)
    {
        auto h = [](int x, int y)->uint32_t {
            uint32_t v = 2166136261u;
            v ^= (uint32_t)x + 0x9e3779b9u + (v << 6) + (v >> 2);
            v ^= (uint32_t)y + 0x85ebca6bu + (v << 6) + (v >> 2);
            v *= 16777619u;
            return v;
            };
        for (int ty = 0; ty < 32; ++ty)
        {
            for (int tx = 0; tx < 32; ++tx)
            {
                // Stable pattern inside the tile
                uint32_t v = h(tx, ty);
                uint8_t g = (uint8_t)(140 + (v & 0x3F));
                uint8_t r = (uint8_t)(18 + ((v >> 6) & 0x1F));
                uint8_t b = (uint8_t)(18 + ((v >> 11) & 0x1F));
                sTile[ty][tx][0] = r;
                sTile[ty][tx][1] = g;
                sTile[ty][tx][2] = b;
            }
        }
        sInit = true;
    }

    const uint32_t W = m_imageBuffer->Width();
    const uint32_t H = m_imageBuffer->Height();

    // Anchor tiles to world origin
    const FVector2D s00 = m_camera.WorldToScreen({ 0.f, 0.f });

    // Compute positive wrap offsets in [0,31]
    auto wrap32 = [](int v) { v %= 32; if (v < 0) v += 32; return v; };
    const int offX = wrap32((int)std::floor(s00.x));
    const int offY = wrap32((int)std::floor(s00.y));

    // Start drawing tiles so that the grid aligns with world origin
    const int startX = -offX;
    const int startY = -offY;

    // Blit tile across the screen
    for (int y = startY; y < (int)H; y += 32)
    {
        for (int x = startX; x < (int)W; x += 32)
        {
            // Blit 32x32
            const int maxY = std::min(y + 32, (int)H);
            const int maxX = std::min(x + 32, (int)W);
            for (int py = std::max(0, y); py < maxY; ++py)
            {
                const int sy = py - y; // tile y
                for (int px = std::max(0, x); px < maxX; ++px)
                {
                    const int sx = px - x; // tile x
                    const uint8_t r = sTile[sy][sx][0];
                    const uint8_t g = sTile[sy][sx][1];
                    const uint8_t b = sTile[sy][sx][2];
                    m_imageBuffer->WriteAt((uint32_t)py, (uint32_t)px, { r, g, b });
                }
            }
        }
    }
    
    // origin markers
    CrossW({ 0.f,0.f }, 5, { 30, 30, 30 });
    BoxW({ 0.f,0.f }, 8, { 30, 30, 30 });

    // static obstacles
    RectOutlineW({ -8.f,-2.f }, { -4.f, 2.f }, { 255,120, 60 }); // left
    RectOutlineW({ 3.f, 4.f }, { 7.f, 7.f }, { 60,160,255 }); // top-right
    RectOutlineW({ 8.f,-5.f }, { 11.f,-1.f }, { 180, 80,220 }); // far right
    RectOutlineW({ -10.f, 3.f }, { -6.f, 6.f }, { 80,200,120 }); // top-left

    // the followed target
    BoxW(sFollow, 6, { 255, 80, 80 });
    CrossW(sFollow, 4, { 80,200,120 });
    DotW(sFollow, { 20, 20,255 });
    CrossScreen((int)(m_Viewport.Width * 0.5f), (int)(m_Viewport.Height * 0.5f), 6, { 255, 255, 0 });

    m_camera.OnFrameEnd();

    m_pDeviceContext->UpdateSubresource(
        m_pCpuImageBuffer.Get(),
        0, nullptr,
        m_imageBuffer->Data(),
        (UINT)m_imageBuffer->RowPitch(),
        0);
}

void pixel_engine::PERenderAPI::InitCameraOnce()
{
    static bool s_inited = false;
    if (s_inited) return;
    s_inited = true;

    m_camera.SetViewportSize(m_Viewport.Width, m_Viewport.Height);
    m_camera.SetViewportOrigin({ m_Viewport.Width * 0.5f, m_Viewport.Height * 0.5f });
    m_camera.SetScreenYDown(true);
    m_camera.SetPosition({ 0.f, 0.f });
    m_camera.SetZoom(50.0f);

    m_camera.EnableWorldClamp(false);
    m_camera.Initialize();
}
