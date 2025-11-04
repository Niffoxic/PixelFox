#include "pch.h"
#include "camera.h"

#include "fox_math/math.h"
#include "pixel_engine/utilities/logger/logger.h"

using namespace pixel_engine;

Camera2D::Camera2D() noexcept
{
}

std::string pixel_engine::Camera2D::GetObjectName() const
{
    return "2DCameraComponent";
}

bool pixel_engine::Camera2D::Initialize()
{
    return true;
}

bool pixel_engine::Camera2D::Release()
{
    return true;
}

void pixel_engine::Camera2D::OnFrameBegin(float deltaTime)
{
}

void pixel_engine::Camera2D::OnFrameEnd()
{
}

void pixel_engine::Camera2D::SetPosition(const FVector2D& position) noexcept
{
    m_transformCamera.Position = position;
}

void pixel_engine::Camera2D::AddPosition(const FVector2D& delta) noexcept
{
    m_transformCamera.Position = m_transformCamera.Position + delta;
}

void pixel_engine::Camera2D::SetRotation(float radians) noexcept
{
    m_transformCamera.Rotation = radians;
}

void pixel_engine::Camera2D::SetScale(const FVector2D& s) noexcept
{
    if (s.x <= 0.f || s.y <= 0.f) 
    {
        logger::error("Camera2D::SetZoom: scale must be > 0");
        return;
    }

    m_transformCamera.Scale = s;
}

void pixel_engine::Camera2D::SetTransform(const FTransform2D& transform) noexcept
{
    m_transformCamera = transform;
}

const FVector2D& pixel_engine::Camera2D::GetPosition() const noexcept
{
    return m_transformCamera.Position;
}

float pixel_engine::Camera2D::GetRotation() const noexcept
{
    return m_transformCamera.Rotation;
}

const FVector2D& pixel_engine::Camera2D::GetScale() const noexcept
{
    return m_transformCamera.Scale;
}

const FTransform2D& pixel_engine::Camera2D::GetTransform() const noexcept
{
    return m_transformCamera;
}

FVector2D pixel_engine::Camera2D::WorldToCamera(const FVector2D& pWorld, const uint32_t& tile) const noexcept
{
    FVector2D vec = m_transformCamera.ToMatrix().TransformPoint(pWorld);
    vec.x *= tile;
    vec.y *= tile;
    return vec;
}
