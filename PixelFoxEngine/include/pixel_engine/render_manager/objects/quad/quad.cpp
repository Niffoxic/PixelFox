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
#include "quad.h"

#include "pixel_engine/render_manager/components/texture/allocator/texture_resource.h"
#include "pixel_engine/utilities/logger/logger.h"

using namespace pixel_engine;

_Use_decl_annotations_
std::string QuadObject::GetObjectName() const
{
    return "QuadObject";
}

_Use_decl_annotations_
bool QuadObject::Initialize()
{
    MarkDirty(true);
    return true;
}

bool QuadObject::Release()
{
    MarkDirty(true);
    return true;
}

_Use_decl_annotations_
void QuadObject::Update(float deltaTime, const PFE_WORLD_SPACE_DESC& space)
{
    UpdateObjectToCameraSpace(space);
}

_Use_decl_annotations_
void QuadObject::SetTransform(const FTransform2D& t)
{
    m_transform = t;
    MarkDirty(true);
}

_Use_decl_annotations_
const FTransform2D& QuadObject::GetTransform() const
{
    return m_transform;
}

_Use_decl_annotations_
FMatrix2DAffine QuadObject::GetAffineMatrix() const
{
    return m_transform.ToMatrix();
}

_Use_decl_annotations_
void QuadObject::SetPosition(float x, float y)
{
    m_transform.Position.x = x;
    m_transform.Position.y = y;
    MarkDirty(true);
}

_Use_decl_annotations_
void QuadObject::SetRotation(float radians)
{
    m_transform.Rotation = radians;
    MarkDirty(true);
}

_Use_decl_annotations_
void QuadObject::SetScale(float sx, float sy)
{
    m_transform.Scale.x = sx;
    m_transform.Scale.y = sy;
    m_bResampleNeeded = true;
    MarkDirty(true);
}

_Use_decl_annotations_
void QuadObject::SetPivot(float px, float py)
{
    m_transform.Pivot.x = px;
    m_transform.Pivot.y = py;
    MarkDirty(true);
}

_Use_decl_annotations_
void pixel_engine::QuadObject::SetTexture(const std::string& path)
{
    m_pTexture = TextureResource::Instance().LoadTexture(path);

    if (not m_pTexture)
    {
        logger::error("Failed to load texture");
        return;
    }
    m_szTexturePath = path;
    m_bResampleNeeded = true;
}

_Use_decl_annotations_
void pixel_engine::QuadObject::SetTexture(Texture* rawTexture)
{
    m_pTexture = rawTexture;
    m_bResampleNeeded = true;
}

_Use_decl_annotations_
fox_math::Vector2D<float> QuadObject::GetPosition() const
{
    return m_transform.Position;
}

_Use_decl_annotations_
float QuadObject::GetRotation() const
{
    return m_transform.Rotation;
}

_Use_decl_annotations_
fox_math::Vector2D<float> QuadObject::GetScale() const
{
    return m_transform.Scale;
}

_Use_decl_annotations_
fox_math::Vector2D<float> QuadObject::GetPivot() const
{
    return m_transform.Pivot;
}

_Use_decl_annotations_
void QuadObject::SetVisible(bool v)
{
    m_visible = v;
}

_Use_decl_annotations_
bool QuadObject::IsVisible() const
{
    return m_visible;
}

_Use_decl_annotations_
void QuadObject::SetLayer(ELayer l)
{
    m_layer = l;
}

_Use_decl_annotations_
ELayer QuadObject::GetLayer() const
{
    return m_layer;
}

_Use_decl_annotations_
Texture* pixel_engine::QuadObject::GetTexture() const
{
    return m_pTexture;
}

_Use_decl_annotations_
FVector2D pixel_engine::QuadObject::GetUAxisRelativeToCamera() const noexcept
{
    return m_ObjectCameraAxisU;
}

_Use_decl_annotations_
FVector2D pixel_engine::QuadObject::GetVAxisRelativeToCamera() const noexcept
{
    return m_ObjectCameraAxisV;
}

_Use_decl_annotations_
FVector2D pixel_engine::QuadObject::GetPositionRelativeToCamera() const noexcept
{
    return m_ObjectCameraViewPosition;
}

_Use_decl_annotations_
void pixel_engine::QuadObject::UpdateObjectToCameraSpace(const PFE_WORLD_SPACE_DESC& cameraView)
{
    FVector2D CamUx
    {
        cameraView.X1.x - cameraView.Origin.x,
        cameraView.X1.y - cameraView.Origin.y
    };
    FVector2D CamUy{
        cameraView.Y1.x - cameraView.Origin.x,
        cameraView.Y1.y - cameraView.Origin.y
    };

    auto matrix = m_transform.ToMatrix().matrix;
    const float a = matrix[0][0];
    const float b = matrix[0][1];
    const float tx = matrix[0][2];
    const float c = matrix[1][0];
    const float d = matrix[1][1];
    const float ty = matrix[1][2];

    const FVector2D u_local{ a, c };
    const FVector2D v_local{ b, d };
    const FVector2D center_local{ tx, ty };

    m_ObjectCameraAxisU.x = u_local.x * CamUx.x + u_local.y * CamUy.x;
    m_ObjectCameraAxisU.y = u_local.x * CamUx.y + u_local.y * CamUy.y;

    m_ObjectCameraAxisV.x = v_local.x * CamUx.x + v_local.y * CamUy.x;
    m_ObjectCameraAxisV.y = v_local.x * CamUx.y + v_local.y * CamUy.y;

    m_ObjectCameraViewPosition.x = cameraView.Origin.x + center_local.x * CamUx.x + center_local.y * CamUy.x;
    m_ObjectCameraViewPosition.y = cameraView.Origin.y + center_local.x * CamUx.y + center_local.y * CamUy.y;
}
