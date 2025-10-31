/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <new>
#include <utility>
#include <cassert>
#include <algorithm>
#include <memory> 
#include <type_traits>
#include <limits>
#include <stdexcept>

namespace fox
{
    template<typename T,
    typename _Alloc = std::allocator<T>>
    class vector
    {
    public:
        using value_type = T;
        using alloc_type         = _Alloc;
        using alloc_traits       = std::allocator_traits<alloc_type>;
        using reference = T&;
        using const_reference = const T&;
        using pointer       = T*;
        using const_pointer = const T*;
        using size_type          = size_t;

    public:
        //~ constructors
        vector() = default;

        template<typename...Args>
        vector(size_type size, Args...args)
        {
            allocate(size);
            construct(size, std::forward<Args>(args)...);
            m_nSize = size;
        }

        //~ operators
        reference operator[](size_type index)
        {
            assert(index < m_nSize && "Index out of Bound");
            return *(m_begin + index);
        }

        //~ configurations
        void resize(const size_type& size)
        {
            deallocate(m_nSize);
            allocate(size);
            construct(size);
            m_nSize = size;
        }

        //~ queries
        bool   empty   () const { return not m_nSize; }
        size_t size    () const { return m_nSize;     }
        size_t capacity() const { return m_nCapacity; }

        //~ getters
        pointer data() { return m_begin; }

#pragma region ITERATOR_COMPATIBILITY
        class Iterator
        {
        public:
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::random_access_iterator_tag;
        public:
            Iterator() noexcept = default;
            explicit Iterator(pointer ptr)
                : m_pointer(ptr)
            {}

            //~ deference
            reference operator*() const noexcept { return *m_pointer; }
            pointer operator->() const noexcept  { return m_pointer; }

            //~ prefix
            Iterator& operator++() noexcept { ++m_pointer; return *this; }
            Iterator& operator--() noexcept { --m_pointer; return *this; }

            //~ postfix
            Iterator operator++(int) noexcept { Iterator tmp(*this); ++(*this); return tmp; }
            Iterator operator--(int) noexcept { Iterator tmp(*this); --(*this); return tmp; }

            //~ arithematic
            Iterator& operator+=(difference_type n) noexcept { m_pointer += n; return *this; }
            Iterator& operator-=(difference_type n) noexcept { m_pointer -= n; return *this; }

            Iterator operator+(difference_type n) noexcept { return Iterator(m_pointer + n); }
            Iterator operator-(difference_type n) noexcept { return Iterator(m_pointer - n); }

            difference_type operator-(const Iterator& other) noexcept
            {
                return m_pointer - other.m_pointer;
            }

            //~ comparision
            bool operator==(const Iterator& other) noexcept { return m_pointer == other.m_pointer; }
            bool operator!=(const Iterator& other) noexcept { return m_pointer != other.m_pointer; }
            bool operator<(const Iterator& other) noexcept { return m_pointer < other.m_pointer; }
            bool operator>(const Iterator& other) noexcept { return m_pointer > other.m_pointer; }
            bool operator<=(const Iterator& other) noexcept { return m_pointer <= other.m_pointer; }
            bool operator>=(const Iterator& other) noexcept { return m_pointer >= other.m_pointer; }

        private:
            pointer m_pointer;
        };
#pragma endregion

        Iterator begin() noexcept
        {
            return Iterator(m_begin);
        }

        Iterator end() noexcept
        {
            return Iterator(m_begin + m_nSize);
        }

        const Iterator begin() const noexcept
        {
            return m_begin;
        }

        const Iterator end() const noexcept
        {
            return m_begin + m_nSize;
        }

    private:
        //~ Helpers
        void allocate(const size_type& size) 
        {
            m_begin = alloc_traits::allocate(m_allocator, size);
            m_nCapacity = size;
        }
        
        void construct(const size_type& size)
        {
            for (size_type i = 0; i < size; i++)
            {
                alloc_traits::construct(m_allocator, m_begin + i);
            }
        }

        template<typename...Args>
        void construct(const size_type& size, Args...args)
        {
            for (size_type i = 0; i < size; i++)
            {
                alloc_traits::construct(
                    m_allocator,
                    m_begin + i,
                    args...
                );
            }
        }

        void deallocate(const size_type& size)
        {
            destroy(size);
            alloc_traits::deallocate(m_allocator, m_begin, size);
        }

        void destroy(const size_type& size)
        {
            for (size_type i = 0; i < size; i++)
                alloc_traits::destroy(m_allocator, m_begin + i);
        }

    private:
        alloc_type m_allocator{};
        size_type m_nSize    { 0u };
        size_type m_nCapacity{ 0u };
        pointer      m_begin    { nullptr };

    };
} // namespace fox
