#pragma once
#include "pch.h"

#include <type_traits>
#include <cstddef>

#include "core/utility.h"
#include "core/algorithm.h"


namespace
{

    struct MoveOnlyAlgoTest
    {
        int  v{ 0 };
        bool moved_from{ false };
        bool copied{ false };

        MoveOnlyAlgoTest() = default;
        explicit MoveOnlyAlgoTest(int x) noexcept : v(x) {}

        MoveOnlyAlgoTest(const MoveOnlyAlgoTest& rhs) noexcept : v(rhs.v)
        {
            const_cast<MoveOnlyAlgoTest&>(rhs).copied = true;
            copied = true;
        }
        MoveOnlyAlgoTest& operator=(const MoveOnlyAlgoTest& rhs) noexcept {
            v = rhs.v;
            const_cast<MoveOnlyAlgoTest&>(rhs).copied = true;
            copied = true;
            return *this;
        }

        MoveOnlyAlgoTest(MoveOnlyAlgoTest&& rhs) noexcept : v(rhs.v) { rhs.moved_from = true; }
        MoveOnlyAlgoTest& operator=(MoveOnlyAlgoTest&& rhs) noexcept {
            v = rhs.v;
            rhs.moved_from = true;
            return *this;
        }

        friend bool operator==(const MoveOnlyAlgoTest& a, const MoveOnlyAlgoTest& b) noexcept { return a.v == b.v; }
        friend bool operator<(const MoveOnlyAlgoTest& a, const MoveOnlyAlgoTest& b) noexcept { return a.v < b.v; }
    };

} // namespace

TEST(FoxAlgorithm, Copy_Trivial) {
    int src[5] = { 1,2,3,4,5 };
    int dst[5] = {};
    auto* ret = fox::copy(src, src + 5, dst);
    EXPECT_EQ(ret, dst + 5);
    EXPECT_TRUE(fox::equal(dst, dst + 5, src));
}

TEST(FoxAlgorithm, Copy_MoveOnlyUsesCopyAssign) {
    MoveOnlyAlgoTest src[3] = { MoveOnlyAlgoTest{1}, MoveOnlyAlgoTest{2}, MoveOnlyAlgoTest{3} };
    MoveOnlyAlgoTest dst[3] = {};

    auto* ret = fox::copy(src, src + 3, dst);
    EXPECT_EQ(ret, dst + 3);

    // Values match
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(dst[i].v, src[i].v);
    }
    // copy path should set copied flags
    EXPECT_TRUE(dst[0].copied || src[0].copied); // at least one side marks copy
}

// -------------------------------- move ---------------------------------

TEST(FoxAlgorithm, Move_Trivial) {
    int src[4] = { 10,20,30,40 };
    int dst[4] = {};
    auto* ret = fox::move(src, src + 4, dst);
    EXPECT_EQ(ret, dst + 4);
    EXPECT_TRUE(fox::equal(dst, dst + 4, src)); // ints are copied bitwise here
}

TEST(FoxAlgorithm, Move_MoveOnlyAlgoTest) {
    MoveOnlyAlgoTest src[3] = { MoveOnlyAlgoTest{5}, MoveOnlyAlgoTest{6}, MoveOnlyAlgoTest{7} };
    MoveOnlyAlgoTest dst[3] = { MoveOnlyAlgoTest{}, MoveOnlyAlgoTest{}, MoveOnlyAlgoTest{} };

    auto* ret = fox::move(src, src + 3, dst);
    EXPECT_EQ(ret, dst + 3);

    // dst receives values, src marked moved_from
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(dst[i].v, (i == 0 ? 5 : i == 1 ? 6 : 7));
        EXPECT_TRUE(src[i].moved_from);
        EXPECT_FALSE(dst[i].copied); // ensure no accidental copy happened
    }
}

// ---------------------------- move_backward ----------------------------

TEST(FoxAlgorithm, MoveBackward_OverlappingRange) {
    // classic overlap case: shift right by 2
    int buf[6] = { 1,2,3,4,5,6 };
    // move [0..4) into [2..6) using d_last = buf+6
    auto* begin_dest = fox::move_backward(buf + 0, buf + 4, buf + 6);
    EXPECT_EQ(begin_dest, buf + 2);
    int expected[6] = { 1,2,1,2,3,4 };
    for (int i = 0; i < 6; ++i) EXPECT_EQ(buf[i], expected[i]);
}

TEST(FoxAlgorithm, MoveBackward_MoveOnly) {
    MoveOnlyAlgoTest buf[5] = { MoveOnlyAlgoTest{1}, MoveOnlyAlgoTest{2}, MoveOnlyAlgoTest{3}, MoveOnlyAlgoTest{4}, MoveOnlyAlgoTest{5} };
    auto* begin_dest = fox::move_backward(buf + 0, buf + 3, buf + 5); // move 3 elements to end
    EXPECT_EQ(begin_dest, buf + 2);
    // End now holds 1,2,3
    EXPECT_EQ(buf[2].v, 1);
    EXPECT_EQ(buf[3].v, 2);
    EXPECT_EQ(buf[4].v, 3);
}

// ------------------------------- fill_n --------------------------------

TEST(FoxAlgorithm, FillN_Trivial) {
    int a[5] = {};
    auto* ret = fox::fill_n(a, 5, 9);
    EXPECT_EQ(ret, a + 5);
    for (int i = 0; i < 5; ++i) EXPECT_EQ(a[i], 9);
}

TEST(FoxAlgorithm, FillN_MovesValueForNonTrivialIsOkay) {
    MoveOnlyAlgoTest a[3] = { MoveOnlyAlgoTest{}, MoveOnlyAlgoTest{}, MoveOnlyAlgoTest{} };
    MoveOnlyAlgoTest val{ 42 };
    auto* ret = fox::fill_n(a, 3, val);
    EXPECT_EQ(ret, a + 3);
    for (int i = 0; i < 3; ++i) EXPECT_EQ(a[i].v, 42);
}

// -------------------------------- equal --------------------------------

TEST(FoxAlgorithm, Equal_Basic) {
    int x[4] = { 1,2,3,4 };
    int y[4] = { 1,2,3,4 };
    int z[4] = { 1,2,3,5 };
    EXPECT_TRUE(fox::equal(x, x + 4, y));
    EXPECT_FALSE(fox::equal(x, x + 4, z));
}

TEST(FoxAlgorithm, Equal_MovesafeTypes) {
    MoveOnlyAlgoTest a[2] = { MoveOnlyAlgoTest{7}, MoveOnlyAlgoTest{8} };
    MoveOnlyAlgoTest b[2] = { MoveOnlyAlgoTest{7}, MoveOnlyAlgoTest{8} };
    EXPECT_TRUE(fox::equal(a, a + 2, b));
}

// ----------------------- lexicographical_compare -----------------------

TEST(FoxAlgorithm, Lexicographic_EqualRangesFalse) {
    const char a[] = "abcd";
    const char b[] = "abcd";
    EXPECT_FALSE(fox::lexicographical_compare(a, a + 4, b, b + 4));
}

TEST(FoxAlgorithm, Lexicographic_PrefixIsLess) {
    const char a[] = "ab";
    const char b[] = "abc";
    EXPECT_TRUE(fox::lexicographical_compare(a, a + 2, b, b + 3));
    EXPECT_FALSE(fox::lexicographical_compare(b, b + 3, a, a + 2));
}

TEST(FoxAlgorithm, Lexicographic_ElementwiseCompare) {
    int a[] = { 1, 5, 9 };
    int b[] = { 1, 6, 0 };
    EXPECT_TRUE(fox::lexicographical_compare(a, a + 3, b, b + 3)); // 5 < 6 at index 1
    EXPECT_FALSE(fox::lexicographical_compare(b, b + 3, a, a + 3));
}
