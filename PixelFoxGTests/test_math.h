/*
 * LLM Generated TestCode
 * reason: limited time I can't implement and write its google test within the time constraint
*/
#pragma once
#include "pch.h"

#include <cmath>
#include <numbers>
#include <type_traits>

#include "fox_math/math.h"

using namespace fox_math;

static constexpr float  kEpsF = 1e-6f;
static constexpr double kEpsD = 1e-12;

// ------------------------------
// Compile-time sanity
// ------------------------------
static_assert(std::is_same_v<decltype(PI), const double>, "PI should be double");
static_assert(std::is_floating_point_v<decltype(PI_v<float>)>, "PI_v<T> must be floating point");
static_assert(std::is_same_v<decltype(DEG2RAD_v<float>), const float>, "Typed constants must match T");

// ------------------------------
// Constants
// ------------------------------
TEST(MathConstants, NumericValues)
{
    EXPECT_NEAR(PI, 3.14159265358979323846, kEpsD);
    EXPECT_NEAR(TWO_PI, 6.28318530717958647692, kEpsD);
    EXPECT_NEAR(HALF_PI, 1.57079632679489661923, kEpsD);

    // Typed constants
    EXPECT_NEAR(PI_v<float>, static_cast<float>(PI), kEpsF);
    EXPECT_NEAR(TWO_PI_v<float>, static_cast<float>(TWO_PI), kEpsF);
    EXPECT_NEAR(HALF_PI_v<float>, static_cast<float>(HALF_PI), kEpsF);
    EXPECT_NEAR(DEG2RAD_v<float>, static_cast<float>(PI / 180.0), kEpsF);
    EXPECT_NEAR(RAD2DEG_v<float>, static_cast<float>(180.0 / PI), kEpsF);
}

// ------------------------------
// Min / Max / Abs
// ------------------------------
TEST(MathBasics, MinMaxAbs)
{
    EXPECT_EQ(Min(3, 5), 3);
    EXPECT_EQ(Min(-4, -7), -7);
    EXPECT_EQ(Max(3, 5), 5);
    EXPECT_EQ(Max(-4, -7), -4);

    EXPECT_EQ(Abs(5), 5);
    EXPECT_EQ(Abs(-5), 5);

    EXPECT_FLOAT_EQ(Abs(-2.5f), 2.5f);
    EXPECT_DOUBLE_EQ(Abs(-2.5), 2.5);
}

// ------------------------------
// Clamp / Lerp / SmoothStep
// ------------------------------
TEST(MathInterp, Clamp)
{
    EXPECT_EQ(Clamp(5, 0, 10), 5);
    EXPECT_EQ(Clamp(-2, 0, 10), 0);
    EXPECT_EQ(Clamp(12, 0, 10), 10);

    EXPECT_FLOAT_EQ(Clamp(0.25f, 0.0f, 1.0f), 0.25f);
    EXPECT_FLOAT_EQ(Clamp(-1.0f, 0.0f, 1.0f), 0.0f);
    EXPECT_FLOAT_EQ(Clamp(2.0f, 0.0f, 1.0f), 1.0f);
}

TEST(MathInterp, Lerp)
{
    EXPECT_EQ(Lerp(0, 10, 0.0f), 0);
    EXPECT_EQ(Lerp(0, 10, 1.0f), 10);
    EXPECT_EQ(Lerp(0, 10, 0.5f), 5);

    EXPECT_NEAR(Lerp(2.0, 6.0, 0.25), 3.0, kEpsD);
}

TEST(MathInterp, SmoothStep)
{
    // SmoothStep(edge0, edge1, x) should clamp and be 0 at start, 1 at end (after normalization)
    EXPECT_NEAR(SmoothStep(0.0f, 1.0f, -1.0f), 0.0f, kEpsF);
    EXPECT_NEAR(SmoothStep(0.0f, 1.0f, 0.0f), 0.0f, kEpsF);
    EXPECT_NEAR(SmoothStep(0.0f, 1.0f, 1.0f), 1.0f, kEpsF);
    EXPECT_NEAR(SmoothStep(0.0f, 1.0f, 2.0f), 1.0f, kEpsF);

    // Midpoint (0.5) remains 0.5 for standard smoothstep
    EXPECT_NEAR(SmoothStep(0.0f, 1.0f, 0.5f), 0.5f, 1e-6f);

    // Non-zero range
    EXPECT_NEAR(SmoothStep(2.0, 6.0, 2.0), 2.0, kEpsD);
    EXPECT_NEAR(SmoothStep(2.0, 6.0, 6.0), 6.0, kEpsD);
    EXPECT_NEAR(SmoothStep(2.0, 6.0, 4.0), 4.0, 1e-12); // middle stays middle
}

// ------------------------------
// Deg <-> Rad + Sin/Cos helpers
// ------------------------------
TEST(Angles, DegRadConversions)
{
    EXPECT_NEAR(DegToRad(180.0), PI, kEpsD);
    EXPECT_NEAR(DegToRad(90.0f), HALF_PI_v<float>, kEpsF);

    EXPECT_NEAR(RadToDeg(PI), 180.0, kEpsD);
    EXPECT_NEAR(RadToDeg(HALF_PI_v<float>), 90.0f, kEpsF);

    // Round-trip
    double d = 37.5;
    EXPECT_NEAR(RadToDeg(DegToRad(d)), d, kEpsD);

    float f = 123.45f;
    EXPECT_NEAR(RadToDeg(DegToRad(f)), f, 1e-5f);
}

TEST(Angles, SinCosHelpers)
{
    // Radians
    EXPECT_NEAR(SinRad(0.0f), 0.0f, kEpsF);
    EXPECT_NEAR(CosRad(0.0f), 1.0f, kEpsF);

    EXPECT_NEAR(SinRad(HALF_PI_v<float>), 1.0f, 1e-6f);
    EXPECT_NEAR(CosRad(HALF_PI_v<float>), 0.0f, 1e-5f);

    // Degrees
    EXPECT_NEAR(SinDeg(90.0f), 1.0f, 1e-6f);
    EXPECT_NEAR(CosDeg(90.0f), 0.0f, 1e-5f);
    EXPECT_NEAR(CosDeg(180.0f), -1.0f, 1e-6f);
}

// ------------------------------
// ApproxEqual
// ------------------------------
TEST(Approx, AbsoluteEpsilon)
{
    EXPECT_TRUE(ApproxEqual(1.0, 1.0 + 1e-10, 1e-9));
    EXPECT_FALSE(ApproxEqual(1.0, 1.0 + 1e-6, 1e-9));

    EXPECT_TRUE(ApproxEqual(10, 10, 0));
    EXPECT_TRUE(ApproxEqual(10, 11, 1));
    EXPECT_FALSE(ApproxEqual(10, 12, 1));
}

TEST(Approx, RelativeAndAbsolute)
{
    // Very small numbers: absolute epsilon dominates
    EXPECT_TRUE(ApproxEqualRelAbs(1e-9, 2e-9, 1e-8, 1e-4));

    // Larger numbers: relative epsilon dominates
    double a = 1000.0;
    double b = 1000.0 + 1e-6; // relative diff ~1e-9
    EXPECT_TRUE(ApproxEqualRelAbs(a, b, 1e-12, 1e-6));

    // Farther apart: should fail
    EXPECT_FALSE(ApproxEqualRelAbs(1.0, 1.01, 1e-6, 1e-4));
}
