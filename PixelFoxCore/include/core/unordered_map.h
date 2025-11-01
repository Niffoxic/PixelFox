#pragma once

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#include "PixelFoxCoreAPI.h"

#include <cstddef>
#include <type_traits>
#include <functional>
#include <new>
#include <cassert>
#include <sal.h>

namespace fox
{
    template
    <
        class Key,
        class T,
        class Hasher = std::hash<Key>,
        class KeyEq = std::equal_to<Key>
    >
    class unordered_map
    {
        struct node
        {
            Key   key;
            T     value;
            node* next;

            node(_In_ const Key& k, _In_ const T& v)
                noexcept(std::is_nothrow_copy_constructible_v<Key>&& 
                         std::is_nothrow_copy_constructible_v<T>)
                : key(k), value(v), next(nullptr)
            {
            }

            node(_In_ Key&& k, _In_ T&& v)
                noexcept(std::is_nothrow_move_constructible_v<Key>&& 
                         std::is_nothrow_move_constructible_v<T>)
                : key(std::move(k)), value(std::move(v)), next(nullptr)
            {
            }
        };

        node**      m_buckets       = nullptr;
        std::size_t m_bucket_count  = 0;
        std::size_t m_size          = 0;
        float       m_max_load      = 0.75f;
        Hasher      m_hash{};
        KeyEq       m_eq{};

        static std::size_t next_pow2(_In_ std::size_t x) noexcept
        {
            if (x < 2) return 2;
            --x;
            x |= x >> 1;
            x |= x >> 2;
            x |= x >> 4;
            x |= x >> 8;
            x |= x >> 16;

            if _CONSTEXPR20 (sizeof(std::size_t) == 8)
            {
                x |= x >> 32;
            }

            return x + 1;
        }

        void alloc_buckets(_In_ std::size_t n)
        {
            m_bucket_count = next_pow2(n < 2 ? 2 : n);
            
            m_buckets = static_cast<node**>(
                ::operator new[](sizeof(node*)* m_bucket_count)
            );

            for (std::size_t i = 0; i < m_bucket_count; ++i)
            {
                m_buckets[i] = nullptr;
            }
        }

        void destroy_all() noexcept
        {
            if (!m_buckets) return;

            for (std::size_t i = 0; i < m_bucket_count; ++i)
            {
                node* c = m_buckets[i];
                while (c)
                {
                    node* n = c->next;
                    delete c;
                    c = n;
                }
            }

            ::operator delete[](m_buckets);
            m_buckets       = nullptr;
            m_bucket_count  = 0;
            m_size          = 0;
        }

        _Ret_range_(0, m_bucket_count - 1)
        std::size_t index_of(_In_ const Key& k) const noexcept
        {
            return static_cast<std::size_t>(m_hash(k)) & (m_bucket_count - 1);
        }

        void grow_if_needed(_In_ std::size_t add = 1)
        {
            const float denom = m_bucket_count ? static_cast<float>(m_bucket_count) : 1.0f;
            const float lf = static_cast<float>(m_size + add) / denom;
            if (lf > m_max_load)
            {
                rehash(m_bucket_count ? m_bucket_count * 2 : 8);
            }
        }

    public:
        using key_type      = Key;
        using mapped_type   = T;
        using hasher        = Hasher;
        using key_equal     = KeyEq;
        using size_type     = std::size_t;

        explicit unordered_map(_In_ std::size_t bucket_hint = 8,
            _In_ const Hasher& h = Hasher{},
            _In_ const KeyEq& eq = KeyEq{},
            _In_ float max_load  = 0.75f)
            : m_max_load(max_load), m_hash(h), m_eq(eq)
        {
            alloc_buckets(bucket_hint);
        }

        unordered_map(_Inout_ unordered_map&& other) noexcept
            : m_buckets     (other.m_buckets)
            , m_bucket_count(other.m_bucket_count)
            , m_size        (other.m_size)
            , m_max_load    (other.m_max_load)
            , m_hash        (std::move(other.m_hash))
            , m_eq(std::move(other.m_eq))
        {
            other.m_buckets      = nullptr;
            other.m_bucket_count = 0;
            other.m_size         = 0;
        }

        unordered_map(const unordered_map&)            = delete;
        unordered_map& operator=(const unordered_map&) = delete;

        _Ret_notnull_ unordered_map& operator=(_Inout_ unordered_map&& other) noexcept
        {
            if (this == &other) return *this;
            destroy_all();
            
            m_buckets       = other.m_buckets;
            m_bucket_count  = other.m_bucket_count;
            m_size          = other.m_size;
            m_max_load      = other.m_max_load;
            m_hash          = std::move(other.m_hash);
            m_eq            = std::move(other.m_eq);
            
            other.m_buckets      = nullptr;
            other.m_bucket_count = 0;
            other.m_size         = 0;
            
            return *this;
        }

        ~unordered_map()
        {
            destroy_all();
        }

        _Ret_z_ size_type size() const noexcept
        {
            return m_size;
        }

        _Must_inspect_result_ bool empty() const noexcept
        {
            return m_size == 0;
        }

        void clear() noexcept
        {
            if (!m_buckets) return;
            for (size_type i = 0; i < m_bucket_count; ++i)
            {
                node* c = m_buckets[i];
                while (c)
                {
                    node* n = c->next;
                    delete c;
                    c = n;
                }
                m_buckets[i] = nullptr;
            }
            m_size = 0;
        }

        void max_load_factor(_In_ float f) noexcept
        {
            m_max_load = f;
        }

        float max_load_factor() const noexcept
        {
            return m_max_load;
        }

        float load_factor() const noexcept
        {
            return m_bucket_count ? float(m_size) / float(m_bucket_count) : 0.0f;
        }

        _Ret_z_ size_type bucket_count() const noexcept
        {
            return m_bucket_count;
        }

        void reserve(_In_ size_type n)
        {
            const size_type need = next_pow2(
                static_cast<size_type>(
                    float(n)/m_max_load) + 1);

            if (need > m_bucket_count)
            {
                rehash(need);
            }
        }

        void rehash(_In_ size_type new_bucket_count)
        {
            if (new_bucket_count < 2)
            {
                new_bucket_count = 2;
            }
            new_bucket_count = next_pow2(new_bucket_count);

            node** old     = m_buckets;
            size_type oldc = m_bucket_count;

            alloc_buckets(new_bucket_count);
            m_size = 0;

            if (old)
            {
                for (size_type i = 0; i < oldc; ++i)
                {
                    node* c = old[i];
                    while (c)
                    {
                        node* nx            = c->next;
                        const size_type idx = index_of(c->key);
                        c->next             = m_buckets[idx];
                        m_buckets[idx]      = c;
                        ++m_size;
                        c                   = nx;
                    }
                }
                ::operator delete[](old);
            }
        }

        _Must_inspect_result_ bool insert_or_assign(
            _In_ const Key& key,
            _In_ const T& value)
        {
            grow_if_needed();
            const size_type idx = index_of(key);
            
            for (node* e = m_buckets[idx]; e; e = e->next)
            {
                if (m_eq(e->key, key))
                {
                    e->value = value;
                    return false;
                }
            }
            
            node* n         = new node(key, value);
            n->next         = m_buckets[idx];
            m_buckets[idx]  = n;
            ++m_size;
            return true;
        }

        bool insert_or_assign(_In_ Key&& key, _In_ T&& value)
        {
            grow_if_needed();
            const size_type idx = index_of(key);
            
            for (node* e = m_buckets[idx]; e; e = e->next)
            {
                if (m_eq(e->key, key))
                {
                    e->value = std::move(value);
                    return false;
                }
            }
            
            node* n         = new node(std::move(key), std::move(value));
            n->next         = m_buckets[idx];
            m_buckets[idx]  = n;
            ++m_size;
            return true;
        }

        template<class... Args>
        _Ret_notnull_ T& try_emplace_get(_In_ const Key& key, _In_ Args&&... args)
        {
            grow_if_needed();
            const size_type idx = index_of(key);
            
            for (node* e = m_buckets[idx]; e; e = e->next)
            {
                if (m_eq(e->key, key))
                {
                    return e->value;
                }
            }

            node* n = static_cast<node*>(::operator new(sizeof(node)));
            ::new (&n->key) Key(key);
            ::new (&n->value) T(std::forward<Args>(args)...);
            
            n->next = m_buckets[idx];
            m_buckets[idx] = n;
            ++m_size;
            return n->value;
        }

        template<class... Args>
        _Ret_notnull_ T& try_emplace_get(_In_ Key&& key, _In_ Args&&... args)
        {
            grow_if_needed();
            const size_type idx = index_of(key);
            
            for (node* e = m_buckets[idx]; e; e = e->next)
            {
                if (m_eq(e->key, key))
                {
                    return e->value;
                }
            }
            
            node* n = static_cast<node*>(::operator new(sizeof(node)));
            ::new (&n->key) Key(std::move(key));
            ::new (&n->value) T(std::forward<Args>(args)...);
            
            n->next = m_buckets[idx];
            m_buckets[idx] = n;
            ++m_size;
            
            return n->value;
        }

        bool erase(_In_ const Key& key) noexcept
        {
            const size_type idx = index_of(key);
            node** link = &m_buckets[idx];
            
            while (*link)
            {
                node* cur = *link;
                if (m_eq(cur->key, key))
                {
                    *link = cur->next;
                    delete cur;
                    --m_size;
                    return true;
                }
                link = &((*link)->next);
            }
            
            return false;
        }

        _Ret_maybenull_ T* find(_In_ const Key& key) noexcept
        {
            const size_type idx = index_of(key);
            
            for (node* e = m_buckets[idx]; e; e = e->next)
            {
                if (m_eq(e->key, key))
                {
                    return &e->value;
                }
            }
            
            return nullptr;
        }

        _Ret_maybenull_ const T* find(_In_ const Key& key) const noexcept
        {
            const size_type idx = index_of(key);
            
            for (const node* e = m_buckets[idx]; e; e = e->next)
            {
                if (m_eq(e->key, key))
                {
                    return &e->value;
                }
            }
            
            return nullptr;
        }

        _Must_inspect_result_ bool contains(_In_ const Key& key) const noexcept
        {
            return find(key) != nullptr;
        }

        _Ret_notnull_ T& operator[](_In_ const Key& key)
        {
            static_assert(std::is_default_constructible_v<T>, "T must be default-constructible for operator[]");
            
            if (T* v = find(key))
            {
                return *v;
            }
            
            return try_emplace_get(key);
        }

        _Ret_notnull_ T& at(_In_ const Key& key)
        {
            T* v = find(key);
            assert(v && "key not found");
            
            return *v;
        }

        _Ret_notnull_ const T& at(_In_ const Key& key) const
        {
            const T* v = find(key);
            assert(v && "key not found");
            
            return *v;
        }

        class const_iterator;

        class iterator
        {
            // let const_iterator peek inside (needed for converting)
            friend class unordered_map::const_iterator;
            using map_t = unordered_map;

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type        = void;
            using difference_type   = std::ptrdiff_t;

            struct ref
            {
                const Key& first;
                T&         second;
                const Key* operator->() const noexcept { return &first; }
            };

        private:
            map_t*    _owner_map { nullptr };
            size_type _next_index{ 0 };
            node*     _current   { nullptr };

            void skip_empty() noexcept
            {
                // hop buckets until we find a node
                while (!_current && _owner_map && _next_index < _owner_map->m_bucket_count)
                {
                    _current = _owner_map->m_buckets[_next_index++];
                }
            }

        public:
            iterator() = default;

            iterator(
                _In_ map_t* owner,
                _In_ size_type next_bucket_index,
                _In_opt_ node* first_node) noexcept
                :   _owner_map(owner),
                    _next_index(next_bucket_index),
                    _current(first_node)
            {
                if (!_current) skip_empty();
            }

            ref operator* () const noexcept { return { _current->key, _current->value }; }
            ref operator->() const noexcept { return { _current->key, _current->value }; }

            iterator& operator++() noexcept
            {
                if (_current) _current = _current->next;
                if (!_current) skip_empty();
                return *this;
            }

            iterator operator++(int) noexcept
            {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            friend bool operator==(const iterator& a, const iterator& b) noexcept
            {
                return a._owner_map  == b._owner_map
                    && a._next_index == b._next_index
                    && a._current    == b._current;
            }
            
            friend bool operator!=(const iterator& a, const iterator& b) noexcept
            {
                return !(a == b);
            }
        };

        class const_iterator
        {
            using map_t = unordered_map;

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type        = void;
            using difference_type   = std::ptrdiff_t;

            struct ref
            {
                const Key& first;
                const T&   second;
                const Key* operator->() const noexcept { return &first; }
            };

        private:
            const map_t* _owner_map { nullptr };
            size_type    _next_index{ 0 };
            const node*  _current   { nullptr };

            void skip_empty() noexcept
            {
                while (!_current && _owner_map &&
                    _next_index < _owner_map->m_bucket_count)
                {
                    _current = _owner_map->m_buckets[_next_index++];
                }
            }

        public:
            const_iterator() = default;

            const_iterator(
                _In_ const map_t* owner,
                _In_ size_type next_bucket_index,
                _In_opt_ const node* first_node) noexcept
                :   _owner_map(owner),
                    _next_index(next_bucket_index),
                    _current(first_node)
            {
                if (!_current) skip_empty();
            }

            //~ convert non-const to const (we need friendship from iterator)
            const_iterator(_In_ const iterator& it) noexcept
                :   _owner_map(it._owner_map),
                    _next_index(it._next_index), 
                    _current(it._current)
            {}

            ref operator* () const noexcept { return { _current->key, _current->value }; }
            ref operator->() const noexcept { return { _current->key, _current->value }; }

            const_iterator& operator++() noexcept
            {
                if (_current) _current = _current->next;
                if (!_current) skip_empty();
                return *this;
            }

            const_iterator operator++(int) noexcept
            {
                const_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            friend bool operator==(
                const const_iterator& a,
                const const_iterator& b) noexcept
            {
                return a._owner_map  == b._owner_map
                    && a._next_index == b._next_index
                    && a._current    == b._current;
            }
            
            friend bool operator!=(
                const const_iterator& a,
                const const_iterator& b) noexcept
            {
                return !(a == b);
            }
        };

        //~ begin and end
        iterator begin() noexcept
        {
            if (!m_buckets || m_bucket_count == 0) return end();
            
            for (size_type i = 0; i < m_bucket_count; ++i)
            {
                if (m_buckets[i]) return iterator(this, i + 1, m_buckets[i]);
            }
            
            return end();
        }

        iterator end() noexcept
        {
            return iterator(this, m_bucket_count, nullptr);
        }

        const_iterator begin() const noexcept { return cbegin(); }
        const_iterator end  () const noexcept { return cend();   }

        const_iterator cbegin() const noexcept
        {
            if (!m_buckets || m_bucket_count == 0) return cend();
            
            for (size_type i = 0; i < m_bucket_count; ++i)
            {
                if (m_buckets[i]) 
                    return const_iterator(this, i + 1, m_buckets[i]);
            }
            
            return cend();
        }

        const_iterator cend() const noexcept
        {
            return const_iterator(this, m_bucket_count, nullptr);
        }
    };
}
