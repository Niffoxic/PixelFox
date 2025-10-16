#pragma once

#include <type_traits>
#include "core/pair.h"


namespace
{

    struct MoveOnly 
    {
        int id{ 0 };

        MoveOnly() = default;
        explicit MoveOnly(int v) noexcept : id(v) {}

        MoveOnly(const MoveOnly&) = delete;
        MoveOnly& operator=(const MoveOnly&) = delete;

        MoveOnly(MoveOnly&&) noexcept = default;
        MoveOnly& operator=(MoveOnly&&) noexcept = default;

        friend bool operator<(const MoveOnly& a, const MoveOnly& b) noexcept
        {
            return a.id < b.id;
        }
        friend bool operator==(const MoveOnly& a, const MoveOnly& b) noexcept 
        {
            return a.id == b.id;
        }
    };

} // namespace

TEST(FoxPair, DefaultConstruct)
{
    fox::pair<int, int> p;
    EXPECT_EQ(p.first, 0);
    EXPECT_EQ(p.second, 0);
    static_assert(std::is_nothrow_default_constructible_v<fox::pair<int, int>>);
}

TEST(FoxPair, ValueConstructionAndForwarding)
{
    int x = 42;
    fox::pair<int&, MoveOnly> p(x, MoveOnly{ 7 });
    EXPECT_EQ(p.first, 42);
    EXPECT_EQ(p.second.id, 7);

    p.first = 99;
    EXPECT_EQ(x, 99);
}

TEST(FoxPair, ConvertingCopyAndMoveConstruction) 
{
    fox::pair<int, int> a{ 10, 20 };

    fox::pair<long, double> b(a);
    EXPECT_EQ(b.first, 10);
    EXPECT_EQ(b.second, 20.0);

    fox::pair<MoveOnly, int> c(MoveOnly{ 77 }, 5);
    fox::pair<MoveOnly, long> d(fox::move(c));
    EXPECT_EQ(d.first.id, 77);
    EXPECT_EQ(d.second, 5);
}

TEST(FoxPair, CopyAndMoveAssignment) 
{
    fox::pair<int, int> a{ 1, 2 }, b{ 3, 4 };
    b = a;
    EXPECT_EQ(b.first, 1);
    EXPECT_EQ(b.second, 2);

    fox::pair<MoveOnly, int> m{ MoveOnly{9}, 8 };
    fox::pair<MoveOnly, int> n{ MoveOnly{1}, 2 };
    n = fox::move(m);
    EXPECT_EQ(n.first.id, 9);
    EXPECT_EQ(n.second, 8);
}

TEST(FoxPair, SwapMemberAndADL)
{
    fox::pair<int, int> a{ 1, 2 }, b{ 7, 8 };

    // member swap
    a.swap(b);
    EXPECT_EQ(a.first, 7);
    EXPECT_EQ(a.second, 8);
    EXPECT_EQ(b.first, 1);
    EXPECT_EQ(b.second, 2);

    // ADL swap
    swap(a, b);
    EXPECT_EQ(a.first, 1);
    EXPECT_EQ(a.second, 2);
    EXPECT_EQ(b.first, 7);
    EXPECT_EQ(b.second, 8);

    static_assert(noexcept(a.swap(b)), "swap should be noexcept when members are noexcept swappable");
}

TEST(FoxPair, ComparisonsLexicographic) 
{
    fox::pair<int, int> a{ 1,2 }, b{ 1,3 }, c{ 2,0 }, d{ 1,2 };

    EXPECT_TRUE(a == d);
    EXPECT_FALSE(a != d);

    EXPECT_TRUE (a <  b);   // second decides
    EXPECT_TRUE (b <  c);   // first decides
    EXPECT_TRUE (c >  b);
    EXPECT_FALSE(b >  c);
    EXPECT_TRUE (a <= d);
    EXPECT_FALSE(d <= a);
    EXPECT_TRUE (a >= d);
}

TEST(FoxPair, MakePairAndTypeDecay)
{
    int x = 5;
    fox::pair<int, double> p = fox::make_pair(x, 3.5f);
    EXPECT_EQ(p.first, 5);
    EXPECT_DOUBLE_EQ(p.second, 3.5);

    static_assert(std::is_same_v<decltype(p), fox::pair<int, double>>);
    static_assert(noexcept(fox::make_pair(1, 2.0)));
}

TEST(FoxPair, StructuredBindingsWork) 
{
    fox::pair<int, MoveOnly> p{ 11, MoveOnly{22} };
    auto [a, b] = fox::move(p);
    EXPECT_EQ(a, 11);
    EXPECT_EQ(b.id, 22);
}
