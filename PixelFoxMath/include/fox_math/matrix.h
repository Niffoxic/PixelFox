#pragma once
#include "PixelFoxMathAPI.h"

#include <type_traits>
#include <cmath>
#include <sal.h>
#include <crtdbg.h>

#include "fox_math/vector.h"


namespace fox_math 
{
    template<typename T,
    typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    class Matrix2DAffine
    {
    public:
        T matrix[2][3];

    public:
        //~ Constructors
        _Success_(true)
        _CONSTEXPR20 Matrix2DAffine() noexcept
            : matrix{ { T(1), T(0), T(0) },
                      { T(0), T(1), T(0) } } //~ setting identity by default
        {}

        _Success_(true)
        _CONSTEXPR20 Matrix2DAffine(_In_ T m00,_In_ T m01, _In_ T m02,
                _In_ T m10, _In_ T m11, _In_ T m12) noexcept
            : 
            matrix{ { m00, m01, m02 },
                    { m10, m11, m12 } }
        {}

        //~ transformatiions
        _Check_return_ _NODISCARD
        static _CONSTEXPR20 Matrix2DAffine Identity() noexcept
        {
            return Matrix2DAffine{};
        }

        _Check_return_ _NODISCARD
        static _CONSTEXPR20 Matrix2DAffine Translation(_In_ T x, _In_ T y) noexcept
        {
            return { T(1), T(0), x,
                     T(0), T(1), y };
        }

        _Check_return_ _NODISCARD
        static Matrix2DAffine Rotation(_In_ T radians) noexcept
        {
            const T c = static_cast<T>(std::cos(radians));
            const T s = static_cast<T>(std::sin(radians));
            return { c, -s, T(0),
                     s,  c, T(0) };
        }

        _Check_return_ _NODISCARD
        static _CONSTEXPR20 Matrix2DAffine Scale(_In_ T sx, _In_ T sy) noexcept
        {
            return { sx,  T(0), T(0),
                     T(0), sy,  T(0) };
        }

        _Check_return_ _NODISCARD
        static _CONSTEXPR20 Matrix2DAffine Shear(_In_ T shx, _In_ T shy) noexcept
        {
            return { T(1), shx,  T(0),
                     shy,  T(1), T(0) };
        }

        _Check_return_ _NODISCARD
        static Matrix2DAffine FromTRS(_In_ T tx, _In_ T ty,
            _In_ T radians,
            _In_ T sx, _In_ T sy) noexcept
        {
            const T c = static_cast<T>(std::cos(radians));
            const T s = static_cast<T>(std::sin(radians));

            Matrix2DAffine M;
            M.matrix[0][0] = c * sx;
            M.matrix[0][1] = -s * sy;
            M.matrix[0][2] = tx;

            M.matrix[1][0] = s * sx;
            M.matrix[1][1] = c * sy;
            M.matrix[1][2] = ty;
            return M;
        }

        //~ Determinant
        _Check_return_ _NODISCARD
        _CONSTEXPR20 T Determinant() const noexcept
        {
            return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
        }

        //~ Inverse
        _Check_return_ _NODISCARD
        Matrix2DAffine Inversed() const noexcept
        {
            const T det = Determinant();
            if (det == T(0))
                return Identity();

            const T invDet = T(1) / det;

            Matrix2DAffine r;
            r.matrix[0][0] = matrix[1][1]  * invDet;
            r.matrix[0][1] = -matrix[0][1] * invDet;
            r.matrix[1][0] = -matrix[1][0] * invDet;
            r.matrix[1][1] = matrix[0][0]  * invDet;

            r.matrix[0][2] = -(r.matrix[0][0] * matrix[0][2] + r.matrix[0][1] * matrix[1][2]);
            r.matrix[1][2] = -(r.matrix[1][0] * matrix[0][2] + r.matrix[1][1] * matrix[1][2]);

            return r;
        }

        //~ Matrix multiply
        _Check_return_ _NODISCARD
        _CONSTEXPR20 Matrix2DAffine operator*(
            _In_ const Matrix2DAffine& r) const noexcept
        {
            Matrix2DAffine out{};

            out.matrix[0][0] = matrix[0][0] * r.matrix[0][0] + matrix[0][1] * r.matrix[1][0];
            out.matrix[0][1] = matrix[0][0] * r.matrix[0][1] + matrix[0][1] * r.matrix[1][1];
            out.matrix[0][2] = matrix[0][0] * r.matrix[0][2] + matrix[0][1] * r.matrix[1][2] + matrix[0][2];

            out.matrix[1][0] = matrix[1][0] * r.matrix[0][0] + matrix[1][1] * r.matrix[1][0];
            out.matrix[1][1] = matrix[1][0] * r.matrix[0][1] + matrix[1][1] * r.matrix[1][1];
            out.matrix[1][2] = matrix[1][0] * r.matrix[0][2] + matrix[1][1] * r.matrix[1][2] + matrix[1][2];

            return out;
        }

        _Success_(return != nullptr) _Ret_notnull_
        _When_(true, _Post_satisfies_(return == this))
        _CONSTEXPR20 Matrix2DAffine& operator*=(
            _In_ const Matrix2DAffine& r) noexcept
        {
            *this = *this * r;
            return *this;
        }

        //~ Comparison
        _Check_return_ _NODISCARD
        _CONSTEXPR20 bool operator==(_In_ const Matrix2DAffine& rhs) const noexcept
        {
            return  matrix[0][0] == rhs.matrix[0][0] &&
                    matrix[0][1] == rhs.matrix[0][1] &&
                    matrix[0][2] == rhs.matrix[0][2] &&
                    matrix[1][0] == rhs.matrix[1][0] &&
                    matrix[1][1] == rhs.matrix[1][1] &&
                    matrix[1][2] == rhs.matrix[1][2];
        }

        _Check_return_ _NODISCARD
        _CONSTEXPR20 bool operator!=(
            _In_ const Matrix2DAffine& rhs) const noexcept
        {
            return !(*this == rhs);
        }

        //~ Transformations
        _Check_return_ _NODISCARD
        fox_math::Vector2D<T> TransformPoint(
            _In_ const fox_math::Vector2D<T>& p) const noexcept
        {
            return { matrix[0][0] * p.x + matrix[0][1] * p.y + matrix[0][2],
                     matrix[1][0] * p.x + matrix[1][1] * p.y + matrix[1][2] };
        }

        _Check_return_ _NODISCARD
        fox_math::Vector2D<T> TransformDirection(
            _In_ const fox_math::Vector2D<T>& d) const noexcept
        {
            return { matrix[0][0] * d.x + matrix[0][1] * d.y,
                     matrix[1][0] * d.x + matrix[1][1] * d.y };
        }

        //~ Access
        _Check_return_ _NODISCARD
        _CONSTEXPR20 T& At(_In_ int row, _In_ int col) noexcept
        {
            _ASSERTE((row >= 0 && row < 2 && col >= 0 && col < 3)
                && "trying to access matrix but row or col is Out of Range");
            return matrix[row][col];
        }

        _Check_return_ _NODISCARD
        _CONSTEXPR20 T At(_In_ int row, _In_ int col) const noexcept
        {
            _ASSERTE((row >= 0 && row < 2 && col >= 0 && col < 3)
                && "trying to access matrix but row or col is Out of Range");
            return matrix[row][col];
        }

        //~ Helper
        _Check_return_ _NODISCARD
        static _CONSTEXPR20 bool NearlyEqual(_In_ T a, _In_ T b, _In_ T eps) noexcept
        {
            return (a > b ? (a - b) : (b - a)) <= eps;
        }

        _Check_return_ _NODISCARD
        static _CONSTEXPR20 bool NearlyEqualMat(
            _In_ const Matrix2DAffine& A,
            _In_ const Matrix2DAffine& B,
            _In_ T eps) noexcept
        {
            return  NearlyEqual(A.matrix[0][0], B.matrix[0][0], eps) &&
                    NearlyEqual(A.matrix[0][1], B.matrix[0][1], eps) &&
                    NearlyEqual(A.matrix[0][2], B.matrix[0][2], eps) &&
                    NearlyEqual(A.matrix[1][0], B.matrix[1][0], eps) &&
                    NearlyEqual(A.matrix[1][1], B.matrix[1][1], eps) &&
                    NearlyEqual(A.matrix[1][2], B.matrix[1][2], eps);
        }
    };
} // namespace fox_math

using FMatrix2DAffine = fox_math::Matrix2DAffine<float>;
