#include "pch.h"
#include "quad.h"

std::string pixel_engine::QuadObject::GetObjectName() const
{
    return std::string();
}

bool pixel_engine::QuadObject::Initialize()
{
    return false;
}

bool pixel_engine::QuadObject::Release()
{
    return false;
}

void pixel_engine::QuadObject::SetTransform(const FTransform2D& t)
{
}

const FTransform2D& pixel_engine::QuadObject::GetTransform() const
{
    // TODO: insert return statement here
}

FMatrix2DAffine pixel_engine::QuadObject::GetAffineMatrix() const
{
    return FMatrix2DAffine();
}

void pixel_engine::QuadObject::SetPreAffine(const FMatrix2DAffine& m)
{
}

void pixel_engine::QuadObject::ClearPreAffine()
{
}

bool pixel_engine::QuadObject::HasPreAffine() const noexcept
{
    return false;
}

void pixel_engine::QuadObject::SetPostAffine(const FMatrix2DAffine& m)
{
}

void pixel_engine::QuadObject::ClearPostAffine()
{
}

bool pixel_engine::QuadObject::HasPostAffine() const noexcept
{
    return false;
}

void pixel_engine::QuadObject::SetPosition(float x, float y)
{
}

void pixel_engine::QuadObject::SetRotation(float radians)
{
}

void pixel_engine::QuadObject::SetUnitSize(float widthUnits, float heightUnits)
{
}

fox_math::Vector2D<float> pixel_engine::QuadObject::GetUnitSize() const
{
    return fox_math::Vector2D<float>();
}

void pixel_engine::QuadObject::SetVisible(bool v)
{
}

bool pixel_engine::QuadObject::IsVisible() const
{
    return false;
}

void pixel_engine::QuadObject::SetLayer(uint32_t l)
{
}

uint32_t pixel_engine::QuadObject::GetLayer() const
{
    return 0;
}
