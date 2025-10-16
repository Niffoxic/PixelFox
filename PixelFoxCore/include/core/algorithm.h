/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

// TODO: Add Pointer specializations

#pragma once
#include <cstddef>
#include <type_traits>
#include <sal.h>
#include <yvals_core.h>
#include "utility.h"

namespace fox 
{

    //~ from [first,last)
    template<class InputIt, class OutputIt>
    _NODISCARD _CONSTEXPR20
    OutputIt copy(_In_ InputIt left, _In_ InputIt right, _Inout_ OutputIt d_left)
    {
        for (; left != right; ++left, (void)++d_left) 
        {
            *d_left = *left;
        }
        return d_left;
    }

    //~ from [first,last)
    template<class InputIt, class OutputIt>
    _NODISCARD _CONSTEXPR20
    OutputIt move(_In_ InputIt left, _In_ InputIt right, _Inout_ OutputIt d_left)
    {
        for (; left != right; ++left, (void)++d_left)
        {
            *d_left = fox::move(*left);
        }
        return d_left;
    }

    //~ copies in reverse so ranges can be overlaped safely
    template<class BidirIt1, class BidirIt2>
    _NODISCARD _CONSTEXPR20
    BidirIt2 move_backward(_In_ BidirIt1 left, _In_ BidirIt1 right, _Inout_ BidirIt2 d_left) 
    {
        while (left != right) 
        {
            *(--d_left) = fox::move(*(--right));
        }
        return d_left;
    }

    //~ writes value to n elements starting from left to right
    template<class OutputIt, class Size, class T>
    _NODISCARD _CONSTEXPR20
    OutputIt fill_n(_Inout_ OutputIt left, _In_ Size count, _In_ const T& value) 
    {
        for (; count > 0; --count, (void)++left) 
        {
            *left = value;
        }
        return left;
    }

    //~ compares [left,right) with [first, first+(left - right))
    template<class InputIt1, class InputIt2>
    _CONSTEXPR20
    bool equal(_In_ InputIt1 left, _In_ InputIt1 right, _In_ InputIt2 first) 
    {
        for (; left != right; ++left, (void)++first)
        {
            if (!(*left == *first)) return false;
        }
        return true;
    }

    //~ returns true if range1 is lexicographically less than range2
    template<class InputIt1, class InputIt2>
    _CONSTEXPR20
        bool lexicographical_compare(
            _In_ InputIt1 left_1, _In_ InputIt1 right_1,
            _In_ InputIt2 left_2, _In_ InputIt2 right_2
        )
    {
        for (; left_1 != right_1 && left_2 != right_2; ++left_1, (void)++left_2)
        {
            if (*left_1 < *left_2)  return true;
            if (*left_2 < *left_1)  return false;
        }
        return (left_1 == right_1) && (left_2 != right_2);
    }

} // namespace fox
