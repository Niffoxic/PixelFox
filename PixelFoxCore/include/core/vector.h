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
#include <algorithm>
#include <type_traits>
#include <limits>
#include <stdexcept>

namespace fox
{
    template<typename T>
    class vector
    {
    public:
        //~ type
        using value_type             = T;
        using size_type              = std::size_t;
        using reference              = T&;
        using const_reference        = const T&;
        using pointer                = T*;
        using const_pointer          = const T*;
        using iterator               = T*; 
        using const_iterator         = const T*;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        //~ constructors
        vector() noexcept
            : m_data(nullptr), m_size(0), m_capacity(0)
        {}

        explicit vector(_In_ size_type count)
            : vector()
        {
            resize(count);
        }

        vector(_In_ size_type count, _In_ const T& value)
            : vector()
        {
            resize(count, value);
        }

        vector(_In_ std::initializer_list<T> init)
            : vector()
        {
            reserve(init.size());
            for (const auto& v : init) push_back(v);
        }

        //~ range constructor (ignoring if its integrals)
        template<class InputIt,
            class = std::enable_if_t<!std::is_integral_v<InputIt>>>
        vector(_In_ InputIt left, _In_ InputIt right)
            : vector()
        {
            for (; left != right; ++left) push_back(*left);
        }

        //~ destructor
        ~vector()
        {
            destroy_elements();
            ::operator delete(static_cast<void*>(m_data));
        }

        //~ copy and move operators
        vector(_In_ const vector& other)
            : vector()
        {
            if (other.m_size)
            {
                reserve(other.m_size);
                // copy construct elements
                for (size_type i = 0; i < other.m_size; ++i)
                {
                    new (static_cast<void*>(m_data + i)) T(other.m_data[i]);
                }
                m_size = other.m_size;
            }
        }

        vector(_Inout_ vector&& other) noexcept
            : m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity)
        {
            other.m_data     = nullptr;
            other.m_size     = 0;
            other.m_capacity = 0;
        }

        //~ assignments
        vector& operator=(_In_ const vector& other)
        {
            if (this == &other) return *this;

            if (other.m_size <= m_capacity)
            {
                size_type i = 0;
                // assign and copy into existing elements
                for (; i < std::min(m_size, other.m_size); ++i)
                {
                    m_data[i] = other.m_data[i];
                }

                // construct new if other is larger
                for (; i < other.m_size; ++i)
                {
                    ::new (static_cast<void*>(m_data + i)) T(other.m_data[i]);
                }

                // destroy extras if this was larger
                for (; i < m_size; ++i)
                {
                    m_data[i].~T();
                }

                m_size = other.m_size;
            }
            else
            {
                vector tmp(other);
                swap(tmp);
            }
            return *this;
        }

        vector& operator=(_Inout_ vector&& other) noexcept
        {
            if (this == &other) return *this;

            destroy_elements();
            ::operator delete(static_cast<void*>(m_data));

            m_data      = other.m_data;
            m_size      = other.m_size;
            m_capacity  = other.m_capacity;

            other.m_data     = nullptr;
            other.m_size     = 0;
            other.m_capacity = 0;

            return *this;
        }

        vector& operator=(_In_ std::initializer_list<T> ilist)
        {
            assign(ilist.begin(), ilist.end());
            return *this;
        }

        // comparisons
        bool operator==(_In_ const vector& other) const
        {
            if (m_size != other.m_size) return false;
            for (size_type i = 0; i < m_size; ++i)
                if (!(m_data[i] == other.m_data[i])) return false;
            return true;
        }

        bool operator!=(_In_ const vector& other) const { return !(*this == other); }

        bool operator< (_In_ const vector& other) const
        {
            return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
        }

        bool operator<=(_In_ const vector& other) const { return !(other < *this); }
        bool operator> (_In_ const vector& other) const { return (other < *this);  }
        bool operator>=(_In_ const vector& other) const { return !(*this < other); }

        // element access
        reference       operator[](_In_ size_type index)       noexcept { return m_data[index]; }
        const_reference operator[](_In_ size_type index) const noexcept { return m_data[index]; }

        reference       at(_In_ size_type index)
        {
            if (index >= m_size) throw std::out_of_range("fox::vector::at");
            return m_data[index];
        }
        const_reference at(_In_ size_type index) const
        {
            if (index >= m_size) throw std::out_of_range("fox::vector::at");
            return m_data[index];
        }

        reference       front()         { return m_data[0]; }
        const_reference front() const   { return m_data[0]; }

        reference       back()       { return m_data[m_size - 1]; }
        const_reference back() const { return m_data[m_size - 1]; }

        _Ret_maybenull_ pointer         data()       noexcept { return m_data; }
        _Ret_maybenull_ const_pointer   data() const noexcept { return m_data; }

        // iterators (may be nullptr when empty)
        _Ret_maybenull_ iterator       begin ()       noexcept { return m_data; }
        _Ret_maybenull_ const_iterator begin () const noexcept { return m_data; }
        _Ret_maybenull_ const_iterator cbegin() const noexcept { return m_data; }

        _Ret_maybenull_ iterator       end ()       noexcept { return m_data + m_size; }
        _Ret_maybenull_ const_iterator end () const noexcept { return m_data + m_size; }
        _Ret_maybenull_ const_iterator cend() const noexcept { return m_data + m_size; }

        reverse_iterator       rbegin ()       noexcept { return reverse_iterator(end());        }
        const_reverse_iterator rbegin () const noexcept { return const_reverse_iterator(end());  }
        const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

        reverse_iterator       rend ()       noexcept { return reverse_iterator(begin());        }
        const_reverse_iterator rend () const noexcept { return const_reverse_iterator(begin());  }
        const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

        // capacity
        bool      empty   () const noexcept { return m_size == 0;   }
        size_type size    () const noexcept { return m_size;        }
        size_type capacity() const noexcept { return m_capacity;    }
        size_type max_size() const noexcept
        {
            return std::numeric_limits<size_type>::max() / (sizeof(T) ? sizeof(T) : 1);
        }

        void reserve(_In_ size_type new_cap)
        {
            if (new_cap <= m_capacity) return;
            reallocate(new_cap);
        }

        void shrink_to_fit()
        {
            if (m_size < m_capacity)
            {
                reallocate(m_size);
            }
        }

        void resize(_In_ size_type new_size)
        {
            if (new_size < m_size)
            {
                // destroys trailing
                for (size_type i = new_size; i < m_size; ++i)
                {
                    m_data[i].~T();
                }
                m_size = new_size;
            }
            else if (new_size > m_size)
            {
                if (new_size > m_capacity)
                {
                    size_type new_cap = std::max(new_size, m_capacity ? m_capacity * 2 : size_type(1));
                    reallocate(new_cap);
                }
                // default and construct appended
                for (size_type i = m_size; i < new_size; ++i)
                {
                    ::new (static_cast<void*>(m_data + i)) T();
                }
                m_size = new_size;
            }
        }

        void resize(_In_ size_type new_size, _In_ const T& value)
        {
            if (new_size < m_size)
            {
                for (size_type i = new_size; i < m_size; ++i) m_data[i].~T();
                m_size = new_size;
            }
            else if (new_size > m_size)
            {
                if (new_size > m_capacity)
                {
                    size_type new_cap = std::max(new_size, m_capacity ? m_capacity * 2 : size_type(1));
                    reallocate(new_cap);
                }
                for (size_type i = m_size; i < new_size; ++i)
                {
                    ::new (static_cast<void*>(m_data + i)) T(value);
                }
                m_size = new_size;
            }
        }

        // modifiers
        void clear() noexcept
        {
            destroy_elements();
            m_size = 0;
        }

        void push_back(_In_ const T& value)
        {
            if (m_size == m_capacity) grow();
            ::new (static_cast<void*>(m_data + m_size)) T(value);
            ++m_size;
        }

        void push_back(_Inout_ T&& value)
        {
            if (m_size == m_capacity) grow();
            ::new (static_cast<void*>(m_data + m_size)) T(std::move(value));
            ++m_size;
        }

        template<class... Args>
        reference emplace_back(_In_ Args&&... args)
        {
            if (m_size == m_capacity) grow();
            ::new (static_cast<void*>(m_data + m_size)) T(std::forward<Args>(args)...);
            ++m_size;
            return back();
        }

        void pop_back()
        {
            m_data[m_size - 1].~T();
            --m_size;
        }

        // insert
        _Ret_maybenull_ iterator insert(_In_ const_iterator cpos, _In_ const T& value)
        {
            return insert_impl_single(cpos, value);
        }

        _Ret_maybenull_ iterator insert(_In_ const_iterator cpos, _Inout_ T&& value)
        {
            size_type idx = static_cast<size_type>(cpos - m_data);
            if (m_size == m_capacity) grow();
            
            iterator pos = m_data + idx;
            
            // move tail one step right
            for (size_type i = m_size; i > idx; --i)
            {
                ::new (static_cast<void*>(m_data + i)) T(std::move(m_data[i - 1]));
                m_data[i - 1].~T();
            }
            
            ::new (static_cast<void*>(pos)) T(std::move(value));
            ++m_size;
            
            return pos;
        }

        _Ret_maybenull_ iterator insert(_In_ const_iterator cpos, _In_ size_type count, _In_ const T& value)
        {
            if (count == 0) return const_cast<iterator>(cpos);
            
            size_type idx = static_cast<size_type>(cpos - m_data);
            
            if (m_size + count > m_capacity)
            {
                size_type new_cap = std::max(m_size + count, m_capacity ? m_capacity * 2 : size_type(1));
                reallocate(new_cap);
            }
            
            iterator pos = m_data + idx;
            
            // move tail
            for (size_type i = m_size; i > idx; --i)
            {
                ::new (static_cast<void*>(m_data + (i + count - 1))) T(std::move(m_data[i - 1]));
                m_data[i - 1].~T();
            }
            
            // fill
            for (size_type j = 0; j < count; ++j)
            {
                ::new (static_cast<void*>(pos + j)) T(value);
            }
            
            m_size += count;
            return pos;
        }

        template<class InputIt,
            class = std::enable_if_t<!std::is_integral_v<InputIt>>>
        _Ret_maybenull_ iterator insert(_In_ const_iterator cpos, _In_ InputIt left, _In_ InputIt right)
        {
            size_type idx = static_cast<size_type>(cpos - m_data);
            size_type added = 0;
            
            for (; left != right; ++left, ++added) 
            {
                insert(m_data + (idx + added), *left);
            }
            
            return m_data + idx;
        }

        _Ret_maybenull_ iterator insert(_In_ const_iterator cpos, _In_ std::initializer_list<T> ilist)
        {
            return insert(cpos, ilist.begin(), ilist.end());
        }

        template<class... Args>
        _Ret_maybenull_ iterator emplace(_In_ const_iterator cpos, _In_ Args&&... args)
        {
            size_type idx = static_cast<size_type>(cpos - m_data);
            if (m_size == m_capacity) grow();
            
            iterator pos = m_data + idx;
            
            // move tail right
            ::new (static_cast<void*>(m_data + m_size)) T(std::move(m_data[m_size - 1]));
            
            for (size_type i = m_size - 1; i > idx; --i)
            {
                m_data[i].~T();
                ::new (static_cast<void*>(m_data + i)) T(std::move(m_data[i - 1]));
            }
            if (idx < m_size) m_data[idx].~T();
            
            ::new (static_cast<void*>(pos)) T(std::forward<Args>(args)...);
            ++m_size;
            
            return pos;
        }

        //~ erase
        _Ret_maybenull_ iterator erase(_In_ const_iterator cpos)
        {
            size_type idx = static_cast<size_type>(cpos - m_data);
            m_data[idx].~T();
            
            for (size_type i = idx; i + 1 < m_size; ++i)
            {
                ::new (static_cast<void*>(m_data + i)) T(std::move(m_data[i + 1]));
                m_data[i + 1].~T();
            }
            
            --m_size;
            return m_data + idx;
        }

        _Ret_maybenull_ iterator erase(_In_ const_iterator left, _In_ const_iterator right)
        {
            if (left == right) return const_cast<iterator>(left);
            
            size_type idx_left = static_cast<size_type>(left - m_data);
            size_type idx_right = static_cast<size_type>(right - m_data);
            size_type count = idx_right - idx_left;

            // destroy range
            for (size_type i = idx_left; i < idx_right; ++i)
            {
                m_data[i].~T();
            }
            
            // move tail down
            for (size_type i = idx_right; i < m_size; ++i)
            {
                ::new (static_cast<void*>(m_data + (i - count))) T(std::move(m_data[i]));
                m_data[i].~T();
            }
            
            m_size -= count;
            return m_data + idx_left;
        }

        //~ assign
        void assign(_In_ size_type count, _In_ const T& value)
        {
            clear();
            if (count > m_capacity) reallocate(count);
            for (size_type i = 0; i < count; ++i)
            {
                ::new (static_cast<void*>(m_data + i)) T(value);
            }
            m_size = count;
        }

        void assign(_In_ std::initializer_list<T> ilist)
        {
            clear();
            reserve(ilist.size());
            for (const auto& v : ilist) push_back(v);
        }

        //~ assign range
        template<class InputIt,
            class = std::enable_if_t<!std::is_integral_v<InputIt>>>
        void assign(_In_ InputIt first, _In_ InputIt last)
        {
            clear();
            for (; first != last; ++first) push_back(*first);
        }

        //~ utilities
        void swap(_Inout_ vector& other) noexcept
        {
            std::swap(m_data, other.m_data);
            std::swap(m_size, other.m_size);
            std::swap(m_capacity, other.m_capacity);
        }

        void append(_In_ const vector& other)
        {
            if (other.m_size == 0) return;
            
            if (m_size + other.m_size > m_capacity)
            {
                size_type new_cap = std::max(m_size + other.m_size, m_capacity ? m_capacity * 2 : size_type(1));
                reallocate(new_cap);
            }
            
            for (size_type i = 0; i < other.m_size; ++i)
            {
                ::new (static_cast<void*>(m_data + (m_size + i))) T(other.m_data[i]);
            }
            m_size += other.m_size;
        }

    private:
        
        //~ Helpers
        void grow()
        {
            size_type new_cap = (m_capacity == 0) ? size_type(1) : m_capacity * 2;
            reallocate(new_cap);
        }

        void reallocate(_In_ size_type new_cap)
        {
            pointer new_mem = static_cast<pointer>(::operator new(new_cap * sizeof(T)));

            // move anbd construct existing into new storage
            size_type i = 0;
            try
            {
                for (; i < m_size; ++i)
                {
                    ::new (static_cast<void*>(new_mem + i)) T(std::move(m_data[i]));
                }
            }
            catch (...)
            {
                // roll back constructed
                for (size_type j = 0; j < i; ++j) new_mem[j].~T();
                ::operator delete(static_cast<void*>(new_mem));
                throw;
            }

            // destroy old
            for (size_type k = 0; k < m_size; ++k)
            {
                m_data[k].~T();
            }
            
            ::operator delete(static_cast<void*>(m_data));
            m_data = new_mem;
            m_capacity = new_cap;
        }

        void destroy_elements() noexcept
        {
            for (size_type i = 0; i < m_size; ++i)
            {
                m_data[i].~T();
            }
        }

        _Ret_maybenull_
        iterator insert_impl_single(_In_ const_iterator cpos, _In_ const T& value)
        {
            size_type idx = static_cast<size_type>(cpos - m_data);
            if (m_size == m_capacity) grow();
            iterator pos = m_data + idx;

            // move tail right by 1
            for (size_type i = m_size; i > idx; --i)
            {
                ::new (static_cast<void*>(m_data + i)) T(std::move(m_data[i - 1]));
                m_data[i - 1].~T();
            }
            
            ::new (static_cast<void*>(pos)) T(value);
            ++m_size;
            return pos;
        }

    private:
        T* m_data           { nullptr };
        size_type m_size    { 0 };
        size_type m_capacity{ 0 };
    };
} // namespace fox
