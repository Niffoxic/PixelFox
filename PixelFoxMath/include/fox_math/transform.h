#pragma once

#include "PixelFoxMathAPI.h"
#include <type_traits>
#include <cmath>
#include <sal.h>
#include <crtdbg.h>

#include "fox_math/vector.h"
#include "fox_math/matrix.h"
#include "fox_math/math.h"

namespace fox_math
{
    template<typename T,
    typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    class Transform2D
    {
    public:
        fox_math::Vector2D<T> Position; 
        fox_math::Vector2D<T> Scale;
        T                     Rotation;
        fox_math::Vector2D<T> Pivot;

    public:
        _Success_(true)
        _CONSTEXPR20 Transform2D() noexcept
            : Position(0, 0), Scale(1, 1), Rotation(0), Pivot(0, 0)
        {}

        _Success_(true)
        _CONSTEXPR20 Transform2D(_In_ fox_math::Vector2D<T> pos,
            _In_ T rot,
            _In_ fox_math::Vector2D<T> scl,
            _In_ fox_math::Vector2D<T> piv = { 0,0 }) noexcept
            : Position(pos), Scale(scl), Rotation(rot), Pivot(piv)
        {}

        _Check_return_ _NODISCARD
        fox_math::Matrix2DAffine<T> ToMatrix() const noexcept
        {
            using M = fox_math::Matrix2DAffine<T>;

            return  M::Translation(Position.x, Position.y)
                    * M::Rotation(Rotation)
                    * M::Scale(Scale.x, Scale.y)
                    * M::Translation(-Pivot.x, -Pivot.y);
        }

        _Check_return_ _NODISCARD
        static fox_math::Matrix2DAffine<T> Compose(
            _In_ const Transform2D& left,
            _In_ const Transform2D& right) noexcept
        {
            return left.ToMatrix() * right.ToMatrix();
        }

        _Success_(true)
        static void Decompose(
            _In_ const fox_math::Matrix2DAffine<T>& inMatrix,
            _Out_ fox_math::Vector2D<T>&            outPos,
            _Out_ T&                                outRotRadians,
            _Out_ fox_math::Vector2D<T>&            outScale
        ) noexcept
        {
            //~ extract trnslation
            outPos.x = inMatrix.matrix[0][2];
            outPos.y = inMatrix.matrix[1][2];
            
            const T a = inMatrix.matrix[0][0];
            const T b = inMatrix.matrix[0][1];

            const T c = inMatrix.matrix[1][0];
            const T d = inMatrix.matrix[1][1];

            //~ length of columns
            const T sx = static_cast<T>(std::sqrt(a * a + c * c));
            const T sy = static_cast<T>(std::sqrt(b * b + d * d));

            //~ Handle tiny scales if there's any
            const T eps = static_cast<T>(1e-12);
            T rot = 0;

            if (sx > eps)
            {
                //~ get sin and cos 
                const T na = a / sx;
                const T nc = c / sx;
                rot = static_cast<T>(std::atan2(nc, na));
            }
            else if (sy > eps)
            {
                //~ use second column to get sin and cost
                const T nb = b / sy;
                const T nd = d / sy;
                rot = static_cast<T>(std::atan2(nd, nb) - T(fox_math::PI) / T(2));
            }
            else
            {
                //~ could be a zero vector
                rot = 0;
            }

            //~ iff there's a reflection
            const T det = a * d - b * c;
            fox_math::Vector2D<T> scl(sx, sy);
            if (det < 0)
            {
                //~ Flipping one axis for keeping the rotation intact
                scl.y = -scl.y;
            }

            outRotRadians = rot;
            outScale = scl;
        }
    };
} // namespace fox_math

//~ types
using FTransform2D = fox_math::Transform2D<float>;
