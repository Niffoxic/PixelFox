#pragma once

#include "PixelFoxMathAPI.h"

namespace fox_math 
{
    // ============================================================
        // Constants (double precision) + templated typed variants
        // ============================================================
    inline constexpr double PI = 3.14159265358979323846;
    inline constexpr double TWO_PI = 6.28318530717958647692;
    inline constexpr double HALF_PI = 1.57079632679489661923;
    inline constexpr double DEG2RADd = PI / 180.0;
    inline constexpr double RAD2DEGd = 180.0 / PI;

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    inline constexpr T PI_v = static_cast<T>(PI);

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    inline constexpr T TWO_PI_v = static_cast<T>(TWO_PI);

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    inline constexpr T HALF_PI_v = static_cast<T>(HALF_PI);

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    inline constexpr T DEG2RAD_v = static_cast<T>(DEG2RADd);

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    inline constexpr T RAD2DEG_v = static_cast<T>(RAD2DEGd);


    template<typename T>
    _Check_return_ _NODISCARD
    _CONSTEXPR20 T Min(_In_ T a, _In_ T b) noexcept
    {
        return (a < b) ? a : b;
    }

    //template<typename T1, typename T2>
    //_Check_return_ _NODISCARD
    //_CONSTEXPR20 auto Min(_In_ T1&& left, _In_ T2&& right) noexcept
    //    -> std::decay_t<decltype(true ? left : right)>
    //{
    //    return (left > right) ? std::forward<T1>(left) :
    //        std::forward<T2>(right);
    //}

    template<typename T>
    _Check_return_ _NODISCARD
    _CONSTEXPR20 T Max(_In_ T a, _In_ T b) noexcept
    {
        return (a > b) ? a : b;
    }

    template<typename T>
    _Check_return_ _NODISCARD
    _CONSTEXPR20 T Abs(_In_ T x) noexcept
    {
        return (x < T(0)) ? -x : x;
    }

    template<typename T>
    _Check_return_ _NODISCARD
    _CONSTEXPR20 T Clamp(_In_ T v, _In_ T lo, _In_ T hi) noexcept
    {
        _ASSERTE(!(hi < lo) && "Clamp(): high must be >= low");
        return (v < lo) ? lo : (v > hi ? hi : v);
    }

    template<typename T, typename U>
    _Check_return_ _NODISCARD
    _CONSTEXPR20 auto Lerp(_In_ T a, _In_ T b, _In_ U t) noexcept
    -> decltype(a + (b - a) * t)
    {
        return a + (b - a) * t;
    }

    // SmoothStep edge0->edge1 (cubic Hermite), clamps t to [0,1]
    template<typename T, typename U>
    _Check_return_ _NODISCARD
    _CONSTEXPR20 auto SmoothStep(_In_ T edge0, _In_ T edge1, _In_ U x) noexcept
    -> decltype(Lerp(edge0, edge1, x))
    {
        _ASSERTE(!(edge1 == edge0) && "SmoothStep(): edges must differ");
        using R = decltype(Lerp(edge0, edge1, x));
        auto t = Clamp<R>(static_cast<R>((x - edge0) / (edge1 - edge0)), R(0), R(1));
        // t * t * (3 - 2 * t)
        auto s = t * t * (R(3) - R(2) * t);
        return Lerp<R>(static_cast<R>(edge0), static_cast<R>(edge1), s);
    }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    _Check_return_ _NODISCARD
    _CONSTEXPR20 T DegToRad(_In_ T deg) noexcept
    {
        return deg * DEG2RAD_v<T>;
    }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    _Check_return_ _NODISCARD
    _CONSTEXPR20 T RadToDeg(_In_ T rad) noexcept
    {
        return rad * RAD2DEG_v<T>;
    }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    _Check_return_ _NODISCARD
    inline T SinRad(_In_ T r) noexcept
    {
        return static_cast<T>(std::sin(r));
    }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    _Check_return_ _NODISCARD
    inline T CosRad(_In_ T r) noexcept
    {
        return static_cast<T>(std::cos(r));
    }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    _Check_return_ _NODISCARD
    inline T SinDeg(_In_ T deg) noexcept
    {
        return SinRad(DegToRad(deg));
    }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    _Check_return_ _NODISCARD
    inline T CosDeg(_In_ T deg) noexcept
    {
        return CosRad(DegToRad(deg));
    }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    _Check_return_ _NODISCARD
    _CONSTEXPR20 bool ApproxEqual(_In_ T a, _In_ T b, _In_ T eps) noexcept
    {
        return Abs(a - b) <= eps;
    }

    // relative and absolute check for floating points only
    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    _Check_return_ _NODISCARD
    inline bool ApproxEqualRelAbs(_In_ T a, _In_ T b, _In_ T absEps, _In_ T relEps) noexcept
    {
        T diff = Abs(a - b);
        if (diff <= absEps) return true;
        return diff <= relEps * Max(Abs(a), Abs(b));
    }
}
