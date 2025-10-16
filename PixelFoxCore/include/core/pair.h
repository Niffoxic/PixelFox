/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once

#include <type_traits>
#include <sal.h>
#include "utility.h"

namespace fox
{
	template<typename T1, typename T2>
	struct pair
	{
		T1 first;
		T2 second;

		_CONSTEXPR20 pair()
			noexcept
			(std::is_nothrow_default_constructible_v<T1> &&
			 std::is_nothrow_default_constructible_v<T2>)
			: first(), second()
		{}

		//~ default copy and move
		_CONSTEXPR20 pair(const pair&) = default;
		_CONSTEXPR20 pair(pair&&)
			noexcept(std::is_nothrow_move_constructible_v<T1>&&
					 std::is_nothrow_move_constructible_v<T2>) = default;

		_CONSTEXPR20 pair& operator=(const pair&) = default;
		_CONSTEXPR20 pair& operator=(pair&&)
			noexcept(std::is_nothrow_move_assignable_v<T1> &&
					 std::is_nothrow_move_assignable_v<T2>) = default;

		//~ Value forwarding
		template<typename U1 = T1, typename U2 = T2,
			std::enable_if_t<std::is_constructible_v<T1, U1&&> &&
							 std::is_constructible_v<T2, U2&&>,
			int> = 0>
		_CONSTEXPR20 pair(_In_ U1&& left, _In_ U2&& right) // could be rvalue
			noexcept(std::is_nothrow_convertible_v<T1, U1&&> &&
					 std::is_nothrow_convertible_v<T2, U2&&>)
			: first(fox::forward<U1>(left)), second(fox::forward<U2>(right))
		{}

		//~ Converting copy from pair<left, right>
		template<typename U1, typename U2,
			std::enable_if_t<std::is_constructible_v<T1, const U1&> &&
							 std::is_constructible_v<T2, const U2&>,
			int> = 0>
		_CONSTEXPR20 pair(_In_ const pair<U1, U2>& other)
			noexcept(std::is_nothrow_constructible_v<T1, const U1&> &&
					 std::is_nothrow_constructible_v<T2, const U2&>)
			: first(other.first), second(other.second)
		{}

		//~ converting move from pair<u1, and u2>
		template<typename U1, typename U2,
			std::enable_if_t<std::is_constructible_v<T1, U1&&> &&
							 std::is_constructible_v<T2, U2&&>,
			int> = 0>
		_CONSTEXPR20 pair(_Inout_ pair<U1, U2>&& other)
			noexcept(std::is_nothrow_constructible_v<T1, U1&&> &&
					 std::is_nothrow_constructible_v<T2, U2&&>)
			: first(fox::forward<U1>(other.first)), second(fox::forward<U2>(other.second))
		{}

		//~ Converting copy from pair<u and u2>
		template<typename U1, typename U2,
			std::enable_if_t<std::is_assignable_v<T1, const U2&>&&
							 std::is_assignable_v<T2, const U2&>,
			int> = 0>
		_CONSTEXPR20 pair& operator=(_In_ const pair<U1, U2>& other)
			noexcept(std::is_nothrow_assignable_v	  <T1, const U1&> &&
					 std::is_nothrow_move_assignable_v<T2, const U2&>)
		{
			first = other.first;
			second = other.second;
			return *this;
		}

		//~ Copy assign from pair operator
		template<typename U1, typename U2,
			std::enable_if_t<std::is_assignable_v<T1&, const U1&> &&
							 std::is_assignable_v<T2&, const U2&>,
			int> = 0>
		_CONSTEXPR20 pair& operator=(_In_ const pair<U1, U2>& other)
			noexcept(std::is_nothrow_assignable_v<T1&, const U1&> &&
					 std::is_nothrow_assignable_v<T2&, const U2&>)
		{
			first = other.first;
			second = other.second;
			return *this;
		}

		//~ move assign from pair operator
		template<typename U1, typename U2,
			std::enable_if_t<std::is_assignable_v<T1&, U1&&> &&
							 std::is_assignable_v<T2&, U2&&>,
			int> = 0>
		_CONSTEXPR20 pair& operator=(_Inout_ pair<U1, U2>&& other)
			noexcept(std::is_nothrow_assignable_v<T1&, U1&&> &&
					 std::is_nothrow_assignable_v<T1&, U1&&>)
		{
			first = fox::forward<U1>(other.first);
			second = fox::forward<U2>(other.second);
			return *this;
		}


		//~ swaps
		_CONSTEXPR20 void swap(_Inout_ pair& other)
			noexcept(noexcept(fox::swap(first, other.first)) && 
					 noexcept(fox::swap(second, other.second)))
		{
			fox::swap(first, other.first);
			fox::swap(second, other.second);
		}
	};

	//~ ADL swap (TODO: I should write another overload for the swapper from core section)
	template<typename T1, typename T2>
	_CONSTEXPR20 void swap(_Inout_ pair<T1, T2>& left, _Inout_ pair<T1, T2>& right)
		noexcept(noexcept(left.swap(right)))
	{
		left.swap(right);
	}

	//~ Relational Operators
	template<typename T1, typename T2>
	_CONSTEXPR20 bool operator==(const pair<T1, T2>& left, const pair<T1, T2>& right)
	{
		return left.first == right.first && left.second == right.second;
	}

	template<typename T1, typename T2>
	_CONSTEXPR20 bool operator!=(const pair<T1, T2>& left, const pair<T1, T2>& right)
	{
		return !(left == right);
	}

	template<typename T1, typename T2>
	_CONSTEXPR20 bool operator<(const pair<T1, T2>& left, const pair<T1, T2>& right)
	{
		return (left.first < right.first) || (!(right.first < left.first) && (left.second < right.second));
	}

	template<typename T1, typename T2>
	_CONSTEXPR20 bool operator>(const pair<T1, T2>& left, const pair<T1, T2>& right)
	{
		return right < left;
	}

	template<typename T1, typename T2>
	_CONSTEXPR20 bool operator<=(const pair<T1, T2>& left, const pair<T1, T2>& right)
	{
		return !(right < left);
	}

	template<typename T1, typename T2>
	_CONSTEXPR20 bool operator>=(const pair<T1, T2>& left, const pair<T1, T2>& right)
	{
		return !(left < right);
	}

	//~ Make pair
	template<typename T1, typename T2>
	_CONSTEXPR20 pair<std::decay_t<T1>, std::decay_t<T2>> make_pair(_In_ T1&& left, _In_ T2&& right)
		noexcept(std::is_nothrow_constructible_v<std::decay_t<T1>, T1&&> &&
				 std::is_nothrow_constructible_v<std::decay_t<T2>, T2&&>)
	{
		using P = pair<std::decay_t<T1>, std::decay_t<T2>>;
		return P(fox::forward<T1>(left), fox::forward<T2>(right));
	}

} // namespace fox
