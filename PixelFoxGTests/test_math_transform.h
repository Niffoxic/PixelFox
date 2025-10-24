/*
 * LLM Generated TestCode
 * reason: limited time I can't implement and write its google test within the time constraint
*/
#pragma once
#include "pch.h"

#include <cmath>
#include <numbers>

#include "fox_math/vector.h"
#include "fox_math/matrix.h"
#include "fox_math/transform.h"

using fox_math::Vector2D;
using fox_math::Matrix2DAffine;
using fox_math::Transform2D;

static constexpr float kEps_transform = 1e-5f;

TEST(Transform2D_Float, ToMatrix_DefaultIsIdentity)
{
    Transform2D<float> t; // default ctor
    auto M = t.ToMatrix();

    auto I = Matrix2DAffine<float>::Identity();
    EXPECT_TRUE(Matrix2DAffine<float>::NearlyEqualMat(M, I, 1e-6f));

    // Sanity: points/directions pass through unchanged
    Vector2D<float> p(3.0f, -2.0f);
    Vector2D<float> d(1.5f, 4.0f);
    auto p2 = M.TransformPoint(p);
    auto d2 = M.TransformDirection(d);
    EXPECT_NEAR(p2.x, p.x, kEps_transform);
    EXPECT_NEAR(p2.y, p.y, kEps_transform);
    EXPECT_NEAR(d2.x, d.x, kEps_transform);
    EXPECT_NEAR(d2.y, d.y, kEps_transform);
}

TEST(Transform2D_Float, ToMatrix_TRS_WithPivot)
{
    // Sprite 100x150, pivot at center
    const float w = 100.0f, h = 150.0f;
    Transform2D<float> t;
    t.Position = { 320.0f, 200.0f };
    t.Scale = { 1.25f, 0.75f };
    t.Rotation = std::numbers::pi_v<float> *0.25f; // 45 deg
    t.Pivot = { w * 0.5f, h * 0.5f };

    auto M = t.ToMatrix();

    // Expected: T * R * S * T(-pivot)
    auto T = Matrix2DAffine<float>::Translation(t.Position.x, t.Position.y);
    auto R = Matrix2DAffine<float>::Rotation(t.Rotation);
    auto S = Matrix2DAffine<float>::Scale(t.Scale.x, t.Scale.y);
    auto P = Matrix2DAffine<float>::Translation(-t.Pivot.x, -t.Pivot.y);
    auto Expected = T * R * S * P;

    EXPECT_TRUE(Matrix2DAffine<float>::NearlyEqualMat(M, Expected, 1e-5f));

    // Transform a couple of sprite corners to be extra sure
    Vector2D<float> p0(0, 0), p1(w, 0), p2(w, h), p3(0, h);
    auto q0 = M.TransformPoint(p0);
    auto q1 = M.TransformPoint(p1);
    auto q2 = M.TransformPoint(p2);
    auto q3 = M.TransformPoint(p3);
    auto e0 = Expected.TransformPoint(p0);
    auto e1 = Expected.TransformPoint(p1);
    auto e2 = Expected.TransformPoint(p2);
    auto e3 = Expected.TransformPoint(p3);

    EXPECT_NEAR(q0.x, e0.x, 1e-4f); EXPECT_NEAR(q0.y, e0.y, 1e-4f);
    EXPECT_NEAR(q1.x, e1.x, 1e-4f); EXPECT_NEAR(q1.y, e1.y, 1e-4f);
    EXPECT_NEAR(q2.x, e2.x, 1e-4f); EXPECT_NEAR(q2.y, e2.y, 1e-4f);
    EXPECT_NEAR(q3.x, e3.x, 1e-4f); EXPECT_NEAR(q3.y, e3.y, 1e-4f);
}

TEST(Transform2D_Float, Compose_EqualsProduct)
{
    Transform2D<float> parent, child;

    parent.Position = { 100.0f, -50.0f };
    parent.Scale = { 1.2f, 0.8f };
    parent.Rotation = std::numbers::pi_v<float> *0.1f;
    parent.Pivot = { 0.0f, 0.0f };

    child.Position = { -20.0f, 10.0f };
    child.Scale = { 0.5f, 2.0f };
    child.Rotation = std::numbers::pi_v<float> *-0.25f;
    child.Pivot = { 10.0f, 5.0f };

    auto C1 = Transform2D<float>::Compose(parent, child);
    auto C2 = parent.ToMatrix() * child.ToMatrix();

    EXPECT_TRUE(Matrix2DAffine<float>::NearlyEqualMat(C1, C2, 1e-6f));

    // Check on a random point
    Vector2D<float> p(7.0f, -3.0f);
    auto a = C1.TransformPoint(p);
    auto b = C2.TransformPoint(p);
    EXPECT_NEAR(a.x, b.x, 1e-5f);
    EXPECT_NEAR(a.y, b.y, 1e-5f);
}

TEST(Transform2D_Float, Decompose_RoundTrip_NoPivot)
{
    // Build matrix with FromTRS (no pivot baked in)
    const float tx = -42.0f, ty = 17.5f;
    const float sx = 1.75f, sy = 0.6f;
    const float ang = std::numbers::pi_v<float> *0.333333f;

    auto M = Matrix2DAffine<float>::FromTRS(tx, ty, ang, sx, sy);

    Vector2D<float> pos;
    float rot = 0.0f;
    Vector2D<float> scl;
    Transform2D<float>::Decompose(M, pos, rot, scl);

    EXPECT_NEAR(pos.x, tx, 1e-5f);
    EXPECT_NEAR(pos.y, ty, 1e-5f);
    EXPECT_NEAR(scl.x, sx, 1e-5f);
    EXPECT_NEAR(scl.y, sy, 1e-5f);
    EXPECT_NEAR(std::sin(rot), std::sin(ang), 1e-5f);
    EXPECT_NEAR(std::cos(rot), std::cos(ang), 1e-5f);

    // Rebuild (no pivot) and compare matrices
    auto R = Matrix2DAffine<float>::Rotation(rot);
    auto S = Matrix2DAffine<float>::Scale(scl.x, scl.y);
    auto T = Matrix2DAffine<float>::Translation(pos.x, pos.y);
    auto M2 = T * R * S;

    EXPECT_TRUE(Matrix2DAffine<float>::NearlyEqualMat(M, M2, 1e-5f));
}

TEST(Transform2D_Float, Decompose_WithReflection)
{
    // Negative scale on Y introduces reflection (det < 0)
    const float tx = 10.0f, ty = -8.0f;
    const float sx = 2.0f, sy = -1.5f; // reflected
    const float ang = std::numbers::pi_v<float> *0.2f;

    auto M = Matrix2DAffine<float>::FromTRS(tx, ty, ang, sx, sy);

    Vector2D<float> pos;
    float rot = 0.0f;
    Vector2D<float> scl;
    Transform2D<float>::Decompose(M, pos, rot, scl);

    // Position exact
    EXPECT_NEAR(pos.x, tx, 1e-5f);
    EXPECT_NEAR(pos.y, ty, 1e-5f);

    // We expect one axis scale to be negative after reflection handling
    // Convention in your code flips Y if det < 0
    EXPECT_NEAR(scl.x, std::abs(sx), 1e-5f);
    EXPECT_NEAR(scl.y, -std::abs(sy), 1e-5f);

    // Rotation should still represent the proper rotation (up to 2π)
    EXPECT_NEAR(std::sin(rot), std::sin(ang), 1e-5f);
    EXPECT_NEAR(std::cos(rot), std::cos(ang), 1e-5f);

    // Rebuild linear part and check determinant sign
    auto R = Matrix2DAffine<float>::Rotation(rot);
    auto S = Matrix2DAffine<float>::Scale(scl.x, scl.y);
    auto T = Matrix2DAffine<float>::Translation(pos.x, pos.y);
    auto M2 = T * R * S;

    // Determinant negative => reflection preserved
    EXPECT_LT(M2.Determinant(), 0.0f);
}

TEST(Transform2D_Float, ComposeThenDecompose_MatchesChildInParentSpace_NoPivot)
{
    Transform2D<float> parent, child;

    parent.Position = { 5.0f, -2.0f };
    parent.Scale = { 1.1f, 1.1f }; // uniform => no shear contribution
    parent.Rotation = std::numbers::pi_v<float> *0.15f;
    parent.Pivot = { 0.0f, 0.0f };

    child.Position = { -3.0f, 4.0f };
    child.Scale = { 1.6f, 1.6f }; // uniform => no shear contribution
    child.Rotation = std::numbers::pi_v<float> *-0.35f;
    child.Pivot = { 0.0f, 0.0f };

    auto C = Transform2D<float>::Compose(parent, child);

    Vector2D<float> pos; float rot = 0.0f; Vector2D<float> scl;
    Transform2D<float>::Decompose(C, pos, rot, scl);

    auto TW = Matrix2DAffine<float>::Translation(pos.x, pos.y)
        * Matrix2DAffine<float>::Rotation(rot)
        * Matrix2DAffine<float>::Scale(scl.x, scl.y);

    auto Expected = parent.ToMatrix() * child.ToMatrix();
    EXPECT_TRUE(Matrix2DAffine<float>::NearlyEqualMat(TW, Expected, 1e-5f));
}
