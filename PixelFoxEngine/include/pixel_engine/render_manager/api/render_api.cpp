#include "pch.h"
#include "render_api.h"

#include "pixel_engine/utilities/logger/logger.h" 
#include "core/vector.h"

// TODO: Make logger thread safe!

pixel_engine::PERenderAPI::PERenderAPI(const CONSTRUCT_RENDER_API_DESC* desc)
{
    m_handleStartEvent = desc->StartEvent;
    m_handleExitEvent  = desc->ExitEvent;

    m_handlePresentEvent = CreateEvent(
        nullptr,
        TRUE,
        FALSE,
        nullptr
    );

    m_handlePresentDoneEvent = CreateEvent(
        nullptr,
        TRUE,
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
    if (not CreateDeviceAndDeviceContext(desc)) return false;
    if (not CreateSwapChain(desc))              return false;
    if (not CreateRTV(desc))                    return false;
    if (not CreateVertexShader(desc))           return false;
    if (not CreatePixelShader(desc))            return false;
    if (not CreateViewport(desc))               return false;

    if (desc->Clock) m_pClock = desc->Clock;

    return true;
}

DWORD pixel_engine::PERenderAPI::Execute()
{
    WaitForSingleObject(m_handleStartEvent, INFINITE);
    HANDLE waits[2] = { m_handleExitEvent, m_handlePresentEvent };
    
    while (true)
    {
        if (WaitForSingleObject(m_handleExitEvent, 0) == WAIT_OBJECT_0)
        {
            return 0u;
        }

        //~ Prepare Images
        CleanFrame();
        float dt = m_pClock ? m_pClock->DeltaTime() : 0.0f;
        WriteFrame(dt);

        //~ Wait for signal when to present or if quit is fired
        DWORD flag = WaitForMultipleObjects(2, waits, FALSE, INFINITE);

        if (flag == WAIT_OBJECT_0) // exit called probably
        {
            return 0u;
        }
        else if (flag == WAIT_OBJECT_0 + 1)
        {
            PresentFrame();
            SetEvent(m_handlePresentDoneEvent); // present done
        }
        else 
        {
            // Unknown error
            return 0u; // TODO: Need to think about returns for error codes
        }
    }
    return 0;
}

void pixel_engine::PERenderAPI::WaitForPresent()
{
    ResetEvent(m_handlePresentDoneEvent);
    SetEvent(m_handlePresentEvent);
    WaitForSingleObject(m_handlePresentDoneEvent, INFINITE);
}

void pixel_engine::PERenderAPI::CleanFrame()
{
    const float clear[4] = { 1.f, 1.0f, 1.0f, 1.0f };
    m_pDeviceContext->ClearRenderTargetView(m_pRTV.Get(), clear);
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

// ================== I M TESTING HERE ===========================

struct TestRGB { unsigned char r, g, b; };

struct TestSurface
{
    unsigned char* data = nullptr;
    UINT width = 0;
    UINT height = 0;
    size_t stride = 0;
    UINT channels = 3;

    inline void Put(UINT x, UINT y, TestRGB c) noexcept
    {
        if (x >= width || y >= height) return;
        unsigned char* p = data + y * stride + x * channels;
        p[0] = c.r; p[1] = c.g; p[2] = c.b;
    }
};

inline size_t TestRowPitchRGB(UINT w) noexcept
{
    const size_t raw = size_t(w) * 3u;
    return (raw + 3u) & ~size_t(3);
}

inline TestSurface TestMakeSurface(unsigned char* buf, UINT w, UINT h, UINT channels = 3) noexcept
{
    TestSurface s{};
    s.data = buf;
    s.width = w;
    s.height = h;
    s.channels = channels;
    s.stride = TestRowPitchRGB(w);
    return s;
}

inline TestRGB TestRGBu(unsigned char r, unsigned char g, unsigned char b) noexcept { return { r, g, b }; }

// color palette
inline TestRGB TestPalette(int idx) noexcept
{
    switch (idx & 7)
    {
    case 0: return TestRGBu(20, 20, 20);    // near-black
    case 1: return TestRGBu(240, 240, 240); // near-white
    case 2: return TestRGBu(255, 85, 85);   // red-ish
    case 3: return TestRGBu(85, 255, 170);  // mint
    case 4: return TestRGBu(85, 170, 255);  // sky
    case 5: return TestRGBu(255, 210, 64);  // gold
    case 6: return TestRGBu(160, 96, 255);  // violet
    default:return TestRGBu(64, 192, 96);   // green
    }
}

inline void TestClear(TestSurface& s, TestRGB c) noexcept
{
    for (UINT y = 0; y < s.height; ++y)
    {
        unsigned char* row = s.data + y * s.stride;
        for (UINT x = 0; x < s.width; ++x)
        {
            row[x * 3 + 0] = c.r;
            row[x * 3 + 1] = c.g;
            row[x * 3 + 2] = c.b;
        }
    }
}

// Bresenham line
inline void TestDrawLine32(unsigned cx0, unsigned cy0, unsigned cx1, unsigned cy1,
    TestRGB col,
    TestSurface& s, UINT w, UINT h)
{
    (void)cx0;
    (void)cy0;

    (void)cx1;
    (void)cy1;

    (void)col;
    (void)s;
    (void)w;
    (void)h;
}

//~ Solid border around 32x32 virtual canvas
inline bool TestIsBorder32(int cx, int cy)
{
    return (cx == 0 || cy == 0 || cx == 31 || cy == 31);
}

//~ Filled circle mask on 32x32 grid
inline bool TestInCircle32(int cx, int cy, float centerX, float centerY, float radius)
{
    float dx = float(cx) - centerX;
    float dy = float(cy) - centerY;
    return (dx * dx + dy * dy) <= (radius * radius);
}

//~ Checkboard pattern on 32x32 grid
inline TestRGB TestChecker32(int cx, int cy, TestRGB a, TestRGB b)
{
    return (((cx ^ cy) & 1) ? b : a);
}

inline TestRGB TestSample32(int cx, int cy, float t)
{
    // Base: soft checkerboard
    TestRGB base = TestChecker32(cx, cy, TestPalette(0), TestPalette(0)); // very dark background

    // Subtle gradient tint
    float gx = (cx / 31.0f);
    float gy = (cy / 31.0f);
    unsigned char tint = (unsigned char)(18.0f * (gx * 0.6f + gy * 0.4f));
    base.r = (unsigned char)std::min(255, base.r + tint);
    base.g = (unsigned char)std::min(255, base.g + tint);
    base.b = (unsigned char)std::min(255, base.b + tint);

    // Border
    if (TestIsBorder32(cx, cy))
        return TestPalette(4); // sky-blue border

    // Smiley face in the center
    if (TestInCircle32(cx, cy, 16.0f, 16.0f, 10.5f))
    {
        // Face base
        TestRGB face = TestPalette(5);
        bool eyeL = (cx >= 11 && cx <= 13 && cy >= 12 && cy <= 13);
        bool eyeR = (cx >= 19 && cx <= 21 && cy >= 12 && cy <= 13);
        if (eyeL || eyeR) return TestPalette(1);

        if (cy >= 19 && cy <= 21 && cx >= 11 && cx <= 21)
        {
            float mx = (cx - 16.0f);
            float my = (cy - 19.0f);
            float curve = my - (mx * mx) * 0.04f;
            if (curve >= -0.7f && curve <= 1.2f)
                return TestPalette(2);
        }

        return face;
    }

    // Animated sparkle dot orbiting the face
    float ang = t * 2.1f; // radians/sec
    int sx = int(16.0f + 14.0f * std::cos(ang));
    int sy = int(16.0f + 14.0f * std::sin(ang));
    if (cx == sx && cy == sy)
        return TestPalette(6);

    if (((cx + cy) % 7) == 0) return TestRGBu(28, 28, 48);
    if (((cx - cy) % 9) == 0) return TestRGBu(22, 36, 64);

    return base;
}

inline void TestBlitCell32ToSurface(TestSurface& s, int cx, int cy, TestRGB c)
{
    const UINT w = s.width, h = s.height;
    const UINT x0 = (UINT)((uint64_t)cx * w / 32u);
    const UINT y0 = (UINT)((uint64_t)cy * h / 32u);
    const UINT x1 = (UINT)((uint64_t)(cx + 1) * w / 32u);
    const UINT y1 = (UINT)((uint64_t)(cy + 1) * h / 32u);
    for (UINT y = y0; y < y1; ++y)
    {
        unsigned char* row = s.data + y * s.stride;
        for (UINT x = x0; x < x1; ++x)
        {
            unsigned char* p = row + x * 3u;
            p[0] = c.r; p[1] = c.g; p[2] = c.b;
        }
    }
}

inline void TestRenderVirtual32(TestSurface& s, float t)
{
    for (int cy = 0; cy < 32; ++cy)
    {
        for (int cx = 0; cx < 32; ++cx)
        {
            const TestRGB col = TestSample32(cx, cy, t);
            TestBlitCell32ToSurface(s, cx, cy, col);
        }
    }
}

void pixel_engine::PERenderAPI::TestImageUpdate(float deltaTime)
{
    static float t = 0.0f;
    t += deltaTime;

    const UINT w = m_Viewport.Width;
    const UINT h = m_Viewport.Height;

    const size_t rowPitch = TestRowPitchRGB(w);
    const size_t bytes = rowPitch * size_t(h);

    fox::vector<unsigned char> cpu(bytes, 0);

    TestSurface surface = TestMakeSurface(cpu.data(), w, h, 3);

    TestRenderVirtual32(surface, t);
    
    m_pDeviceContext->UpdateSubresource(
        m_pCpuImageBuffer.Get(),
        0,
        nullptr,
        cpu.data(),
        (UINT)rowPitch,
        0);
}
