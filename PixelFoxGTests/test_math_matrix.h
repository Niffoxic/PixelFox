/*
 * LLM Generated TestCode
 * reason: limited time I can't implement and write its google test within the time constraint
*/
#pragma once
#include "pch.h"

#include "fox_math/matrix.h"
#include "fox_math/math.h"

using fox_math::Vector2D;
using fox_math::Matrix2DAffine;

static constexpr float kEps_mat = 1e-5f;

TEST(Matrix2DAffine_Float, IdentityBasics)
{
    Matrix2DAffine<float> I; // default is identity
    auto P = Vector2D<float>(3.0f, -2.0f);
    auto D = Vector2D<float>(1.5f, 4.0f);

    auto P2 = I.TransformPoint(P);
    auto D2 = I.TransformDirection(D);

    EXPECT_NEAR(P2.x, P.x, kEps_mat);
    EXPECT_NEAR(P2.y, P.y, kEps_mat);
    EXPECT_NEAR(D2.x, D.x, kEps_mat);
    EXPECT_NEAR(D2.y, D.y, kEps_mat);

    EXPECT_NEAR(I.Determinant(), 1.0f, kEps_mat);
}

TEST(Matrix2DAffine_Float, Translation)
{
    auto T = Matrix2DAffine<float>::Translation(2.0f, 3.0f);

    auto P = Vector2D<float>(1.0f, 1.0f);
    auto D = Vector2D<float>(-4.0f, 5.0f);

    auto P2 = T.TransformPoint(P);
    auto D2 = T.TransformDirection(D);

    EXPECT_NEAR(P2.x, 3.0f, kEps_mat); // 1 + 2
    EXPECT_NEAR(P2.y, 4.0f, kEps_mat); // 1 + 3

    // directions ignore translation
    EXPECT_NEAR(D2.x, -4.0f, kEps_mat);
    EXPECT_NEAR(D2.y, 5.0f, kEps_mat);

    EXPECT_NEAR(T.Determinant(), 1.0f, kEps_mat);
}

TEST(Matrix2DAffine_Float, Rotation90)
{
    const float rad = static_cast<float>(fox_math::PI) * 0.5f; // 90°
    auto R = Matrix2DAffine<float>::Rotation(rad);

    // Direction along +X should rotate to +Y
    auto X = Vector2D<float>(1.0f, 0.0f);
    auto Y = Vector2D<float>(0.0f, 1.0f);

    auto Xr = R.TransformDirection(X);
    EXPECT_NEAR(Xr.x, Y.x, kEps_mat);
    EXPECT_NEAR(Xr.y, Y.y, kEps_mat);

    // A point should also rotate around origin (no translation)
    auto P = Vector2D<float>(2.0f, 0.0f);
    auto Pr = R.TransformPoint(P);
    EXPECT_NEAR(Pr.x, 0.0f, kEps_mat);
    EXPECT_NEAR(Pr.y, 2.0f, kEps_mat);

    EXPECT_NEAR(R.Determinant(), 1.0f, kEps_mat);
}

TEST(Matrix2DAffine_Float, Scale)
{
    auto S = Matrix2DAffine<float>::Scale(2.0f, 3.0f);

    auto D = Vector2D<float>(1.0f, 1.0f);
    auto Ds = S.TransformDirection(D);
    EXPECT_NEAR(Ds.x, 2.0f, kEps_mat);
    EXPECT_NEAR(Ds.y, 3.0f, kEps_mat);

    auto P = Vector2D<float>(-2.0f, 4.0f);
    auto Ps = S.TransformPoint(P);
    EXPECT_NEAR(Ps.x, -4.0f, kEps_mat);
    EXPECT_NEAR(Ps.y, 12.0f, kEps_mat);

    EXPECT_NEAR(S.Determinant(), 6.0f, kEps_mat);
}

TEST(Matrix2DAffine_Float, Shear)
{
    // x' = x + shx * y; y' = shy * x + y
    auto H = Matrix2DAffine<float>::Shear(2.0f, -1.0f);
    auto P = Vector2D<float>(1.0f, 3.0f);

    auto Ph = H.TransformPoint(P);
    // x' = 1 + 2*3 = 7
    // y' = -1*1 + 3 = 2
    EXPECT_NEAR(Ph.x, 7.0f, kEps_mat);
    EXPECT_NEAR(Ph.y, 2.0f, kEps_mat);

    // determinant = 1*1 - shx*shy = 1 - (2 * -1) = 3
    EXPECT_NEAR(H.Determinant(), 3.0f, kEps_mat);
}

TEST(Matrix2DAffine_Float, FromTRS_Equals_T_times_R_times_S)
{
    const float tx = 5.0f, ty = -2.0f;
    const float sx = 2.0f, sy = 0.5f;
    const float ang = 0.25f * static_cast<float>(fox_math::PI);

    auto T = Matrix2DAffine<float>::Translation(tx, ty);
    auto R = Matrix2DAffine<float>::Rotation(ang);
    auto S = Matrix2DAffine<float>::Scale(sx, sy);

    // Column vector convention: A * B means apply B first, then A.
    auto TRS_composed = T * (R * S);

    auto TRS_direct = Matrix2DAffine<float>::FromTRS(tx, ty, ang, sx, sy);

    EXPECT_TRUE(Matrix2DAffine<float>::NearlyEqualMat(TRS_composed,
        TRS_direct, 1e-5f));

    // Sanity: apply to a point
    auto P = Vector2D<float>(1.0f, 2.0f);
    auto p1 = TRS_composed.TransformPoint(P);
    auto p2 = TRS_direct.TransformPoint(P);

    EXPECT_NEAR(p1.x, p2.x, kEps_mat);
    EXPECT_NEAR(p1.y, p2.y, kEps_mat);
}

TEST(Matrix2DAffine_Float, CompositionOrder)
{
    auto T = Matrix2DAffine<float>::Translation(10.0f, 0.0f);
    auto R = Matrix2DAffine<float>::Rotation(static_cast<float>(fox_math::PI) * 0.5f); // 90°

    // Apply R then T:
    auto M = T * R;

    auto P = Vector2D<float>(1.0f, 0.0f);
    // R * P = (0, 1); T * (0,1) = (10, 1)
    auto result = M.TransformPoint(P);

    EXPECT_NEAR(result.x, 10.0f, kEps_mat);
    EXPECT_NEAR(result.y, 1.0f, kEps_mat);
}

TEST(Matrix2DAffine_Float, InverseRoundTrip)
{
    const float ang = 0.33f * static_cast<float>(fox_math::PI);
    auto M = Matrix2DAffine<float>::FromTRS(3.0f, -4.0f, ang, 2.0f, 0.75f);

    auto Minv = M.Inversed();

    // Check that M^{-1} * M behaves like identity on both point and direction
    auto P = Vector2D<float>(-1.5f, 2.25f);
    auto D = Vector2D<float>(3.0f, -2.0f);

    auto P_id = (Minv * M).TransformPoint(P);
    auto D_id = (Minv * M).TransformDirection(D);

    EXPECT_NEAR(P_id.x, P.x, kEps_mat);
    EXPECT_NEAR(P_id.y, P.y, kEps_mat);
    EXPECT_NEAR(D_id.x, D.x, kEps_mat);
    EXPECT_NEAR(D_id.y, D.y, kEps_mat);
}

TEST(Matrix2DAffine_Float, InverseSingularReturnsIdentity)
{
    // sy = 0 => singular; Inversed() returns Identity by design
    auto M = Matrix2DAffine<float>::Scale(2.0f, 0.0f);
    auto Minv = M.Inversed();

    auto I = Matrix2DAffine<float>::Identity();
    EXPECT_TRUE(Matrix2DAffine<float>::NearlyEqualMat(Minv, I, 1e-6f));
}

TEST(Matrix2DAffine_Float, OperatorMulAssign)
{
    auto A = Matrix2DAffine<float>::Translation(1.0f, 2.0f);
    auto B = Matrix2DAffine<float>::Scale(2.0f, 3.0f);

    auto C1 = A * B;
    A *= B;

    EXPECT_TRUE(Matrix2DAffine<float>::NearlyEqualMat(A, C1, 1e-6f));
}

TEST(Matrix2DAffine_Float, TransformPointVsDirection)
{
    auto M = Matrix2DAffine<float>::Translation(5.0f, -7.0f)
        * Matrix2DAffine<float>::Rotation(static_cast<float>(fox_math::PI) * 0.25f); // 45°

    auto P = Vector2D<float>(1.0f, 0.0f);
    auto D = Vector2D<float>(1.0f, 0.0f);

    auto Pt = M.TransformPoint(P);
    auto Dt = M.TransformDirection(D);

    // Direction should not include translation
    EXPECT_NEAR(Pt.x - Dt.x, 5.0f, 1e-4f);
    EXPECT_NEAR(Pt.y - Dt.y, -7.0f, 1e-4f);
}

TEST(Matrix2DAffine_Float, EqualityOperators)
{
    auto A = Matrix2DAffine<float>::FromTRS(1.0f, 2.0f, 0.1f, 2.0f, 3.0f);
    auto B = A;
    auto C = Matrix2DAffine<float>::FromTRS(1.0f, 2.0f, 0.1f, 2.0f, 3.001f);

    EXPECT_TRUE(A == B);
    EXPECT_FALSE(A != B);
    EXPECT_FALSE(A == C); // exact-compare is false
    EXPECT_TRUE(Matrix2DAffine<float>::NearlyEqualMat(A, C, 0.01f)); // near-compare is true
}
