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
    m_bScreenDirty = true;
    m_pLastCamera = nullptr;
    return true;
}

bool QuadObject::Release()
{
    m_hasPre = false;
    m_hasPost = false;
    MarkDirty(true);
    m_bScreenDirty = true;
    m_pLastCamera = nullptr;
    return true;
}

void QuadObject::Update(float deltaTime, const PFE_WORLD_SPACE_DESC& space)
{
    RebuildIfDirty(space.pCamera);
    UpdateWorldPosition(space);
}

_Use_decl_annotations_
void QuadObject::SetTransform(const FTransform2D& t)
{
    m_base = t;
    MarkDirty(true);
    m_bScreenDirty = true;
}

_Use_decl_annotations_
const FTransform2D& QuadObject::GetTransform() const
{
    return m_base;
}

_Use_decl_annotations_
FMatrix2DAffine QuadObject::GetAffineMatrix() const
{
    RebuildIfDirty(nullptr);
    return m_cachedWorld;
}

_Use_decl_annotations_
void QuadObject::SetPreAffine(const FMatrix2DAffine& m)
{
    m_pre = m;
    m_hasPre = true;
    MarkDirty(true);
    m_bScreenDirty = true;
}

void QuadObject::ClearPreAffine()
{
    if (m_hasPre)
    {
        m_hasPre = false;
        MarkDirty(true);
        m_bScreenDirty = true;
    }
}

_Use_decl_annotations_
bool QuadObject::HasPreAffine() const noexcept
{
    return m_hasPre;
}

_Use_decl_annotations_
void QuadObject::SetPostAffine(const FMatrix2DAffine& m)
{
    m_post = m;
    m_hasPost = true;
    MarkDirty(true);
    m_bScreenDirty = true;
}

void QuadObject::ClearPostAffine()
{
    if (m_hasPost)
    {
        m_hasPost = false;
        MarkDirty(true);
        m_bScreenDirty = true;
    }
}

_Use_decl_annotations_
bool QuadObject::HasPostAffine() const noexcept
{
    return m_hasPost;
}

void QuadObject::SetPosition(float x, float y)
{
    m_base.Position.x = x;
    m_base.Position.y = y;
    MarkDirty(true);
    m_bScreenDirty = true;
}

void QuadObject::SetRotation(float radians)
{
    m_base.Rotation = radians;
    MarkDirty(true);
    m_bScreenDirty = true;
}

void QuadObject::SetScale(float sx, float sy)
{
    m_base.Scale.x = sx;
    m_base.Scale.y = sy;
    MarkDirty(true);
    m_bScreenDirty = true;
}

void QuadObject::SetPivot(float px, float py)
{
    m_base.Pivot.x = px;
    m_base.Pivot.y = py;
    MarkDirty(true);
    m_bScreenDirty = true;
}

void pixel_engine::QuadObject::SetTexture(const std::string& path)
{
    m_pTexture   = TextureResource::Instance().LoadTexture(path);
    auto sampled = BilinearSampler::Instance().GetSampledImage
    (m_pTexture, m_nTilePx, m_base.Scale);

    if (sampled) m_pSampledTexture = std::move(sampled);
    
    m_szTexturePath = path;
}

_Use_decl_annotations_
fox_math::Vector2D<float> QuadObject::GetPosition() const
{
    return m_base.Position;
}

_Use_decl_annotations_
float QuadObject::GetRotation() const
{
    return m_base.Rotation;
}

_Use_decl_annotations_
fox_math::Vector2D<float> QuadObject::GetScale() const
{
    return m_base.Scale;
}

_Use_decl_annotations_
fox_math::Vector2D<float> QuadObject::GetPivot() const
{
    return m_base.Pivot;
}

void QuadObject::SetUnitSize(float widthUnits, float heightUnits)
{
    m_unitSize.x = widthUnits;
    m_unitSize.y = heightUnits;
}

_Use_decl_annotations_
fox_math::Vector2D<float> QuadObject::GetUnitSize() const
{
    return m_unitSize;
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

//~ Build the discrete sampling grid from cached screen space center.
_Use_decl_annotations_
bool QuadObject::BuildDiscreteGrid(float step, PFE_SAMPLE_GRID_2D& gridOut) const
{
    if (m_bScreenDirty || m_pLastCamera == nullptr)
        return false;

    const float halfWU = m_unitSize.x * 0.5f;
    const float halfHU = m_unitSize.y * 0.5f;

    gridOut.cols = static_cast<int>(std::ceil(m_unitSize.x / step));
    gridOut.rows = static_cast<int>(std::ceil(m_unitSize.y / step));
    if (gridOut.cols <= 0 || gridOut.rows <= 0) return false;

    const float u0 = -halfWU;
    const float v0 = -halfHU;

    // deltas in screen space
    gridOut.dU.x = m_SuScreen.x * step;
    gridOut.dU.y = m_SuScreen.y * step;
    gridOut.dV.x = m_SvScreen.x * step;
    gridOut.dV.y = m_SvScreen.y * step;

    // Starting corner in screen space
    gridOut.RowStart.x = m_baseScreen.x + u0 * m_SuScreen.x + v0 * m_SvScreen.x;
    gridOut.RowStart.y = m_baseScreen.y + u0 * m_SuScreen.y + v0 * m_SvScreen.y;

    return true;
}

_Use_decl_annotations_
Texture* pixel_engine::QuadObject::GetTexture() const
{
    return m_pSampledTexture.get();
}

_Use_decl_annotations_
void QuadObject::RebuildIfDirty(const Camera2D* camera) const
{
    if (IsDirty())
    {
        const FMatrix2DAffine baseM = m_base.ToMatrix();
        if (m_hasPre && m_hasPost)
            m_cachedWorld = m_pre * baseM * m_post;
        else if (m_hasPre)
            m_cachedWorld = m_pre * baseM;
        else if (m_hasPost)
            m_cachedWorld = baseM * m_post;
        else
            m_cachedWorld = baseM;

        MarkDirty(false);
        m_bScreenDirty = true;
    }
}

void pixel_engine::QuadObject::UpdateWorldPosition(const PFE_WORLD_SPACE_DESC& space) const
{
    const FVector2D CamUx
    { space.X1.x - space.Origin.x,
      space.X1.y - space.Origin.y };

    const FVector2D CamUy
    { space.Y1.x - space.Origin.x,
        space.Y1.y - space.Origin.y };

    const float a = m_cachedWorld.matrix[0][0];
    const float b = m_cachedWorld.matrix[0][1];
    const float tx = m_cachedWorld.matrix[0][2];
    const float c = m_cachedWorld.matrix[1][0];
    const float d = m_cachedWorld.matrix[1][1];
    const float ty = m_cachedWorld.matrix[1][2];

    const FVector2D u_world{ a, c };
    const FVector2D v_world{ b, d };
    const FVector2D center_world{ tx, ty };

    m_SuScreen.x = u_world.x * CamUx.x + u_world.y * CamUy.x;
    m_SuScreen.y = u_world.x * CamUx.y + u_world.y * CamUy.y;

    m_SvScreen.x = v_world.x * CamUx.x + v_world.y * CamUy.x;
    m_SvScreen.y = v_world.x * CamUx.y + v_world.y * CamUy.y;

    m_baseScreen.x = space.Origin.x + center_world.x * CamUx.x + center_world.y * CamUy.x;
    m_baseScreen.y = space.Origin.y + center_world.x * CamUx.y + center_world.y * CamUy.y;

    // cache state
    m_pLastCamera = const_cast<Camera2D*>(space.pCamera);
    m_bScreenDirty = false;
}
