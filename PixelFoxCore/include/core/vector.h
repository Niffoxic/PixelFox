/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once

#include "PixelFoxCoreAPI.h"
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <new>
#include <utility>
#include <cassert>
#include <algorithm>
#include <compare> 
#include <memory>
#include <type_traits>
#include <limits>
#include <stdexcept>
#include <sal.h>

namespace fox
{
    template<typename T, typename _Alloc = std::allocator<T>>
    class vector
    {
    public:
        //~ related to types
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;


        //~ related to allocator
        using alloc_type = _Alloc;
        using alloc_traits = std::allocator_traits<alloc_type>;
        // TODO: Add propagation on copy, move and swap operations
        using POCCA = typename alloc_traits::propagate_on_container_copy_assignment;
        using POCMA = typename alloc_traits::propagate_on_container_move_assignment;
        using POCS  = typename alloc_traits::propagate_on_container_swap;
        using IsAlwaysEqual = typename alloc_traits::is_always_equal;
    public:
        vector() noexcept = default;

        ~vector() noexcept
        {
            destroy(m_nSize);
            deallocate(m_nCapacity);
        }

        vector(_In_ const alloc_type& alloc) noexcept
            : m_allocator(alloc), m_nSize(0), m_nCapacity(0), m_data(nullptr)
        {}

        // count + repeated-args constructor
        template<typename... Args,
            typename = std::enable_if_t<std::is_constructible_v<T, Args&&...>>>
        explicit vector(_In_ size_type size, _In_ Args&&... args)
            : m_allocator(), m_nSize(0), m_nCapacity(0), m_data(nullptr)
        {
            if (!size) return;
            allocate(size);
            construct(size, std::forward<Args>(args)...);
        }

        // generic range constructor (disabled for integrals)
        template<typename InputIt,
            typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
        vector(_In_ InputIt first, _In_ InputIt last)
            : m_allocator(), m_nSize(0), m_nCapacity(0), m_data(nullptr)
        {
            const auto n = static_cast<size_type>(std::distance(first, last));
            if (!n) return;
            allocate(n);
            construct(first, last);
        }

        // initializer_list constructor
        vector(_In_ std::initializer_list<value_type> init)
            : m_allocator(), m_nSize(0), m_nCapacity(0), m_data(nullptr)
        {
            const auto n = static_cast<size_type>(init.size());
            if (!n) return;
            allocate(n);
            construct(init.begin(), init.end());
        }

        // copy ctor
        vector(const vector& other)
            : m_allocator(alloc_traits::select_on_container_copy_construction(other.m_allocator))
            , m_nSize(0), m_nCapacity(0), m_data(nullptr)
        {
            if (!other.m_nSize) return;
            allocate(other.m_nSize);
            for (size_type i = 0; i < other.m_nSize; ++i)
                alloc_traits::construct(m_allocator, m_data + i, other.m_data[i]);
            m_nSize = other.m_nSize;
        }

        // move ctor
        vector(vector&& other) noexcept
            : m_allocator(std::move(other.m_allocator)),
            m_nSize(other.m_nSize),
            m_nCapacity(other.m_nCapacity),
            m_data(other.m_data)
        {
            other.m_data = nullptr;
            other.m_nSize = 0;
            other.m_nCapacity = 0;
        }

        // copy assign
        vector& operator=(const vector& other)
        {
            if (this == &other) return *this;

            if (other.m_nSize > m_nCapacity) {
                destroy(m_nSize);
                deallocate(m_nCapacity);
                allocate(other.m_nSize);
            }
            else {
                destroy(m_nSize);
            }

            for (size_type i = 0; i < other.m_nSize; ++i)
                alloc_traits::construct(m_allocator, m_data + i, other.m_data[i]);

            m_nSize = other.m_nSize;
            return *this;
        }

        // move assign
        vector& operator=(vector&& other) noexcept
        {
            if (this == &other) return *this;

            destroy(m_nSize);
            deallocate(m_nCapacity);

            m_allocator = std::move(other.m_allocator);
            m_data = other.m_data;
            m_nSize = other.m_nSize;
            m_nCapacity = other.m_nCapacity;

            other.m_data = nullptr;
            other.m_nSize = 0;
            other.m_nCapacity = 0;
            return *this;
        }

        //~ overload operators
        friend bool operator==(const vector& a, const vector& b) noexcept
        {
            if (a.m_nSize != b.m_nSize) return false;
            for (size_type i = 0; i < a.m_nSize; ++i)
                if (!(a.m_data[i] == b.m_data[i])) return false;
            return true;
        }

        friend bool operator!=(const vector& a, const vector& b) noexcept
        {
            return !(a == b);
        }

        friend bool operator<(const vector& a, const vector& b)
        {
            const size_type n = (a.m_nSize < b.m_nSize) ? a.m_nSize : b.m_nSize;
            for (size_type i = 0; i < n; ++i) {
                if (a.m_data[i] < b.m_data[i])  return true;
                if (b.m_data[i] < a.m_data[i])  return false;
            }
            return a.m_nSize < b.m_nSize;
        }

        friend bool operator>(const vector& a, const vector& b) { return b < a; }
        friend bool operator<=(const vector& a, const vector& b) { return !(b < a); }
        friend bool operator>=(const vector& a, const vector& b) { return !(a < b); }

        friend auto operator<=>(const vector& a, const vector& b)
        {
            using elem_cmp = std::compare_three_way_result_t<const value_type&, const value_type&>;

            const size_type n = (a.m_nSize < b.m_nSize) ? a.m_nSize : b.m_nSize;

            for (size_type i = 0; i < n; ++i)
            {
                auto r = (a.m_data[i] <=> b.m_data[i]);
                if (r != 0) return r;
            }

            if (a.m_nSize < b.m_nSize) return std::strong_ordering::less;
            if (a.m_nSize > b.m_nSize) return std::strong_ordering::greater;
            return std::strong_ordering::equal;
        }

        void swap(vector& other) noexcept(
            alloc_traits::propagate_on_container_swap::value ||
            alloc_traits::is_always_equal::value)
        {
            if constexpr (alloc_traits::propagate_on_container_swap::value) 
            {
                using std::swap;
                swap(m_allocator, other.m_allocator);
                swap(m_data, other.m_data);
                swap(m_nSize, other.m_nSize);
                swap(m_nCapacity, other.m_nCapacity);
            }
            else
            {
                if (m_allocator == other.m_allocator)
                {
                    using std::swap;
                    swap(m_data, other.m_data);
                    swap(m_nSize, other.m_nSize);
                    swap(m_nCapacity, other.m_nCapacity);
                }
                else 
                {
                    using std::swap;
                    swap(m_data, other.m_data);
                    swap(m_nSize, other.m_nSize);
                    swap(m_nCapacity, other.m_nCapacity);
                }
            }
        }

        friend void swap(vector& a, vector& b) noexcept(noexcept(a.swap(b)))
        {
            a.swap(b);
        }

        //~ element access
        reference operator[](_In_ size_type index)
        {
            if (index >= m_nSize)
                throw std::out_of_range("fox::vector::operator[] index out of range");

            return m_data[index];
        }
        const_reference operator[](_In_ size_type index) const
        {
            if (index >= m_nSize)
                throw std::out_of_range("fox::vector::operator[] index out of range");

            return m_data[index];
        }

        reference       front() { assert(m_nSize); return m_data[0]; }
        const_reference front() const { assert(m_nSize); return m_data[0]; }

        reference       back() { assert(m_nSize); return m_data[m_nSize - 1]; }
        const_reference back()  const { assert(m_nSize); return m_data[m_nSize - 1]; }

        _Check_return_ _Ret_maybenull_ pointer data() noexcept { return m_data; }
        _Check_return_ _Ret_maybenull_ const_pointer data() const noexcept { return m_data; }

        //~ capacity queries
        bool   empty() const noexcept { return m_nSize == 0; }
        size_t size() const noexcept { return m_nSize; }
        size_t capacity() const noexcept { return m_nCapacity; }

        constexpr size_type max_size() const noexcept
        {
            return std::min<size_type>(
                alloc_traits::max_size(m_allocator),
                static_cast<size_type>(
                    std::numeric_limits<difference_type>::max()));
        }

        //~ modifiers
        void resize(_In_ const size_type& n)
        {
            destroy(m_nSize);
            deallocate(m_nCapacity);
            if (!n) return;
            allocate(n);
            construct(n);
        }

        void resize(_In_ const size_type& n, const value_type& val) 
        {
            reserve(n);

            for (size_t i = 0; i < n; i++)
            {
                alloc_traits::construct(m_allocator, m_data + i, val);
            }
        }

        void reserve(_In_ const size_type& n)
        {
            if (n <= m_nCapacity) return;
            destroy(m_nSize);
            deallocate(m_nCapacity);
            allocate(n);
            m_nSize = 0;
        }

        //~ assign count
        template<typename... Args,
            typename = std::enable_if_t<std::is_constructible_v<T, Args&&...>>>
        void assign(_In_ const size_type& count, _In_ Args&&... args)
        {
            destroy(m_nSize);
            if (count > m_nCapacity) {
                deallocate(m_nCapacity);
                allocate(count);
            }
            for (size_type i = 0; i < count; ++i)
                alloc_traits::construct(m_allocator, m_data + i, std::forward<Args>(args)...);
            m_nSize = count;
        }

        //~ utilities
        void clear()
        {
            destroy(m_nSize);
            m_nSize = 0;
        }

        void push_back(_In_ const value_type& val)
        {
            if (m_nSize + 1 > m_nCapacity) reallocate();
            alloc_traits::construct(m_allocator,
                m_data + m_nSize,
                val);
            m_nSize++;
        }

        void push_back(_Inout_ value_type&& val)
        {
            if (m_nSize + 1 > m_nCapacity) reallocate();

            alloc_traits::construct(m_allocator,
                m_data + m_nSize,
                std::move(val));

            m_nSize++;
        }

        const_reference at(_In_ const size_type& index)
        {
            if (index >= m_nSize)
                throw std::out_of_range("fox::vector::at() index out of range");
            return m_data[index];
        }

        reference at(_In_ const size_type& index) const
        {
            if (index >= m_nSize)
                throw std::out_of_range("fox::vector::at() index out of range");
            return m_data[index];
        }

        void shrink_to_fit()
        {
            if (m_nSize == m_nCapacity) return;
            pointer tmp = alloc_traits::allocate(
                m_allocator,
                m_nSize);

            int counter = 0;
            for (auto iter = begin(); iter != end(); iter++)
            {
                alloc_traits::construct(
                    m_allocator,
                    tmp + counter,
                    std::move(*iter)
                );
                ++counter;
            }

            deallocate(m_nSize);
            m_data = tmp;
            m_nCapacity = m_nSize;
        }

        template<typename... Args>
        reference emplace_back(Args&&...args)
        {
            if (m_nSize >= m_nCapacity) reallocate();

            alloc_traits::construct(
                m_allocator,
                m_data + m_nSize,
                std::forward<Args>(args)...);

            ++m_nSize;
            return m_data[m_nSize - 1];
        }

        void pop_back()
        {
            if (not m_nSize) return;
            --m_nSize;
            alloc_traits::destroy(m_allocator, m_data + m_nSize);
        }

        //~ assign from range
        template<typename InputIt,
            typename = std::enable_if_t<!std::is_integral_v<InputIt>>>
        void assign(_In_ InputIt first, _In_ InputIt last)
        {
            const auto n = static_cast<size_type>(std::distance(first, last));
            destroy(m_nSize);
            if (n > m_nCapacity) {
                deallocate(m_nCapacity);
                allocate(n);
            }
            size_type i = 0;
            for (auto it = first; it != last; ++it, ++i)
                alloc_traits::construct(m_allocator, m_data + i, *it);
            m_nSize = i;
        }

        void assign(_In_ std::initializer_list<value_type> init)
        {
            assign(init.begin(), init.end());
        }

        //~ iterators
        class Iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type        = T;
            using reference         = T&;
            using pointer           = T*;

            Iterator() noexcept = default;
            explicit Iterator(pointer ptr) noexcept : m_pointer(ptr) {}

            reference operator*()  const noexcept { return *m_pointer; }
            pointer   operator->() const noexcept { return std::addressof(operator*()); }

            Iterator& operator++() noexcept { ++m_pointer; return *this; }
            Iterator& operator--() noexcept { --m_pointer; return *this; }
            Iterator  operator++(int) noexcept { Iterator t(*this); ++(*this); return t; }
            Iterator  operator--(int) noexcept { Iterator t(*this); --(*this); return t; }

            Iterator& operator+=(difference_type n) noexcept { m_pointer += n; return *this; }
            Iterator& operator-=(difference_type n) noexcept { m_pointer -= n; return *this; }

            Iterator  operator+(difference_type n) const noexcept { return Iterator(m_pointer + n); }
            Iterator  operator-(difference_type n) const noexcept { return Iterator(m_pointer - n); }

            difference_type operator-(const Iterator& other) const noexcept { return m_pointer - other.m_pointer; }

            bool operator==(const Iterator& other) const noexcept { return m_pointer == other.m_pointer; }
            bool operator!=(const Iterator& other) const noexcept { return m_pointer != other.m_pointer; }
            bool operator< (const Iterator& other) const noexcept { return m_pointer < other.m_pointer; }
            bool operator> (const Iterator& other) const noexcept { return m_pointer > other.m_pointer; }
            bool operator<=(const Iterator& other) const noexcept { return m_pointer <= other.m_pointer; }
            bool operator>=(const Iterator& other) const noexcept { return m_pointer >= other.m_pointer; }

        private:
            pointer m_pointer{ nullptr };
        };

        Iterator begin() noexcept { return Iterator(m_data); }
        Iterator end()   noexcept { return Iterator(m_data + m_nSize); }
        const Iterator begin() const noexcept { return Iterator(m_data); }
        const Iterator end()   const noexcept { return Iterator(m_data + m_nSize); }

        //~ reverse iterator
        class ReverseIterator
        {
        public:
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using reference = T&;
            using pointer = T*;

            ReverseIterator() noexcept = default;
            explicit ReverseIterator(pointer base) noexcept : m_base(base) {}

            pointer base() const noexcept { return m_base; }

            reference operator*() const noexcept { return *(m_base - 1); }
            pointer   operator->() const noexcept { return std::addressof(operator*()); }

            ReverseIterator& operator++()    noexcept { --m_base; return *this; }
            ReverseIterator  operator++(int) noexcept { ReverseIterator t(*this); --m_base; return t; }
            ReverseIterator& operator--()    noexcept { ++m_base; return *this; }
            ReverseIterator  operator--(int) noexcept { ReverseIterator t(*this); ++m_base; return t; }

            ReverseIterator& operator+=(difference_type n) noexcept { m_base -= n; return *this; }
            ReverseIterator& operator-=(difference_type n) noexcept { m_base += n; return *this; }
            ReverseIterator  operator+(difference_type n) const noexcept { return ReverseIterator(m_base - n); }
            ReverseIterator  operator-(difference_type n) const noexcept { return ReverseIterator(m_base + n); }
            difference_type  operator-(const ReverseIterator& rhs) const noexcept { return rhs.m_base - m_base; }
            reference operator[](difference_type n) const noexcept { return *(*this + n); }

            //~ comparisons
            bool operator==(const ReverseIterator& rhs) const noexcept { return m_base == rhs.m_base; }
            bool operator!=(const ReverseIterator& rhs) const noexcept { return m_base != rhs.m_base; }
            bool operator< (const ReverseIterator& rhs) const noexcept { return m_base > rhs.m_base; }
            bool operator> (const ReverseIterator& rhs) const noexcept { return m_base < rhs.m_base; }
            bool operator<=(const ReverseIterator& rhs) const noexcept { return m_base >= rhs.m_base; }
            bool operator>=(const ReverseIterator& rhs) const noexcept { return m_base <= rhs.m_base; }

        private:
            pointer m_base{ nullptr };
        };

        ReverseIterator rbegin() noexcept { return ReverseIterator(m_data + m_nSize); }
        ReverseIterator rend()   noexcept { return ReverseIterator(m_data); }
        const ReverseIterator rbegin() const noexcept { return ReverseIterator(m_data + m_nSize); }
        const ReverseIterator rend()   const noexcept { return ReverseIterator(m_data); }

        Iterator insert(Iterator iter, const value_type& value)
        {
            size_type index = static_cast<size_type>(iter - begin());

            if (m_nSize == m_nCapacity) reallocate();

            Iterator pos = begin() + index;

            if (index == m_nSize)
            {
                push_back(value);
                return pos;
            }

            alloc_traits::construct(m_allocator, m_data + m_nSize, std::move(m_data[m_nSize - 1]));

            for (size_type i = m_nSize - 1; i > index; --i) 
                m_data[i] = std::move(m_data[i - 1]);
            
            m_data[index] = value;

            ++m_nSize;
            return begin() + index;
        }

        Iterator insert(Iterator iter,value_type&& value)
        {
            size_type index = static_cast<size_type>(iter - begin());

            if (m_nSize == m_nCapacity) reallocate();

            Iterator pos = begin() + index;

            if (index == m_nSize)
            {
                push_back(std::move(value));
                return pos;
            }

            alloc_traits::construct(
                m_allocator,
                m_data + m_nSize,
                std::move(m_data[m_nSize - 1]));

            for (size_type i = m_nSize - 1; i > index; --i)
                m_data[i] = std::move(m_data[i - 1]);

            m_data[index] = std::move(value);

            ++m_nSize;
            return begin() + index;
        }

        Iterator insert(Iterator pos, size_type count, const value_type& value)
        {
            if (count == 0) return pos;

            const size_type index = static_cast<size_type>(pos - begin());

            if (m_nSize + count > m_nCapacity)
                reallocate(m_nSize + count);

            pos = begin() + index;

            if (index == m_nSize) 
            {
                for (size_type i = 0; i < count; ++i) push_back(value);
                return pos;
            }

            for (size_type i = m_nSize; i-- > index; )
            {
                alloc_traits::construct(m_allocator,
                    m_data + (i + count),
                    std::move(m_data[i]));

                alloc_traits::destroy(m_allocator, m_data + i);
            }

            for (size_type k = 0; k < count; ++k)
                alloc_traits::construct(
                    m_allocator,
                    m_data + (index + k), value);

            m_nSize += count;
            return begin() + static_cast<std::ptrdiff_t>(index);
        }

        template <class InputIt,
        std::enable_if_t<!std::is_integral_v<InputIt>, int> = 0>
        Iterator insert(Iterator pos, InputIt first, InputIt last)
        {
            if (first == last) return pos;

            using Cat = typename std::iterator_traits<InputIt>::iterator_category;
            const size_type index0 = static_cast<size_type>(pos - begin());

            if constexpr (std::is_base_of_v<std::forward_iterator_tag, Cat>)
            {
                const size_type count = static_cast<size_type>(std::distance(first, last));
                if (count == 0) return pos;

                if (m_nSize + count > m_nCapacity)
                    reallocate(m_nSize + count);

                pos = begin() + index0;

                if (index0 == m_nSize)
                {
                    for (auto it = first; it != last; ++it)
                    {
                        alloc_traits::construct(m_allocator, m_data + m_nSize, *it);
                        ++m_nSize;
                    }
                    return begin() + index0;
                }

                for (size_type i = m_nSize; i-- > index0; )
                {
                    alloc_traits::construct(m_allocator, m_data + (i + count), std::move(m_data[i]));
                    alloc_traits::destroy(m_allocator, m_data + i);
                }

                size_type k = 0;
                for (auto it = first; it != last; ++it, ++k)
                    alloc_traits::construct(m_allocator, m_data + (index0 + k), *it);

                m_nSize += count;
                return begin() + index0;
            }
            else
            {
                Iterator first_inserted = begin() + index0;
                size_type idx = index0;
                bool grabbed = false;

                for (; first != last; ++first, ++idx)
                {
                    Iterator ins = insert(begin() + idx, *first);
                    if (!grabbed) { first_inserted = ins; grabbed = true; }
                }

                return first_inserted;
            }
        }

        Iterator insert(Iterator pos, Iterator start, Iterator end)
        {
            const size_type src_first = static_cast<size_type>(start - begin());
            const size_type src_last = static_cast<size_type>(end - begin());
            const size_type count = (src_last > src_first) ? (src_last - src_first) : 0;
            if (count == 0) return pos;

            pointer tmp = alloc_traits::allocate(m_allocator, count);
            for (size_type i = 0; i < count; ++i)
                alloc_traits::construct(m_allocator,
                    tmp + i, m_data[src_first + i]);

            size_type index0 = static_cast<size_type>(pos - begin());
            if (m_nSize + count > m_nCapacity)
                reallocate(m_nSize + count);

            pos = begin() + index0;

            if (index0 == m_nSize)
            {
                for (size_type i = 0; i < count; ++i)
                {
                    alloc_traits::construct(m_allocator, m_data + m_nSize, tmp[i]);
                    ++m_nSize;
                }
            }
            else
            {
                for (size_type i = m_nSize; i-- > index0; )
                {
                    alloc_traits::construct(m_allocator, m_data + (i + count), std::move(m_data[i]));
                    alloc_traits::destroy(m_allocator, m_data + i);
                }

                for (size_type k = 0; k < count; ++k)
                    alloc_traits::construct(m_allocator, m_data + (index0 + k), tmp[k]);

                m_nSize += count;
            }

            for (size_type i = 0; i < count; ++i)
                alloc_traits::destroy(m_allocator, tmp + i);
            alloc_traits::deallocate(m_allocator, tmp, count);

            return begin() + index0;
        }

        Iterator insert(Iterator position, std::initializer_list<value_type> init)
        {
            return insert(position, std::begin(init), std::end(init));
        }

        template<class... Args>
        Iterator emplace(Iterator position, Args&&... args)
        {
            const size_type index = static_cast<size_type>(position - begin());

            if (m_nSize == m_nCapacity)
                reallocate();

            position = begin() + index;

            if (index == m_nSize)
            {
                alloc_traits::construct(m_allocator, m_data + m_nSize, std::forward<Args>(args)...);
                ++m_nSize;
                return begin() + index;
            }

            alloc_traits::construct(m_allocator, m_data + m_nSize, std::move(m_data[m_nSize - 1]));

            for (size_type i = m_nSize - 1; i > index; --i)
                m_data[i] = std::move(m_data[i - 1]);

            alloc_traits::destroy(m_allocator, m_data + index);
            alloc_traits::construct(m_allocator, m_data + index, std::forward<Args>(args)...);

            ++m_nSize;
            return begin() + index;
        }

        Iterator erase(Iterator at) 
        {
            if (at == end()) return end();

            size_type index = std::distance(begin(), at);

            for (size_type i = index; i < m_nSize - 1; ++i)
            {
                m_data[i] = std::move(m_data[i + 1]);
            }
            --m_nSize;
            alloc_traits::destroy(m_allocator, m_data + m_nSize);

            return begin() + index;
        }

        Iterator erase(Iterator first, Iterator last)
        {
            if (first == last) return first;

            const size_type idx_first = static_cast<size_type>(first - begin());
            const size_type idx_last  = static_cast<size_type>(last - begin());
            const size_type count     = idx_last - idx_first;

            for (size_type i = idx_first; i + count < m_nSize; ++i)
                m_data[i] = std::move(m_data[i + count]);

            for (size_type i = 0; i < count; ++i)
                alloc_traits::destroy(m_allocator, m_data + (m_nSize - 1 - i));

            m_nSize -= count;

            return begin() + idx_first;
        }

    private:
        //~ helpers
        void allocate(_In_ const size_type& n)
        {
            m_data = alloc_traits::allocate(m_allocator, n);
            m_nCapacity = n;
        }

        void deallocate(_In_ const size_type& n) noexcept
        {
            if (m_data)
            {
                alloc_traits::deallocate(m_allocator, m_data, n);
                m_data = nullptr;
            }
        }

        void destroy(_In_ const size_type& n) noexcept
        {
            for (size_type i = 0; i < n; ++i)
                alloc_traits::destroy(m_allocator, m_data + i);
        }

        void construct(_In_ const size_type& n)
        {
            for (size_type i = 0; i < n; ++i)
                alloc_traits::construct(m_allocator, m_data + i);
            m_nSize = n;
        }

        template<typename... Args>
        void construct(_In_ const size_type& n, _In_ Args&&... args)
        {
            for (size_type i = 0; i < n; ++i)
                alloc_traits::construct(m_allocator, m_data + i, std::forward<Args>(args)...);
            m_nSize = n;
        }

        template<typename InputIt>
        std::enable_if_t<!std::is_integral_v<InputIt>, void>
            construct(_In_ InputIt first, _In_ InputIt last)
        {
            size_type i = 0;
            for (auto it = first; it != last; ++it, ++i)
                alloc_traits::construct(m_allocator, m_data + i, *it);
            m_nSize = i;
        }

        //~ growth helper
        void reallocate(_In_ const size_type& newCap = 0)
        {
            const size_type desired = newCap ? newCap : ((m_nCapacity + 1) << 1);
            pointer tmp = alloc_traits::allocate(m_allocator, desired);

            for (size_type i = 0; i < m_nSize; ++i)
                alloc_traits::construct(m_allocator, tmp + i, std::move(m_data[i]));

            // destroy old objects then free the old buffer
            for (size_type i = 0; i < m_nSize; ++i)
                alloc_traits::destroy(m_allocator, m_data + i);

            const size_type oldCap = m_nCapacity;
            deallocate(oldCap);

            m_data = tmp;
            m_nCapacity = desired;
        }

    private:
        alloc_type m_allocator{};
        size_type  m_nSize{ 0u };
        size_type  m_nCapacity{ 0u };
        pointer    m_data{ nullptr };
    };

} // namespace fox
