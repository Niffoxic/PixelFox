/*
 * LLM Generated TestCode
 * reason: limited time I can't implement and write its google test within time constraints
*/
#pragma once
#include "pch.h"

#include "fox_math/vector.h"

using fox_math::Vector2D;

static constexpr float kEps = 1e-5f;

// -----------------------------
// Integer type: basic behavior
// -----------------------------
TEST(Vector2D_Int_Basics, ConstructorsAndEquality)
{
    Vector2D<int> a;
    EXPECT_EQ(a.x, 0);
    EXPECT_EQ(a.y, 0);

    Vector2D<int> b(3, -4);
    EXPECT_EQ(b.x, 3);
    EXPECT_EQ(b.y, -4);

    Vector2D<int> c(3, -4);
    EXPECT_TRUE(b == c);
    EXPECT_FALSE(b != c);
}

TEST(Vector2D_Int_Basics, ArithmeticAndIndex)
{
    Vector2D<int> a(1, 2);
    Vector2D<int> b(3, 4);

    auto s = a + b;
    EXPECT_EQ(s.x, 4);
    EXPECT_EQ(s.y, 6);

    auto d = b - a;
    EXPECT_EQ(d.x, 2);
    EXPECT_EQ(d.y, 2);

    auto m = a * 5;
    EXPECT_EQ(m.x, 5);
    EXPECT_EQ(m.y, 10);

    auto q = b / 2; // integer division
    EXPECT_EQ(q.x, 1);
    EXPECT_EQ(q.y, 2);

    a += b;
    EXPECT_EQ(a.x, 4);
    EXPECT_EQ(a.y, 6);

    b -= Vector2D<int>(1, 1);
    EXPECT_EQ(b.x, 2);
    EXPECT_EQ(b.y, 3);

    // operator[]
    Vector2D<int> v(10, 20);
    v[0] = 7;
    v[1] = 9;
    EXPECT_EQ(v[0], 7);
    EXPECT_EQ(v[1], 9);
}

// ---------------------------------
// Float type: full math operations
// ---------------------------------
TEST(Vector2D_Float, UnaryAndConstants)
{
    auto z = Vector2D<float>::Zero();
    EXPECT_FLOAT_EQ(z.x, 0.0f);
    EXPECT_FLOAT_EQ(z.y, 0.0f);

    auto o = Vector2D<float>::One();
    EXPECT_FLOAT_EQ(o.x, 1.0f);
    EXPECT_FLOAT_EQ(o.y, 1.0f);

    auto ux = Vector2D<float>::UnitX();
    auto uy = Vector2D<float>::UnitY();
    EXPECT_FLOAT_EQ(ux.x, 1.0f); EXPECT_FLOAT_EQ(ux.y, 0.0f);
    EXPECT_FLOAT_EQ(uy.x, 0.0f); EXPECT_FLOAT_EQ(uy.y, 1.0f);

    Vector2D<float> a(2.0f, -3.0f);
    auto pos = +a;
    auto neg = -a;
    EXPECT_FLOAT_EQ(pos.x, 2.0f);
    EXPECT_FLOAT_EQ(pos.y, -3.0f);
    EXPECT_FLOAT_EQ(neg.x, -2.0f);
    EXPECT_FLOAT_EQ(neg.y, 3.0f);
}

TEST(Vector2D_Float, DotCrossLength)
{
    Vector2D<float> a(1.0f, 2.0f);
    Vector2D<float> b(3.0f, 4.0f);

    EXPECT_FLOAT_EQ(a.Dot(b), 11.0f);       // 1*3 + 2*4
    EXPECT_FLOAT_EQ(a.Cross(b), -2.0f);     // 1*4 - 2*3

    EXPECT_FLOAT_EQ(a.LengthSq(), 5.0f);
    EXPECT_NEAR(a.Length(), std::sqrt(5.0f), kEps);
}

TEST(Vector2D_Float, NormalizeAndNormalized)
{
    Vector2D<float> v(3.0f, 4.0f);
    auto n = v.Normalized();
    EXPECT_NEAR(n.x, 3.0f / 5.0f, kEps);
    EXPECT_NEAR(n.y, 4.0f / 5.0f, kEps);

    Vector2D<float> w(0.0f, 0.0f);
    auto nz = w.Normalized(); // stays zero, no NaNs
    EXPECT_FLOAT_EQ(nz.x, 0.0f);
    EXPECT_FLOAT_EQ(nz.y, 0.0f);

    v.Normalize();
    EXPECT_NEAR(v.x, 3.0f / 5.0f, kEps);
    EXPECT_NEAR(v.y, 4.0f / 5.0f, kEps);
}

TEST(Vector2D_Float, Perpendiculars)
{
    Vector2D<float> a(2.0f, 3.0f);
    auto ccw = a.PerpendicularCCW(); // (-y, x)
    auto cw = a.PerpendicularCW();  // (y, -x)
    EXPECT_FLOAT_EQ(ccw.x, -3.0f); EXPECT_FLOAT_EQ(ccw.y, 2.0f);
    EXPECT_FLOAT_EQ(cw.x, 3.0f); EXPECT_FLOAT_EQ(cw.y, -2.0f);
}

TEST(Vector2D_Float, ProjectReflectRotate)
{
    Vector2D<float> v(2.0f, 3.0f);

    // Project onto X axis => (x, 0)
    Vector2D<float> xAxis(1.0f, 0.0f);
    auto projX = v.ProjectOnto(xAxis);
    EXPECT_FLOAT_EQ(projX.x, 2.0f);
    EXPECT_FLOAT_EQ(projX.y, 0.0f);

    // Reflect over horizontal axis (normal (0,1)): y flips
    Vector2D<float> n(0.0f, 1.0f);
    auto r = v.Reflect(n);
}
