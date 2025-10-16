/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */
#pragma once

#include <type_traits>
#include <cstddef>
#include <sal.h>
#include <yvals_core.h>

namespace fox
{
    // Iterator category tags
    struct input_iterator_tag                                    {};
    struct forward_iterator_tag      : input_iterator_tag        {};
    struct bidirectional_iterator_tag: forward_iterator_tag      {};
    struct random_access_iterator_tag: bidirectional_iterator_tag{};
    struct contiguous_iterator_tag   : random_access_iterator_tag{};

    // iterator_traits
    //~ Raw pointer specifics
    template<class It>
    struct iterator_traits
    {
        using difference_type   = void;
        using value_type        = void;
        using pointer           = void;
        using reference         = void;
        using iterator_category = void;
    };

    template<class T>
    struct iterator_traits<T*> 
    {
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;
        using iterator_category = contiguous_iterator_tag;
    };

    template<class T>
    struct iterator_traits<const T*>
    {
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = const T*;
        using reference         = const T&;
        using iterator_category = contiguous_iterator_tag;
    };

    // reverse_iterator
    template<class It>
    class reverse_iterator 
    {
    public:
        using iterator_type     = It;
        using traits            = fox::iterator_traits<It>;
        using difference_type   = typename traits::difference_type;
        using value_type        = typename traits::value_type;
        using pointer           = typename traits::pointer;
        using reference         = typename traits::reference;
        using iterator_category = typename traits::iterator_category;
    
        //~ constructors
        _CONSTEXPR20 reverse_iterator() noexcept : m_it() {}
        _CONSTEXPR20 explicit reverse_iterator(It it) noexcept : m_it(it) {}

        // returns forward iterator right to the element we refer to
        _NODISCARD _CONSTEXPR20 It base() const noexcept { return m_it; }

        // dereference
        _NODISCARD _CONSTEXPR20 reference operator*() const noexcept 
        {
            It tmp = m_it;
            --tmp;               // point left to the base()
            return *tmp;
        }

        _NODISCARD _CONSTEXPR20 pointer operator->() const noexcept
        { 
            return fox::addressof(operator*());
        }

        // indexing
        _NODISCARD _CONSTEXPR20 reference operator[](difference_type n) const noexcept 
        {
            return *(*this + n);
        }

        // operators
        _CONSTEXPR20 reverse_iterator& operator++()    noexcept { --m_it; return *this; }
        _CONSTEXPR20 reverse_iterator  operator++(int) noexcept { reverse_iterator tmp(*this); --m_it; return tmp; }

        _CONSTEXPR20 reverse_iterator& operator--()    noexcept { ++m_it; return *this; }
        _CONSTEXPR20 reverse_iterator  operator--(int) noexcept { reverse_iterator tmp(*this); ++m_it; return tmp; }

        _NODISCARD _CONSTEXPR20
        reverse_iterator operator+(difference_type n) const noexcept
        {
            return reverse_iterator(m_it - n);
        }

        _CONSTEXPR20
        reverse_iterator& operator+=(difference_type n) noexcept
        {
            m_it -= n; return *this;
        }

        _NODISCARD _CONSTEXPR20
        reverse_iterator operator-(difference_type n) const noexcept 
        {
            return reverse_iterator(m_it + n);
        }

        _CONSTEXPR20 
        reverse_iterator& operator-=(difference_type n) noexcept
        {
            m_it += n; return *this;
        }

        friend _NODISCARD _CONSTEXPR20
        bool operator==(const reverse_iterator& a, const reverse_iterator& b) noexcept 
        {
            return a.m_it == b.m_it;
        }

        friend _NODISCARD _CONSTEXPR20 
        bool operator!=(const reverse_iterator& a, const reverse_iterator& b) noexcept
        {
            return !(a == b);
        }

        friend _NODISCARD _CONSTEXPR20
        bool operator< (const reverse_iterator& a, const reverse_iterator& b) noexcept 
        {
            return b.m_it < a.m_it;
        }

        friend _NODISCARD _CONSTEXPR20
        bool operator> (const reverse_iterator& a, const reverse_iterator& b) noexcept
        {
            return b < a;
        }

        friend _NODISCARD _CONSTEXPR20 
        bool operator<=(const reverse_iterator& a, const reverse_iterator& b) noexcept 
        {
            return !(b < a);
        }

        friend _NODISCARD _CONSTEXPR20
        bool operator>=(const reverse_iterator& a, const reverse_iterator& b) noexcept
        {
            return !(a < b);
        }

        // distance between two reverse_iterators
        friend _NODISCARD _CONSTEXPR20 
        difference_type operator-(const reverse_iterator& a, const reverse_iterator& b) noexcept
        {
            if (b.m_it > a.m_it) return b.m_it - a.m_it;
            return a.m_it - b.m_it;
        }

        // friend + for n + iter
        friend _NODISCARD _CONSTEXPR20 
        reverse_iterator operator+(difference_type n, const reverse_iterator& it) noexcept
        {
            return it + n;
        }

    private:
        It m_it;
    };

} // namespace fox
