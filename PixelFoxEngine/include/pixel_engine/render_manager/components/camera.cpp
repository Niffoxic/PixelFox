#include "pch.h"
#include "camera.h"

#include "fox_math/math.h"
#include "pixel_engine/utilities/logger/logger.h"

using namespace pixel_engine;

Camera2D::Camera2D() noexcept
{
    RebuildCached();
}

std::string pixel_engine::Camera2D::GetObjectName() const
{
    return "2DCameraComponent";
}

bool pixel_engine::Camera2D::Initialize()
{
    if (m_nViewWidth == 0 || m_nViewHeight == 0)
    {
        logger::warning("Camera2D Initialized with viewport 0!");
    }
    return true;
}

bool pixel_engine::Camera2D::Release()
{
    return true;
}

void pixel_engine::Camera2D::OnFrameBegin(float deltaTime)
{
    FollowTarget     (deltaTime);
    ShakeCameraEffect(deltaTime);

    if (m_bDirty) 
    {
        ApplyClamp();
        RebuildCached();
        m_bDirty = false;
    }
}

void pixel_engine::Camera2D::OnFrameEnd()
{
}

void pixel_engine::Camera2D::SetPosition(const FVector2D& position) noexcept
{
    m_transformCamera.Position = position;
    m_bDirty                   = true;
}

void pixel_engine::Camera2D::SetRotation(float radians) noexcept
{
    m_transformCamera.Rotation = radians;
    m_bDirty                   = true;
}

void pixel_engine::Camera2D::SetZoom(float zoom) noexcept
{
    if (zoom <= 0.f)
    {
        logger::error("Camera2D::SetZoom:: user trying to zoom below 0!");
        return;
    }

    m_transformCamera.Scale = { zoom, zoom };
    m_bDirty                = true;
}

void pixel_engine::Camera2D::SetScale(const FVector2D& s) noexcept
{
    if (s.x <= 0.f || s.y <= 0.f) 
    {
        logger::error("Camera2D::SetZoom: scale must be > 0");
        return;
    }

    m_transformCamera.Scale = s;
    m_bDirty = true;
}

void pixel_engine::Camera2D::SetTransform(const FTransform2D& transform) noexcept
{
    m_transformCamera = transform;
    m_bDirty          = true;
}

const FVector2D& pixel_engine::Camera2D::GetPosition() const noexcept
{
    return m_transformCamera.Position;
}

float pixel_engine::Camera2D::GetRotation() const noexcept
{
    return m_transformCamera.Rotation;
}

float pixel_engine::Camera2D::GetZoom() const noexcept
{
    return m_transformCamera.Scale.x;
}

const FVector2D& pixel_engine::Camera2D::GetScale() const noexcept
{
    return m_transformCamera.Scale;
}

const FTransform2D& pixel_engine::Camera2D::GetTransform() const noexcept
{
    return m_transformCamera;
}

void pixel_engine::Camera2D::SetViewportSize(uint32_t width, uint32_t height) noexcept
{
    if (width == 0 || height == 0) 
        logger::warning("Camera2D.SetViewportSize: zero dimension.");

    m_nViewHeight = height;
    m_nViewWidth  = width;
    m_bDirty      = true;
}

void pixel_engine::Camera2D::SetViewportOrigin(const FVector2D& originPx) noexcept
{
    m_vecViewOriginPx  = originPx;
    m_bDirty           = true;
}

uint32_t pixel_engine::Camera2D::GetViewportWidth() const noexcept
{
    return m_nViewWidth;
}

uint32_t pixel_engine::Camera2D::GetViewportHeight() const noexcept
{
    return m_nViewHeight;
}

const FVector2D& pixel_engine::Camera2D::GetViewportOrigin() const noexcept
{
    return m_vecViewOriginPx;
}

void pixel_engine::Camera2D::SetScreenYDown(bool yDown) noexcept
{
    m_bYDown = yDown;
    m_bDirty = true;
}

bool pixel_engine::Camera2D::IsScreenYDown() const noexcept
{
    return m_bYDown;
}

FMatrix2DAffine pixel_engine::Camera2D::GetViewMatrix() const noexcept
{
    return m_matView;
}

FMatrix2DAffine pixel_engine::Camera2D::GetInvViewMatrix() const noexcept
{
    return m_matInvView;
}

FMatrix2DAffine pixel_engine::Camera2D::GetScreenMatrix() const noexcept
{
    return m_matScreen;
}

FVector2D pixel_engine::Camera2D::WorldToView(const FVector2D& pWorld) const noexcept
{
    return m_matView.TransformPoint(pWorld);
}

FVector2D pixel_engine::Camera2D::ViewToWorld(const FVector2D& pView) const noexcept
{
    return m_matInvView.TransformPoint(pView);
}

FVector2D pixel_engine::Camera2D::WorldToScreen(const FVector2D& pWorld) const noexcept
{
    FVector2D vec = WorldToView(pWorld);
    vec           = m_matScreen.TransformPoint(vec);
    auto shake    = GetShakeNoise();

    return {vec.x + m_vecViewOriginPx.x + shake.first,
            vec.y + m_vecViewOriginPx.y + shake.second};
}

FVector2D pixel_engine::Camera2D::ScreenToWorld(const FVector2D& pScreen) const noexcept
{
    auto shake = GetShakeNoise();
    
    FVector2D vec =
    { pScreen.x - m_vecViewOriginPx.x - shake.first, 
      pScreen.y - m_vecViewOriginPx.y - shake.second };

    auto invScreen   = m_matScreen.Inversed();
    FVector2D result = invScreen  .TransformPoint(vec);

    return ViewToWorld(result);
}

pixel_engine::PFE_RECT pixel_engine::Camera2D::ScreenRectToWorldAABB() const noexcept
{
    const FVector2D s0{ 0.f,0.f };
    const FVector2D s1{ static_cast<float>(m_nViewWidth),0.f };
    
    const FVector2D s2{ static_cast<float>(m_nViewWidth),
                        static_cast<float>(m_nViewHeight) };

    const FVector2D s3{ 0.f,static_cast<float>(m_nViewHeight)};

    const FVector2D w0 = ScreenToWorld(s0);
    const FVector2D w1 = ScreenToWorld(s1);
    const FVector2D w2 = ScreenToWorld(s2);
    const FVector2D w3 = ScreenToWorld(s3);

    const float minx = std::min(std::min(w0.x, w1.x), std::min(w2.x, w3.x));
    const float maxx = std::max(std::max(w0.x, w1.x), std::max(w2.x, w3.x));
    const float miny = std::min(std::min(w0.y, w1.y), std::min(w2.y, w3.y));
    const float maxy = std::max(std::max(w0.y, w1.y), std::max(w2.y, w3.y));

    return { minx, miny, maxx - minx, maxy - miny };
}

void pixel_engine::Camera2D::Pan(const FVector2D& deltaWorld) noexcept
{
    m_transformCamera.Position = m_transformCamera.Position + deltaWorld;
    m_bDirty                   = true;
}

void pixel_engine::Camera2D::ZoomAtScreenPoint(float factor, const FVector2D& screenPt) noexcept
{
    if (factor <= 0.f) 
    {
        logger::error("Camera2D.ZoomAtScreenPoint: factor must be > 0");
        return;
    }

    const FVector2D worldBefore = ScreenToWorld(screenPt);

    auto scale = m_transformCamera.Scale;
    m_transformCamera.Scale = { scale.x * factor, scale.y * factor };
    
    m_bDirty = true; 
    RebuildCached();

    const FVector2D worldAfter = ScreenToWorld(screenPt);

    auto pos = m_transformCamera.Position;
    m_transformCamera.Position = pos + (worldBefore - worldAfter);

    m_bDirty = true;
}

void pixel_engine::Camera2D::RotateAtWorldPoint(float deltaRad, const FVector2D& worldPt) noexcept
{
    auto pos           = m_transformCamera.Position;
    const float c      = std::cos(deltaRad), s = std::sin(deltaRad);
    const FVector2D to = pos - worldPt;
    
    const FVector2D rot{ c * to.x - s * to.y, s * to.x + c * to.y };

    m_transformCamera.Position = worldPt + rot;
    m_transformCamera.Rotation += deltaRad;
    
    m_bDirty = true;
}

void pixel_engine::Camera2D::SetFollowTarget(const FVector2D* target) noexcept
{
    m_pFollowTarget = target;
}

void pixel_engine::Camera2D::SetFollowSmoothing(float smooth01) noexcept
{
    m_nFollowSmooth = fox_math::Clamp(smooth01, 0.f, 1.f);
}

void pixel_engine::Camera2D::SetWorldBounds(const PFE_RECT& bounds) noexcept
{
    m_rectWorldBounds = bounds;
}

void pixel_engine::Camera2D::EnableWorldClamp(bool enable) noexcept
{
    m_bWorldClamp = enable;
}

pixel_engine::PFE_RECT pixel_engine::Camera2D::GetWorldBounds() const noexcept
{
    return m_rectWorldBounds;
}

float pixel_engine::Camera2D::GetFollowSmoothing() const noexcept
{
    return m_nFollowSmooth;
}

bool pixel_engine::Camera2D::IsWorldClampEnabled() const noexcept
{
    return m_bWorldClamp;
}

void pixel_engine::Camera2D::StartShake(float amplitudePx, float frequencyHz, float durationSec) noexcept
{
    m_nShakeAmp       = std::max(0.f, amplitudePx);
    m_nShakeFreq      = std::max(0.f, frequencyHz);
    m_nShakeTime      = std::max(0.f, durationSec);
    m_nShakeTotalTime = 0.f;
}

void pixel_engine::Camera2D::StopShake() noexcept
{
    m_nShakeAmp  = m_nShakeFreq      = 0.f;
    m_nShakeTime = m_nShakeTotalTime = 0.f;
}

bool pixel_engine::Camera2D::IsShaking() const noexcept
{
    return (m_nShakeTime > 0.f) && (m_nShakeAmp > 0.f) && (m_nShakeFreq > 0.f);
}

void pixel_engine::Camera2D::RebuildCached() noexcept
{
    m_matInvView = m_transformCamera.ToMatrix();
    m_matInvView = m_matInvView.Inversed();

    const float zx = m_transformCamera.Scale.x;
    const float zy = m_transformCamera.Scale.y;
    const float sy = m_bYDown ? 1.f : -1.f;

    m_matScreen = FMatrix2DAffine::Scale(zx, sy * zy);
}

void pixel_engine::Camera2D::ApplyClamp() noexcept
{
    if (!m_bWorldClamp      ||
         m_nViewWidth  == 0 ||
         m_nViewHeight == 0) return;

    const FVector2D center{ m_vecViewOriginPx.x,
                            m_vecViewOriginPx.y };

    const FVector2D worldCenter = ScreenToWorld(center);
    const FVector2D worldRight  = ScreenToWorld(
                                  { center.x + 0.5f * m_nViewWidth,
                                    center.y });

    const FVector2D worldUp = ScreenToWorld(
        { center.x, m_bYDown ? center.y - 0.5f * m_nViewHeight 
          : center.y + 0.5f * m_nViewHeight });

    const float halfW = std::abs(worldRight.x - worldCenter.x);
    const float halfH = std::abs(worldUp.y - worldCenter.y);

    auto pos = m_transformCamera.Position;

    const float minX = m_rectWorldBounds.x + halfW;
    const float maxX = m_rectWorldBounds.x + m_rectWorldBounds.w - halfW;
    const float minY = m_rectWorldBounds.y + halfH;
    const float maxY = m_rectWorldBounds.y + m_rectWorldBounds.h - halfH;

    if (minX <= maxX) pos.x = fox_math::Clamp(pos.x, minX, maxX);
    if (minY <= maxY) pos.y = fox_math::Clamp(pos.y, minY, maxY);

    m_transformCamera.Position = pos;
}

void pixel_engine::Camera2D::FollowTarget(float delatTime) noexcept
{
    if (not m_pFollowTarget) return;

    const float t = fox_math::Clamp(delatTime * m_nFollowSmooth, 0.f, 1.f);

    m_transformCamera.Position.x = fox_math::Lerp(m_transformCamera.Position.x, m_pFollowTarget->x, t);
    m_transformCamera.Position.y = fox_math::Lerp(m_transformCamera.Position.y, m_pFollowTarget->y, t);

    m_bDirty = true;
}

void pixel_engine::Camera2D::ShakeCameraEffect(float deltaTime) noexcept
{
    if (m_nShakeTime <= 0.f) return;

    m_nShakeTotalTime += deltaTime;
    m_nShakeTime      -= deltaTime;
    
    if (m_nShakeTime <= 0.0f)
    {
        m_nShakeAmp = m_nShakeFreq = m_nShakeTotalTime = 0.f;
    }
    m_bDirty = true;
}

std::pair<float, float> pixel_engine::Camera2D::GetShakeNoise() const noexcept
{
    float sx = 0.f, sy = 0.0f;

    if (m_nShakeAmp > 0.f && m_nShakeFreq > 0.f && m_nShakeTime > 0.f)
    {
        const float ph = 0.5f;
        sx = m_nShakeAmp * std::sinf(fox_math::TWO_PI *
            m_nShakeFreq * m_nShakeTotalTime + ph);

        sy = m_nShakeAmp * std::cosf(fox_math::TWO_PI *
            m_nShakeFreq * m_nShakeTotalTime);
    }

    return { sx, sy };
}
