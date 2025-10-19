#pragma once

#include "PixelFoxCoreAPI.h"

#include <cstddef>
#include <utility>
#include <type_traits>
#include <functional>
#include <new>
#include <initializer_list>
#include <cassert>
#include <sal.h>

namespace fox
{
	template<class T>
	class list
	{
		struct node
		{
			T	  value;
			node* next{ nullptr };

			template<class...Args>
			explicit node(_In_ Args&&... args)
				noexcept(std::is_nothrow_constructible_v<T, Args...>)
			: value(std::forward<Args>(args)...), next(nullptr)
			{}
		};

		node*		m_pHead{ nullptr };
		std::size_t m_nSize{	0	 };

		_NODISCARD node* tail_node() const noexcept
		{
			node* t = m_pHead;
			if (!t) return nullptr;
			while (t->next) { t = t->next; }
			return t;
		}

	public:
		list() noexcept  = default;

		_NODISCARD bool empty() const 
		{
			return m_nSize == 0 || m_pHead == 0;
		}

		_NODISCARD size_t size() const { return m_nSize; }
		
		//~ copy and move
		list(_In_ const list& other)
		{
			node* tail = nullptr;
			for (node* left = other.m_pHead; left != nullptr; left = left->next)
			{
				node* n = new node(left->value);
				if (!m_pHead) 
				{
					m_pHead = tail = n;
				}
				else 
				{
					tail->next = n;
					tail = n;
				}
				++m_nSize;
			}
		}

		list(_Inout_ list&& other) noexcept
			: m_pHead(other.m_pHead), m_nSize(other.m_nSize)
		{
			other.m_pHead = nullptr;
			other.m_nSize = 0;
		}

		_Ret_maybenull_ list& operator=(_In_ const list& other)
		{
			if (this == &other) return *this;
			clear();

			node* tail = nullptr;

			for (node* left = other.m_pHead; left != nullptr; left = left->next)
			{
				node* new_node = new node(left->value); 
				if (m_pHead == nullptr)
				{
					m_pHead = tail = new_node;
				}
				else
				{
					tail->next = new_node;
					tail = new_node;
				}
				++m_nSize;
			}

			return *this;
		}

		_Ret_maybenull_ list& operator=(_Inout_ list&& other) noexcept
		{
			if (this == &other) return *this;
			clear();
			m_pHead			= other.m_pHead;
			m_nSize			= other.m_nSize;
			other.m_pHead	= nullptr;
			other.m_nSize	= 0;
			return *this;
		}

		~list()
		{
			clear();
		}

		void clear() noexcept
		{
			node* left = m_pHead;
			while (left)
			{
				node* next = left->next;
				delete left;
				left = next;
			}
			m_pHead = nullptr;
			m_nSize = 0;
		}

		_Ret_notnull_ T& front() noexcept
		{
			assert(m_pHead && "list::front() called on empty list");
			return m_pHead->value;
		}

		_Ret_notnull_ const T& front() const noexcept
		{
			assert(m_pHead && "list::front() called on empty list");
			return m_pHead->value;
		}

		void push_front(_In_ const T& v) 
		{
			node* new_node = new node(v);
			new_node->next = m_pHead;
			m_pHead = new_node;
			m_nSize++; 
		}

		void push_front(_In_ T&& v)
		{
			node* new_node = new node(std::move(v));
			new_node->next = m_pHead;
			m_pHead = new_node;
			m_nSize++;
		}

		template<class...Args>
		_Ret_notnull_ T& emplace_front(_In_ Args&&...args)
		{
			node* new_node = new node(std::forward<Args>(args)...);
			new_node->next = m_pHead;
			m_pHead = new_node;

			m_nSize++;

			return new_node->value;
		}

		template<class Predicate>
		_Must_inspect_result_ _Success_(return != 0)
		bool remove_if(Predicate&& predicate) 
		{
			node** link = &m_pHead;
			while (*link)
			{
				node* left = *link;
				if (predicate(left->value))
				{
					*link = left->next;
					delete left;
					m_nSize--;
					return true;
				}
				link = &((*link)->next);
			}
			return false;
		}

		class iterator
		{
			friend class const_iterator;
			node* n = nullptr;
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type		= T;
			using difference_type	= std::ptrdiff_t;
			using pointer			= T*;
			using reference			= T&;

					 iterator()				   noexcept = default;
			explicit iterator(_In_opt_ node* x) noexcept : n(x) {}

			reference operator* () const noexcept { return n->value;						 }
			pointer   operator->() const noexcept { return &n->value;						 }
			iterator& operator++()		 noexcept { n = n->next; return *this;				 }
			iterator  operator++(int)	 noexcept { iterator t = *this; ++(*this); return t; }

			friend bool operator==(_In_ const iterator& a, _In_ const iterator& b) noexcept { return a.n == b.n; }
			friend bool operator!=(_In_ const iterator& a, _In_ const iterator& b) noexcept { return a.n != b.n; }
		};

		class const_iterator
		{
			const node* n = nullptr;
		public:
			using iterator_category		= std::forward_iterator_tag;
			using value_type			= const T;
			using difference_type		= std::ptrdiff_t;
			using pointer				= const T*;
			using reference				= const T&;

					 const_iterator()					     noexcept = default;
			explicit const_iterator(_In_opt_ const node* x)	 noexcept : n(x) {}
					 const_iterator(_In_ const iterator& it) noexcept : n(it.n) {}

			reference		operator* () const noexcept	{ return n->value;								 }
			pointer			operator->() const noexcept	{ return &n->value;								 }
			const_iterator& operator++()	   noexcept	{ n = n->next; return *this;					 }
			const_iterator  operator++(int)    noexcept { const_iterator t = *this; ++(*this); return t; }

			friend bool operator==(_In_ const const_iterator& a, _In_ const const_iterator& b) noexcept { return a.n == b.n; }
			friend bool operator!=(_In_ const const_iterator& a, _In_ const const_iterator& b) noexcept { return a.n != b.n; }
		};

		//~ front access
		_NODISCARD iterator		  begin ()		 noexcept { return iterator(m_pHead);		}
		_NODISCARD iterator		  end   ()		 noexcept { return iterator(nullptr);		}
		_NODISCARD const_iterator begin () const noexcept { return const_iterator(m_pHead); }
		_NODISCARD const_iterator end   () const noexcept { return const_iterator(nullptr); }
		_NODISCARD const_iterator cbegin() const noexcept { return const_iterator(m_pHead); }
		_NODISCARD const_iterator cend  () const noexcept { return const_iterator(nullptr); }

		class const_reverse_iterator;
		class reverse_iterator
		{
			friend class const_reverse_iterator;
			node* m_head{ nullptr };
			node* m_cur { nullptr };
		public:
			using iterator_category = std::forward_iterator_tag; // only supports ++
			using value_type		= T;
			using difference_type	= std::ptrdiff_t;
			using pointer			= T*;
			using reference			= T&;

			reverse_iterator() noexcept = default;
			reverse_iterator(_In_opt_ node* head, _In_opt_ node* cur) noexcept : m_head(head), m_cur(cur) {}

			reference operator*()  const noexcept { return m_cur->value; }
			pointer   operator->() const noexcept { return &m_cur->value; }

			reverse_iterator& operator++() noexcept
			{
				if (!m_cur) { return *this; }
				if (m_cur == m_head) { m_cur = nullptr; return *this; }
				
				node* p = m_head;
				while (p && p->next != m_cur) { p = p->next; }
				
				m_cur = p;
				
				return *this;
			}
			reverse_iterator operator++(int) noexcept { reverse_iterator t = *this; ++(*this); return t; }

			friend bool operator==(_In_ const reverse_iterator& a, _In_ const reverse_iterator& b) noexcept { return a.m_cur == b.m_cur; }
			friend bool operator!=(_In_ const reverse_iterator& a, _In_ const reverse_iterator& b) noexcept { return a.m_cur != b.m_cur; }
		};

		class const_reverse_iterator
		{
			const node* m_head{ nullptr };
			const node* m_cur { nullptr };
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type		= const T;
			using difference_type	= std::ptrdiff_t;
			using pointer			= const T*;
			using reference			= const T&;

			const_reverse_iterator() noexcept = default;
			const_reverse_iterator(_In_opt_ const node* head, _In_opt_ const node* cur) noexcept : m_head(head), m_cur(cur) {}
			const_reverse_iterator(_In_ const reverse_iterator& it) noexcept : m_head(it.m_head), m_cur(it.m_cur)
			{}

			reference operator* () const noexcept { return m_cur->value; }
			pointer   operator->() const noexcept { return &m_cur->value; }

			const_reverse_iterator& operator++() noexcept
			{
				if (!m_cur) { return *this; }
				if (m_cur == m_head) { m_cur = nullptr; return *this; }
				
				const node* p = m_head;
				while (p && p->next != m_cur) { p = p->next; }
				
				m_cur = p;
				
				return *this;
			}
			const_reverse_iterator operator++(int) noexcept { const_reverse_iterator t = *this; ++(*this); return t; }

			friend bool operator==(_In_ const const_reverse_iterator& a, _In_ const const_reverse_iterator& b) noexcept { return a.m_cur == b.m_cur; }
			friend bool operator!=(_In_ const const_reverse_iterator& a, _In_ const const_reverse_iterator& b) noexcept { return a.m_cur != b.m_cur; }
		};

		//~ Reverse access
		_NODISCARD reverse_iterator rbegin()			  noexcept { return reverse_iterator(m_pHead, tail_node());		  }
		_NODISCARD reverse_iterator rend()				  noexcept { return reverse_iterator(m_pHead, nullptr);			  }
		_NODISCARD const_reverse_iterator rbegin()  const noexcept { return const_reverse_iterator(m_pHead, tail_node()); }
		_NODISCARD const_reverse_iterator rend()    const noexcept { return const_reverse_iterator(m_pHead, nullptr);	  }
		_NODISCARD const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(m_pHead, tail_node()); }
		_NODISCARD const_reverse_iterator crend()   const noexcept { return const_reverse_iterator(m_pHead, nullptr);	  }
	};
}
