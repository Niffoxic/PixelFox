#pragma once
#include "pch.h"
#include <type_traits>
#include "core/initializer_list.h"


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
