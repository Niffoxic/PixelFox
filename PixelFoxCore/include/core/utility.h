/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once

#include <sal.h>
#include <type_traits>

namespace fox
{
	//~ move: cast to rvalue-ref of the same type
	template<typename T>
	_CONSTEXPR20 std::remove_reference_t<T>&& move(_Inout_ T&& t) noexcept
	{
		return static_cast<std::remove_reference_t<T>&&>(t);
	}

	//~ forward (lval overload): just to preserver lval
	// if T = U&, this collapses to U&
	template<class T>
	_CONSTEXPR20 T&& forward(_Inout_ std::remove_reference_t<T>& t) noexcept
	{
		return static_cast<T&&>(t);
	}

	// forward (rvalue overload): preserves rval
	template<class T>
	_CONSTEXPR20 T&& forward(_Inout_ std::remove_reference_t<T>&& t) noexcept
	{
		static_assert(!std::is_lvalue_reference_v<T>,
			"bad forward of lvalue as rvalue");
		return static_cast<T&&>(t);
	} 

	//~ swapper.
	template<typename T>
	_CONSTEXPR20 void swap(_Inout_ T& left, _Inout_ T& right) 
		noexcept
		(std::is_nothrow_move_constructible_v<T>&&
		 std::is_nothrow_move_assignable_v   <T>)
	{
		T temp = fox::move(left );
		left   = fox::move(right);
		right  = std::move(temp );
	}

	//~ exchange - replaces object with new val and also returns old val
	template<typename T, class U = T>
	_CONSTEXPR20 T exchange(_Inout_ T& obj, _In_ U&& new_val)
		noexcept
		(std::is_nothrow_move_constructible_v<T>&&
		 std::is_nothrow_assignable_v        <T&, U&&>)
	{
		T prev = fox::move(obj);
		obj = fox::forward<U>(new_val);
		return prev;
	}
}
