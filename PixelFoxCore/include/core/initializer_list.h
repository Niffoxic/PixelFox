/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 *  Description:
 *      Since I cant use initializer list that comes with the default stl
 *      I implemented a minimal initializer list myself I hope it works out!
 *  -----------------------------------------------------------------------------
 */

#pragma once

#include <cstddef>
#include <sal.h>
#include <yvals_core.h>

namespace fox
{
    template<class T>
    class initializer_list
    {
    public:
        using value_type      = T;
        using reference       = const T&;
        using const_reference = const T&;
        using size_type       = std::size_t;
        using iterator        = const T*;
        using const_iterator  = const T*;

        _CONSTEXPR20 initializer_list() noexcept
            : m_data(nullptr), m_size(0)
        {}

        _CONSTEXPR20 initializer_list
        (
            _Pre_satisfies_(size == 0 || data != nullptr)
            _In_reads_opt_ (size) const T* data,
            _In_                  size_type size
        ) noexcept
            : m_data(data), m_size(size)
        {}

        // Observerss
        _NODISCARD _CONSTEXPR20 _Ret_maybenull_
        const T* begin() const noexcept { return m_data;           }

        _NODISCARD _CONSTEXPR20 _Ret_maybenull_
        const T* end  () const noexcept { return m_data + m_size;  }

        _NODISCARD _CONSTEXPR20
        size_type size() const noexcept { return m_size;           }

        _NODISCARD _CONSTEXPR20 _Ret_maybenull_
        const T* data () const noexcept { return m_data;           }

    private:
        const T*  m_data;
        size_type m_size;
    };

    // Range-for helpers
    template<class T>
    _NODISCARD _CONSTEXPR20 _Ret_maybenull_
    const T* begin(_In_ initializer_list<T> il) noexcept { return il.begin();   }

    template<class T>
    _NODISCARD _CONSTEXPR20 _Ret_maybenull_
    const T* end  (_In_ initializer_list<T> il) noexcept { return il.end();     }

} // namespace fox
