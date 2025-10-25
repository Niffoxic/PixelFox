#pragma once

#include "PixelFoxMathAPI.h"
#include <type_traits>
#include <cmath>
#include <sal.h>
#include <crtdbg.h>

namespace fox_math
{
    template<typename T = int,
        typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    class Vector2D
    {
    public:
        T x;
        T y;

    public:
        //~ Constructors
        _Success_(true)
        _CONSTEXPR20 Vector2D() noexcept: x(0), y(0) {}

        _Success_(true)
        _CONSTEXPR20 Vector2D(_In_ T _x, _In_ T _y) noexcept
        : x(_x), y(_y)
        {}

        //~ Unary operations
        _Check_return_ _NODISCARD
        _CONSTEXPR20 Vector2D operator+() const noexcept
        {
            return *this;
        }

        _Check_return_ _NODISCARD
        _CONSTEXPR20 Vector2D operator-() const noexcept
        {
            return { -x, -y };
        }

        //~ Arithmetic operation between 2 2D vectors
        _Check_return_ _NODISCARD
        _CONSTEXPR20 Vector2D operator+(_In_ const Vector2D& other) const noexcept
        {
            return { x + other.x, y + other.y };
        }

       _Check_return_ _NODISCARD
       _CONSTEXPR20 Vector2D operator-(_In_ const Vector2D& other) const noexcept
       {
            return { x - other.x, y - other.y };
       }

        _Success_(return != nullptr) _Ret_notnull_
        _When_(true, _Post_satisfies_(return == this))
        Vector2D& operator+=(_In_ const Vector2D& other) noexcept
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        _Success_(return != nullptr)
        _Ret_notnull_
        _When_(true, _Post_satisfies_(return == this))
        Vector2D& operator-=(_In_ const Vector2D& other) noexcept
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        //~ Arithmetic operation between a scalar and a vector2D
        _Check_return_ _NODISCARD
        _CONSTEXPR20 Vector2D operator*(_In_ T scalar) const noexcept
        {
            return { x * scalar, y * scalar };
        }

        _Check_return_ _NODISCARD
        _CONSTEXPR20 Vector2D operator/(_In_ T scalar) const noexcept
        {
            return { x / scalar, y / scalar };
        }

        _Success_(return != nullptr) _Ret_notnull_
        _When_(true, _Post_satisfies_(return == this))
        Vector2D& operator*=(_In_ T scalar) noexcept
        {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        _Success_(return != nullptr) _Ret_notnull_
        _When_(true, _Post_satisfies_(return == this))
        Vector2D& operator/=(_In_ T scalar) noexcept
        {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        //~ Comparisons operations
        _Check_return_ _NODISCARD
        _CONSTEXPR20 bool operator==(_In_ const Vector2D& rhs) const noexcept
        {
            return x == rhs.x && y == rhs.y;
        }

        _Check_return_ _NODISCARD
        _CONSTEXPR20 bool operator!=(_In_ const Vector2D& rhs) const noexcept
        {
            return !(*this == rhs);
        }

        //~ Access through []
        _Check_return_ _NODISCARD
        _CONSTEXPR20 T& operator[](_In_ size_t i) noexcept
        {
            _ASSERTE(i < 2 && "Trying to Access Vector2D with higher Index Val");
            return (i == 0) ? x : y;
        }

        _Check_return_ _NODISCARD
        _CONSTEXPR20 T operator[](_In_ size_t i) const noexcept
        {
            _ASSERTE(i < 2 && "Trying to Access Vector2D with higher Index Val");
            return (i == 0) ? x : y;
        }

        //~ Vector Related Formulas
        _Check_return_ _NODISCARD
        _CONSTEXPR20 T Dot(_In_ const Vector2D& rhs) const noexcept
        {
            return x * rhs.x + y * rhs.y;
        }

        _Check_return_ _NODISCARD
        _CONSTEXPR20 T Cross(_In_ const Vector2D& rhs) const noexcept
        {
            return x * rhs.y - y * rhs.x;
        }

        _Check_return_ _NODISCARD
        _CONSTEXPR20 T LengthSq() const noexcept
        {
            return x * x + y * y;
        }

        _Check_return_ _NODISCARD
        T Length() const noexcept
        {
            return static_cast<T>(std::sqrt(LengthSq()));
        }

        _Check_return_ _NODISCARD
        Vector2D Normalized() const noexcept
        {
            T len = Length();
            return (len != static_cast<T>(0)) ? (*this / len) : Vector2D{};
        }

        _Success_(true)
        void Normalize() noexcept
        {
            T len = Length();
            if (len != static_cast<T>(0))
            {
                x /= len;
                y /= len;
            }
        }

        //~ Perpendiculars
        _Check_return_ _NODISCARD
        _CONSTEXPR20 Vector2D PerpendicularCCW() const noexcept
        {
            return { -y, x };
        }

        _Check_return_ _NODISCARD
        _CONSTEXPR20 Vector2D PerpendicularCW() const noexcept
        {
            return { y, -x };
        }

        _Check_return_ _NODISCARD
        Vector2D ProjectOnto(_In_ const Vector2D& onto) const noexcept
        {
            T d = onto.LengthSq();
            return (d != static_cast<T>(0)) ? onto * (Dot(onto) / d) : Vector2D{};
        }

        // n must be normalized
        _Check_return_ _NODISCARD
        Vector2D Reflect(_In_ const Vector2D& n) const noexcept
        {
            return *this - n * (static_cast<T>(2) * Dot(n));
        }

        _Check_return_ _NODISCARD
        Vector2D Rotated(_In_ T radians) const noexcept
        {
            T c = static_cast<T>(std::cos(radians));
            T s = static_cast<T>(std::sin(radians));
            return { x * c - y * s, x * s + y * c };
        }

        //~ Distances
        _Check_return_ _NODISCARD
        static _CONSTEXPR20 T DistanceSq(_In_ const Vector2D& a, _In_ const Vector2D& b) noexcept
        {
            T dx = a.x - b.x;
            T dy = a.y - b.y;
            return dx * dx + dy * dy;
        }

        _Check_return_ _NODISCARD
        static T Distance(_In_ const Vector2D& a, _In_ const Vector2D& b) noexcept
        {
            return static_cast<T>(std::sqrt(DistanceSq(a, b)));
        }

        _Check_return_ _NODISCARD
        static _CONSTEXPR20 Vector2D Lerp(_In_ const Vector2D& a, _In_ const Vector2D& b, _In_ T t) noexcept
        {
            return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
        }

        _Check_return_ _NODISCARD
        static _CONSTEXPR20 Vector2D Clamp(_In_ const Vector2D& v, _In_ const Vector2D& lo, _In_ const Vector2D& hi) noexcept
        {
            T cx = (v.x < lo.x) ? lo.x : (v.x > hi.x ? hi.x : v.x);
            T cy = (v.y < lo.y) ? lo.y : (v.y > hi.y ? hi.y : v.y);
            return { cx, cy };
        }

        //~ Helpers
        _Check_return_ _NODISCARD
        static _CONSTEXPR20 bool NearlyEqual(_In_ T a, _In_ T b, _In_ T eps) noexcept
        {
            return (a > b ? (a - b) : (b - a)) <= eps;
        }

        _Check_return_ _NODISCARD
        static _CONSTEXPR20 bool NearlyEqualVec(_In_ const Vector2D& a, _In_ const Vector2D& b, _In_ T eps) noexcept
        {
            return NearlyEqual(a.x, b.x, eps) && NearlyEqual(a.y, b.y, eps);
        }

        _Check_return_ _NODISCARD
        static bool IsFinite(_In_ const Vector2D& v) noexcept
        {
            using std::isfinite;
            return isfinite(v.x) && isfinite(v.y);
        }

        _Check_return_ _NODISCARD
        static Vector2D FromAngle(_In_ T radians) noexcept
        {
            return { static_cast<T>(std::cos(radians)), static_cast<T>(std::sin(radians)) };
        }

        //~ Common constants
        _Check_return_ _NODISCARD
        static _CONSTEXPR20 Vector2D Zero() noexcept
        {
            return { 0, 0 };
        }

        _Check_return_ _NODISCARD
        static _CONSTEXPR20 Vector2D One() noexcept
        {
            return { 1, 1 };
        }

        _Check_return_ _NODISCARD
        static _CONSTEXPR20 Vector2D UnitX() noexcept
        {
            return { 1, 0 };
        }

        _Check_return_ _NODISCARD
        static _CONSTEXPR20 Vector2D UnitY() noexcept
        {
            return { 0, 1 };
        }
    };

    // for left multiply
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    _Check_return_ _NODISCARD
    _CONSTEXPR20 Vector2D<T> operator*(_In_ T s, _In_ const Vector2D<T>& v) noexcept
    {
        return { v.x * s, v.y * s };
    }
} // namespace fox_math

    //~ type defines
using FVector2D = fox_math::Vector2D<float>;
