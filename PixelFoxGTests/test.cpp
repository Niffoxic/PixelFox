#include "pch.h"

#include <type_traits>
#include "core/initializer_list.h"
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


// Initializer List Tests
TEST(FoxInitializerList, DefaultIsEmptyAndNull)
{
    fox::initializer_list<int> il;
    EXPECT_EQ(il.size(), 0u);
    EXPECT_EQ(il.begin(), nullptr);
    EXPECT_EQ(il.data(), nullptr);
    EXPECT_EQ(il.end(), il.begin()); // both nullptr for empty
}

TEST(FoxInitializerList, PointerSizeConstructorBasics)
{
    int raw[]{ 1,2,3,4 };
    fox::initializer_list<int> il(raw, 4);

    EXPECT_EQ(il.size(), 4u);
    ASSERT_NE(il.begin(), nullptr);
    EXPECT_EQ(il.begin(), il.data());
    EXPECT_EQ(*(il.begin()), 1);
    EXPECT_EQ(*(il.begin() + 3), 4);
    EXPECT_EQ(il.end(), il.begin() + 4);
}

TEST(FoxInitializerList, IterateAndSum) 
{
    int raw[]{ 10, 20, 30 };
    fox::initializer_list<int> il(raw, 3);

    int sum = 0;
    for (const int* p = il.begin(); p != il.end(); ++p) sum += *p;
    EXPECT_EQ(sum, 60);
}

TEST(FoxInitializerList, FreeBeginEndHelpers)
{
    int raw[]{ 7, 8 };
    fox::initializer_list<int> il(raw, 2);

    const int* b = fox::begin(il);
    const int* e = fox::end(il);

    ASSERT_NE(b, nullptr);
    EXPECT_EQ(*(b + 0), 7);
    EXPECT_EQ(*(b + 1), 8);
    EXPECT_EQ(e, b + 2);
}

TEST(FoxInitializerList, ConstCorrectness)
{
    const int raw[]{ 5, 6, 7 };
    fox::initializer_list<int> il(raw, 3);

    static_assert(std::is_same_v<decltype(il.begin()), const int*>, "begin() must be const int*");
    static_assert(std::is_same_v<decltype(il.data()), const int*>, "data() must be const int*");
}
