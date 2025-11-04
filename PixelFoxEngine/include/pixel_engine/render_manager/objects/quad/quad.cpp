#include "pch.h"
#include "quad.h"

#include "pixel_engine/render_manager/components/texture/texture_resource.h"

//~ test
#include "pixel_engine/render_manager/components/texture/sampler/bilinear_sampler.h"

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

void QuadObject::SetPosition(float x, float y)
{
    m_transform.Position.x = x;
    m_transform.Position.y = y;
    MarkDirty(true);
}

void QuadObject::SetRotation(float radians)
{
    m_transform.Rotation = radians;
    MarkDirty(true);
}

void QuadObject::SetScale(float sx, float sy)
{
    m_transform.Scale.x = sx;
    m_transform.Scale.y = sy;
    MarkDirty(true);
}

void QuadObject::SetPivot(float px, float py)
{
    m_transform.Pivot.x = px;
    m_transform.Pivot.y = py;
    MarkDirty(true);
}

void pixel_engine::QuadObject::SetTexture(const std::string& path)
{
    //~ only testing sampler here so fixed 32 px
    m_pTexture   = TextureResource::Instance().LoadTexture(path);

    if (not m_pTexture)
    {
        logger::error("Failed to load texture");
        return;
    }

    auto sampled = BilinearSampler::Instance().GetSampledImage
    (m_pTexture, 32, m_transform.Scale);

    if (sampled) m_pSampledTexture = std::move(sampled);
    else
    {
        logger::error("Failed to sample texture");
        return;
    }

    logger::success("The Sampler Made the size with: (width:{}, height:{})",
        m_pSampledTexture->GetWidth(),
        m_pSampledTexture->GetHeight());
    
    m_szTexturePath = path;
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

void QuadObject::SetVisible(bool v)
{
    m_visible = v;
}

_Use_decl_annotations_
bool QuadObject::IsVisible() const
{
    return m_visible;
}

void QuadObject::SetLayer(uint32_t l)
{
    m_layer = l;
}

_Use_decl_annotations_
uint32_t QuadObject::GetLayer() const
{
    return m_layer;
}

_Use_decl_annotations_
Texture* pixel_engine::QuadObject::GetTexture() const
{
    return m_pSampledTexture.get();
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

void pixel_engine::QuadObject::UpdateObjectToCameraSpace(const PFE_WORLD_SPACE_DESC& cameraView)
{
    FVector2D CamUx
    { cameraView.X1.x - cameraView.Origin.x,
      cameraView.X1.y - cameraView.Origin.y };

    FVector2D CamUy
    {   cameraView.Y1.x - cameraView.Origin.x,
        cameraView.Y1.y - cameraView.Origin.y };

    auto matrix    = m_transform.ToMatrix().matrix;
    const float a  = matrix[0][0]; // scaling x direction with cos theta
    const float b  = matrix[0][1]; // -sin theta rotation
    const float tx = matrix[0][2]; // translation x
    const float c  = matrix[1][0]; // sin theta
    const float d  = matrix[1][1]; // scalling y direction with cos theta
    const float ty = matrix[1][2]; // transation y

    const FVector2D u_local     { a, c };
    const FVector2D v_local     { b, d };
    const FVector2D center_local{ tx, ty };

    m_ObjectCameraAxisU.x = u_local.x * CamUx.x + u_local.y * CamUy.x;
    m_ObjectCameraAxisU.y = u_local.x * CamUx.y + u_local.y * CamUy.y;

    m_ObjectCameraAxisV.x = v_local.x * CamUx.x + v_local.y * CamUy.x;
    m_ObjectCameraAxisV.y = v_local.x * CamUx.y + v_local.y * CamUy.y;

    m_ObjectCameraViewPosition.x = cameraView.Origin.x +
        center_local.x *
        CamUx.x + center_local.y * CamUy.x;

    m_ObjectCameraViewPosition.y = cameraView.Origin.y + center_local.x
        * CamUx.y + center_local.y * CamUy.y;
}
