#pragma once
#include "pch.h"
#include <type_traits>
#include "core/utility.h"

namespace
{
    struct Probe
    {
        int  id{ 0 };
        bool moved_from{ false };

        Probe() = default;
        explicit Probe(int v) noexcept : id(v) {}

        Probe(const Probe&) = delete;
        Probe& operator=(const Probe&) = delete;

        Probe(Probe&& other) noexcept : id(other.id) { other.moved_from = true; }
        Probe& operator=(Probe&& other) noexcept
        {
            id = other.id;
            other.moved_from = true;
            return *this;
        }
    };

    int category(Probe&) { return 1; }
    int category(Probe&&) { return 2; }

} // namespace

TEST(FoxUtility, MoveCastsToRvalueRef)
{
    Probe p{ 42 };
    // type check
    static_assert(std::is_same_v<decltype(fox::move(p)), Probe&&>, "move must yield rvalue ref");
    
    // runtime move into another
    Probe q{ 7 };
    q = fox::move(p);
    EXPECT_EQ(q.id, 42);
    EXPECT_TRUE(p.moved_from);
}

TEST(FoxUtility, ForwardPreservesValueCategory)
{
    Probe x{ 1 };
    // lvalue forwarding
    int cat_l = category(fox::forward<Probe&>(x));
    EXPECT_EQ(cat_l, 1);
    
    // rvalue forwarding
    int cat_r = category(fox::forward<Probe>(Probe{ 2 }));
    EXPECT_EQ(cat_r, 2);

    // static checks
    static_assert(std::is_same_v<decltype(fox::forward<Probe&>(x)), Probe&>, "forward lvalue");
    static_assert(std::is_same_v<decltype(fox::forward<Probe>(Probe{ 3 })), Probe&& > , "forward rvalue");
}

TEST(FoxUtility, SwapThreeMove)
{
    Probe a{ 11 }, b{ 22 };
    EXPECT_TRUE((noexcept(fox::swap(a, b))));
    fox::swap(a, b);
    EXPECT_EQ(a.id, 22);
    EXPECT_EQ(b.id, 11);
    EXPECT_TRUE(a.moved_from || b.moved_from);
}

TEST(FoxUtility, ExchangeReplacesAndReturnsOld)
{
    int v = 5;
    int old = fox::exchange(v, 9);
    EXPECT_EQ(old, 5);
    EXPECT_EQ(v, 9);

    Probe p{ 123 };
    Probe prev = fox::exchange(p, Probe{ 456 });
    EXPECT_EQ(prev.id, 123);
    EXPECT_EQ(p.id, 456);
}

TEST(FoxUtility, NoexceptContracts)
{
    struct NT {
        NT() = default;
        NT(NT&&) noexcept = default;
        NT& operator=(NT&&) noexcept = default;
    };
    NT a, b;
    EXPECT_TRUE((noexcept(fox::swap(a, b))));
    EXPECT_TRUE((noexcept(fox::exchange(a, NT{}))));
}
