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
#include "camera.h"

#include "fox_math/math.h"
#include "pixel_engine/utilities/logger/logger.h"

using namespace pixel_engine;

_Use_decl_annotations_
std::string pixel_engine::Camera2D::GetObjectName() const
{
    return "2DCameraComponent";
}

_Use_decl_annotations_
bool pixel_engine::Camera2D::Initialize()
{
    return true;
}

_Use_decl_annotations_
bool pixel_engine::Camera2D::Release()
{
    return true;
}

_Use_decl_annotations_
void pixel_engine::Camera2D::OnFrameBegin(float deltaTime)
{
    if (m_pFollowSprite && deltaTime > 0.0f)
    {
        const FVector2D targetPos = m_pFollowSprite->GetPosition();

        constexpr float stiffness = 10.0f;
        const float alpha = 1.0f - std::exp(-stiffness * deltaTime);

        const FVector2D delta = FVector2D
        {
            targetPos.x - m_transformCamera.Position.x,
            targetPos.y - m_transformCamera.Position.y
        };
        m_transformCamera.Position.x += delta.x * alpha;
        m_transformCamera.Position.y += delta.y * alpha;
    }
}

void pixel_engine::Camera2D::OnFrameEnd()
{
}

void pixel_engine::Camera2D::SetPosition(const FVector2D& position) noexcept
{
    m_transformCamera.Position = position;
}

_Use_decl_annotations_
void pixel_engine::Camera2D::AddPosition(const FVector2D& delta) noexcept
{
    m_transformCamera.Position = m_transformCamera.Position + delta;
}

_Use_decl_annotations_
void pixel_engine::Camera2D::SetRotation(float radians) noexcept
{
    m_transformCamera.Rotation = radians;
}

_Use_decl_annotations_
void pixel_engine::Camera2D::SetScale(const FVector2D& s) noexcept
{
    if (s.x <= 0.f || s.y <= 0.f) 
    {
        logger::error("Camera2D::SetZoom: scale must be > 0");
        return;
    }

    m_transformCamera.Scale = s;
}

_Use_decl_annotations_
void pixel_engine::Camera2D::SetTransform(const FTransform2D& transform) noexcept
{
    m_transformCamera = transform;
}

_Use_decl_annotations_
const FVector2D& pixel_engine::Camera2D::GetPosition() const noexcept
{
    return m_transformCamera.Position;
}

_Use_decl_annotations_
float pixel_engine::Camera2D::GetRotation() const noexcept
{
    return m_transformCamera.Rotation;
}

_Use_decl_annotations_
const FVector2D& pixel_engine::Camera2D::GetScale() const noexcept
{
    return m_transformCamera.Scale;
}

_Use_decl_annotations_
const FTransform2D& pixel_engine::Camera2D::GetTransform() const noexcept
{
    return m_transformCamera;
}

_Use_decl_annotations_
FVector2D pixel_engine::Camera2D::WorldToCamera(
    const FVector2D& pWorld,
    const uint32_t& tile) const noexcept
{
    FVector2D rel
    {
        pWorld.x - m_transformCamera.Position.x,
        pWorld.y - m_transformCamera.Position.y
    };

    const float r = -m_transformCamera.Rotation;
    const float cs = std::cos(r);
    const float sn = std::sin(r);

    FVector2D v
    {
        rel.x * cs - rel.y * sn,
        rel.x * sn + rel.y * cs
    };

    v.x *= (m_transformCamera.Scale.x * static_cast<float>(tile));
    v.y *= (m_transformCamera.Scale.y * static_cast<float>(tile));

    return v;
}

void pixel_engine::Camera2D::FollowSprite(PEISprite* sprite)
{
    m_pFollowSprite = sprite;

    if (m_pFollowSprite)
    {
        const FVector2D targetPos = m_pFollowSprite->GetPosition();
        m_transformCamera.Position = targetPos;
    }
}
