#include "pch.h"
#include "quad.h"

using namespace pixel_engine;

std::string QuadObject::GetObjectName() const
{
    return "QuadObject";
}

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

void QuadObject::Update(float deltaTime, const Camera2D* camera)
{
    m_bScreenDirty = true;
    RebuildIfDirty(camera);
}

void QuadObject::SetTransform(const FTransform2D& t)
{
    m_base = t;
    MarkDirty(true);
    m_bScreenDirty = true;
}

const FTransform2D& QuadObject::GetTransform() const
{
    return m_base;
}

FMatrix2DAffine QuadObject::GetAffineMatrix() const
{
    RebuildIfDirty(nullptr);
    return m_cachedWorld;
}

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

bool QuadObject::HasPreAffine() const noexcept
{
    return m_hasPre;
}

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

fox_math::Vector2D<float> QuadObject::GetPosition() const
{
    return m_base.Position;
}

float QuadObject::GetRotation() const
{
    return m_base.Rotation;
}

fox_math::Vector2D<float> QuadObject::GetScale() const
{
    return m_base.Scale;
}

fox_math::Vector2D<float> QuadObject::GetPivot() const
{
    return m_base.Pivot;
}

void QuadObject::SetUnitSize(float widthUnits, float heightUnits)
{
    m_unitSize.x = widthUnits;
    m_unitSize.y = heightUnits;
}

fox_math::Vector2D<float> QuadObject::GetUnitSize() const
{
    return m_unitSize;
}

void QuadObject::SetVisible(bool v)
{
    m_visible = v;
}

bool QuadObject::IsVisible() const
{
    return m_visible;
}

void QuadObject::SetLayer(uint32_t l)
{
    m_layer = l;
}

uint32_t QuadObject::GetLayer() const
{
    return m_layer;
}

//~ Build the discrete sampling grid from cached screen space center.
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

void QuadObject::RebuildIfDirty(const Camera2D* camera) const
{
    if (not IsDirty())
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

    if (camera != nullptr && (m_bScreenDirty || camera != m_pLastCamera))
    {
        const FVector2D S_origin = camera->WorldToScreen({ 0.0f, 0.0f }, m_nTilePx);
        const FVector2D S_x1 = camera->WorldToScreen({ 1.0f, 0.0f }, m_nTilePx);
        const FVector2D S_y1 = camera->WorldToScreen({ 0.0f, 1.0f }, m_nTilePx);

        const FVector2D CamUx{ S_x1.x - S_origin.x, S_x1.y - S_origin.y }; // px per +X world
        const FVector2D CamUy{ S_y1.x - S_origin.x, S_y1.y - S_origin.y }; // px per +Y world

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

        m_baseScreen.x = S_origin.x + center_world.x * CamUx.x + center_world.y * CamUy.x;
        m_baseScreen.y = S_origin.y + center_world.x * CamUx.y + center_world.y * CamUy.y;

        // cache state
        m_pLastCamera = const_cast<Camera2D*>(camera);
        m_bScreenDirty = false;
    }
}
