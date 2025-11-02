#include "pch.h"
#include "quad.h"

std::string pixel_engine::QuadObject::GetObjectName() const
{
    return "QuadObject";
}

bool pixel_engine::QuadObject::Initialize()
{
    return true;
}

bool pixel_engine::QuadObject::Release()
{
    m_hasPre = false;
    m_hasPost = false;
    return true;
}

void pixel_engine::QuadObject::SetTransform(const FTransform2D& t)
{
    m_base = t;
}

const FTransform2D& pixel_engine::QuadObject::GetTransform() const
{
    return m_base;
}

FMatrix2DAffine pixel_engine::QuadObject::GetAffineMatrix() const
{
    // Compose: Pre * Base * Post
    const FMatrix2DAffine baseM = m_base.ToMatrix();

    if (m_hasPre && m_hasPost)
        return m_pre * baseM * m_post;
    if (m_hasPre)
        return m_pre * baseM;
    if (m_hasPost)
        return baseM * m_post;
    return baseM;
}

void pixel_engine::QuadObject::SetPreAffine(const FMatrix2DAffine& m)
{
    m_pre = m;
    m_hasPre = true;
}

void pixel_engine::QuadObject::ClearPreAffine()
{
    m_hasPre = false;
}

bool pixel_engine::QuadObject::HasPreAffine() const noexcept
{
    return m_hasPre;
}

void pixel_engine::QuadObject::SetPostAffine(const FMatrix2DAffine& m)
{
    m_post = m;
    m_hasPost = true;
}

void pixel_engine::QuadObject::ClearPostAffine()
{
    m_hasPost = false;
}

bool pixel_engine::QuadObject::HasPostAffine() const noexcept
{
    return m_hasPost;
}

void pixel_engine::QuadObject::SetPosition(float x, float y)
{
    m_base.Position.x = x;
    m_base.Position.y = y;
}

void pixel_engine::QuadObject::SetRotation(float radians)
{
    m_base.Rotation = radians;
}

void pixel_engine::QuadObject::SetScale(float sx, float sy)
{
    m_base.Scale.x = sx;
    m_base.Scale.y = sy;
}

void pixel_engine::QuadObject::SetPivot(float px, float py)
{
    m_base.Pivot.x = px;
    m_base.Pivot.y = py;
}

fox_math::Vector2D<float> pixel_engine::QuadObject::GetPosition() const
{
    return m_base.Position;
}

float pixel_engine::QuadObject::GetRotation() const
{
    return m_base.Rotation;
}

fox_math::Vector2D<float> pixel_engine::QuadObject::GetScale() const
{
    return m_base.Scale;
}

fox_math::Vector2D<float> pixel_engine::QuadObject::GetPivot() const
{
    return  m_base.Pivot;
}

void pixel_engine::QuadObject::SetUnitSize(float widthUnits, float heightUnits)
{
    m_unitSize.x = widthUnits;
    m_unitSize.y = heightUnits;
}

fox_math::Vector2D<float> pixel_engine::QuadObject::GetUnitSize() const
{
    return m_unitSize;
}

void pixel_engine::QuadObject::SetVisible(bool v)
{
    m_visible = v;
}

bool pixel_engine::QuadObject::IsVisible() const
{
    return m_visible;
}

void pixel_engine::QuadObject::SetLayer(uint32_t l)
{
    m_layer = l;
}

uint32_t pixel_engine::QuadObject::GetLayer() const
{
    return m_layer;
}
