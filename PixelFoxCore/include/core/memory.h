/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once

#include <new>
#include <type_traits>
#include <cstddef>
#include <sal.h>
#include "utility.h"

namespace fox 
{

	template<typename T>
	_CONSTEXPR20 T* addressof(_Inout_ T& obj) noexcept 
	{
		// to char ref to get built-in address
		return reinterpret_cast<T*>(
			&const_cast<unsigned char&>(reinterpret_cast<const unsigned char&>(obj)));
	}

	template<typename T>
	_CONSTEXPR20 void destroy_at(_Inout_ T* p) noexcept
	{
		if _CONSTEXPR20(!std::is_trivially_destructible_v<T>) 
		{
			p->~T();
		}
	}

	template<class T, class... Args>
	_CONSTEXPR20 _NODISCARD T* construct_at(_Inout_ T* p, _In_ Args&&... args) 
	{
		return ::new (static_cast<void*>(p)) T(fox::forward<Args>(args)...);
	}

	//~ Cleans the mem in a range from left to right.
	template<typename It>
	_CONSTEXPR20 void destroy(_In_ It left, _In_ It right) noexcept
	{
		using T = std::remove_reference_t<decltype(*left)>;
		if _CONSTEXPR20(!std::is_trivially_destructible_v<T>)
		{
			for (; left != right; left++)
			{
				fox::destroy_at(fox::addressof(*left));
			}
		}
	}

	//~ Uninit copy - conjstructs into uninitialized output range
	template<typename InputIt, typename NoThrowIt>
	NoThrowIt uninitialized_copy(
		_In_	InputIt left,
		_In_	InputIt right,
		_Inout_ NoThrowIt d_left)
	{
		NoThrowIt cur = d_left;

		try
		{
			for (; left != right; ++left, (void)++cur)
			{
				fox::construct_at(fox::addressof(*cur), *left);
			}
			return cur;
		}
		catch (...) 
		{
			for (; d_left != cur; ++d_left)
			{
				fox::destroy_at(fox::addressof(*d_left));
			}
			throw;
		}
	}

	template<typename InputIt, typename NoThrowIt>
	NoThrowIt uninitialized_move(
		_In_	InputIt left,
		_In_	InputIt right,
		_Inout_ NoThrowIt d_left)
	{
		NoThrowIt cur = d_left;
		try
		{
			for (; left != right; ++left, (void)++cur)
			{
				fox::construct_at(fox::addressof(*cur), fox::move(*left));
			}
			return cur;
		}
		catch (...)
		{
			for (; d_left != cur; ++d_left)
			{
				fox::destroy_at(fox::addressof(*d_left));
			}
			throw;
		}
	}

	//~ Allocator - just with basic new and delete
	// - allocate(0) returns nullptr, thats it
	template<typename T>
	struct allocator
	{
		using value_type = T;

		_CONSTEXPR20 allocator() noexcept = default;
		template<typename U>
		_CONSTEXPR20 allocator(const allocator<U>&) noexcept {}

		_NODISCARD T* allocate(_In_ std::size_t n)
		{
			if (n == 0) return nullptr;
			void* raw = ::operator new(n * sizeof(T));
			return static_cast<T*>(raw);
		}

		void deallocate(_Inout_opt_ T* p, _In_ std::size_t n) noexcept
		{
			(void)n;
			::operator delete(p);
		}

		//~ Helper traits
		using propagate_on_container_swap = std::false_type;
		using is_always_equal			  = std::true_type;
	};

	template<typename Alloc>
	struct allocator_traits
	{
		using allocator_type = Alloc;
		using value_type	 = typename Alloc::value_type;
		using pointer		 = value_type*;
		using const_pointer  = const value_type*;

	private:
		template<typename A, typename U, typename = void>
		struct rebind_impl { using type = fox::allocator<U>; };

		template<class A, class U>
		struct rebind_impl<A, U, std::void_t<typename A::template rebind<U>::other>> 
		{
			using type = typename A::template rebind<U>::other;
		};

	public:
		template<class U>
		using rebind_alloc = typename rebind_impl<Alloc, U>::type;

		// allocate/deallocate
		static pointer allocate(_Inout_ allocator_type& a, _In_ std::size_t n) 
		{
			return a.allocate(n);
		}

		static void deallocate(_Inout_ allocator_type& a, _Inout_opt_ pointer p, _In_ std::size_t n) noexcept 
		{
			a.deallocate(p, n);
		}

		// construct/destroy
		template<class T, class... Args>
		static void construct(_Inout_ allocator_type& a, _Inout_ T* p, _In_ Args&&... args)
		{
			fox::construct_at(p, fox::forward<Args>(args)...);
		}

		template<class T>
		static void destroy(_Inout_ allocator_type& a, _Inout_ T* p) noexcept 
		{
			fox::destroy_at(p);
		}

		//~ Helper traits
		using propagate_on_container_swap = typename Alloc::propagate_on_container_swap;
		using is_always_equal			  = typename Alloc::is_always_equal;
	};

} // namespace fox
