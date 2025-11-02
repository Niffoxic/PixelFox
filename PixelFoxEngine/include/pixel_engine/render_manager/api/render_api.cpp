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
    m_pCamera = desc->Camera;

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
    constexpr int   TILE_PX = 32;
    constexpr float STEP = 1.0f / TILE_PX;
    constexpr int   COUNT = 1300;

    struct image_data { float width_units, height_units; };
    image_data quad_units{ 1.0f, 1.0f };
    const float halfWU = quad_units.width_units * 0.5f;
    const float halfHU = quad_units.height_units * 0.5f;

    const int VPW = static_cast<int>(m_Viewport.Width);
    const int VPH = static_cast<int>(m_Viewport.Height);

    auto MakeRGB = [](uint8_t r, uint8_t g, uint8_t b) -> PFE_FORMAT_R8G8B8_UINT {
        PFE_FORMAT_R8G8B8_UINT c{}; c.R.Value = r; c.G.Value = g; c.B.Value = b; return c;
        };

    // time
    static float t = 0.0f, globalAngle = 0.0f;
    t += deltaTime;
    globalAngle += 1.0f * deltaTime;

    // per-instance params
    static std::array<FVector2D, COUNT> basePos{};
    static std::array<PFE_FORMAT_R8G8B8_UINT, COUNT> color{};
    static std::array<float, COUNT> amplitude{};
    static std::array<float, COUNT> speed{};
    static std::array<float, COUNT> phase{};
    static std::array<bool, COUNT> moveX{};
    static bool initialized = false;

    if (!initialized)
    {
        const int gridW = 45, gridH = 5;
        const float spacing = 1.f;
        const float startX = -((gridW - 1) * spacing) * 0.5f;
        const float startY = -((gridH - 1) * spacing) * 0.5f;

        for (int i = 0; i < COUNT; ++i)
        {
            int gx = i % gridW, gy = i / gridW;
            basePos[i] = { startX + gx * spacing, startY + gy * spacing };
            color[i] = MakeRGB((i * 37) % 255, (i * 73) % 255, (i * 97) % 255);
            amplitude[i] = 1.5f + 0.05f * (i % 13);
            speed[i] = 4.7f + 0.03f * (i % 17);
            phase[i] = 0.5f * float(i);
            moveX[i] = ((i & 1) == 0);
        }
        initialized = true;
    }

    const int cols = static_cast<int>(std::ceil(quad_units.width_units / STEP));
    const int rows = static_cast<int>(std::ceil(quad_units.height_units / STEP));

    const FVector2D S_origin = m_pCamera->WorldToScreen({ 0.0f, 0.0f }, TILE_PX);
    const FVector2D S_x1 = m_pCamera->WorldToScreen({ 1.0f, 0.0f }, TILE_PX);
    const FVector2D S_y1 = m_pCamera->WorldToScreen({ 0.0f, 1.0f }, TILE_PX);
    const FVector2D CamUx{ S_x1.x - S_origin.x, S_x1.y - S_origin.y }; // px per X world
    const FVector2D CamUy{ S_y1.x - S_origin.x, S_y1.y - S_origin.y }; // px per Y world

    for (int idx = 0; idx < COUNT; ++idx)
    {
        const float off = amplitude[idx] * std::sin(t * speed[idx] + phase[idx]);
        FVector2D pos = basePos[idx];
        if (moveX[idx]) pos.x += off; else pos.y += off;

        const float theta = globalAngle + (idx * 0.25f);
        const float cs = std::cos(theta), sn = std::sin(theta);
        const FVector2D ux_world{ cs,  sn }; // local +u axis in world
        const FVector2D vy_world{ -sn,  cs }; // local +v axis in world

        // map object bases to screen via camera bases
        const FVector2D Su{ ux_world.x * CamUx.x + ux_world.y * CamUy.x,
                            ux_world.x * CamUx.y + ux_world.y * CamUy.y };
        const FVector2D Sv{ vy_world.x * CamUx.x + vy_world.y * CamUy.x,
                            vy_world.x * CamUx.y + vy_world.y * CamUy.y };

        // object center in screen space
        const FVector2D base{
            S_origin.x + pos.x * CamUx.x + pos.y * CamUy.x,
            S_origin.y + pos.x * CamUx.y + pos.y * CamUy.y
        };

        // starting corner (u0,v0) and per-step deltas
        const float u0 = -halfWU, v0 = -halfHU;
        FVector2D rowStart{
            base.x + u0 * Su.x + v0 * Sv.x,
            base.y + u0 * Su.y + v0 * Sv.y
        };
        const FVector2D dU{ Su.x * STEP, Su.y * STEP };
        const FVector2D dV{ Sv.x * STEP, Sv.y * STEP };

        // AABB of rotated quad
        const FVector2D A = rowStart;
        const FVector2D B{ rowStart.x + dU.x * cols, rowStart.y + dU.y * cols };
        const FVector2D C{ rowStart.x + dV.x * rows, rowStart.y + dV.y * rows };
        const FVector2D D{ B.x + dV.x * rows,       B.y + dV.y * rows };

        float minX = std::min(std::min(A.x, B.x), std::min(C.x, D.x));
        float maxX = std::max(std::max(A.x, B.x), std::max(C.x, D.x));
        float minY = std::min(std::min(A.y, B.y), std::min(C.y, D.y));
        float maxY = std::max(std::max(A.y, B.y), std::max(C.y, D.y));

        if (maxX < 0.0f || maxY < 0.0f || minX >= VPW || minY >= VPH)
            continue;

        // raster
        const auto c = color[idx];
        for (int j = 0; j < rows; ++j)
        {
            FVector2D p = rowStart;
            for (int i = 0; i < cols; ++i)
            {
                const int ix = static_cast<int>(std::lround(p.x));
                const int iy = static_cast<int>(std::lround(p.y));
                if ((unsigned)ix < (unsigned)VPW && (unsigned)iy < (unsigned)VPH)
                    m_imageBuffer->WriteAt(iy, ix, c);

                p.x += dU.x; p.y += dU.y;
            }
            rowStart.x += dV.x; rowStart.y += dV.y;
        }
    }

    {
        auto Red = MakeRGB(255, 0, 0);
        const int cx = VPW / 2, cy = VPH / 2;
        if ((unsigned)cx < (unsigned)VPW && (unsigned)cy < (unsigned)VPH) {
            m_imageBuffer->WriteAt(cy, cx, Red);
            if (cx + 1 < VPW) m_imageBuffer->WriteAt(cy, cx + 1, Red);
            if (cy + 1 < VPH) m_imageBuffer->WriteAt(cy + 1, cx, Red);
        }
    }

    m_pDeviceContext->UpdateSubresource(
        m_pCpuImageBuffer.Get(), 0, nullptr,
        m_imageBuffer->Data(), (UINT)m_imageBuffer->RowPitch(), 0);
}
