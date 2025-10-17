#pragma once

#include "pch.h"

#include <type_traits>
#include <cstddef>

#include "core/iterator.h"
#include "core/memory.h"

namespace 
{
    typedef struct _TEST_NODE 
    {
        int v;
        constexpr explicit _TEST_NODE(int x) : v(x) {}
    } Node;

} // namespace


TEST(FoxIterator, PointerTraitsAreContiguous)
{
    using T = int;

    // raw pointer
    static_assert(std::is_same_v<fox::iterator_traits<T*>::value_type, T>);
    static_assert(std::is_same_v<fox::iterator_traits<T*>::pointer, T*>);
    static_assert(std::is_same_v<fox::iterator_traits<T*>::reference, T&>);
    static_assert(std::is_same_v<fox::iterator_traits<T*>::difference_type, std::ptrdiff_t>);
    static_assert(std::is_base_of_v<fox::random_access_iterator_tag, 
        fox::iterator_traits<T*>::iterator_category>);
    static_assert(std::is_base_of_v<fox::contiguous_iterator_tag,
        fox::iterator_traits<T*>::iterator_category>);

    // const raw pointer
    static_assert(std::is_same_v<
        fox::iterator_traits<const T*>::value_type, T>);
    static_assert(std::is_same_v<
        fox::iterator_traits<const T*>::pointer, const T*>);
    static_assert(std::is_same_v<
        fox::iterator_traits<const T*>::reference, const T&>);
    static_assert(std::is_same_v<
        fox::iterator_traits<const T*>::difference_type, std::ptrdiff_t>);
    static_assert(std::is_base_of_v<
        fox::contiguous_iterator_tag,
        fox::iterator_traits<const T*>::iterator_category>);
}

//~ REVERESE ITER
TEST(FoxIterator, ReverseIteratorBasicTraversal)
{
    int arr[5] = { 1,2,3,4,5 };

    fox::reverse_iterator<int*> rbegin(arr + 5); // base right + 1
    fox::reverse_iterator<int*> rend(arr + 0); // base at most left

    // most right
    EXPECT_EQ(*rbegin, 5);
    EXPECT_NE(rbegin, rend);

    int acc = 0, count = 0;
    for (auto it = rbegin; it != rend; ++it) 
    {
        acc += *it;
        ++count;
    }
    EXPECT_EQ(count, 5);
    EXPECT_EQ(acc, 1 + 2 + 3 + 4 + 5);
}

TEST(FoxIterator, ReverseIteratorBaseAndIndexing) 
{
    int arr[4] = { 10,20,30,40 };
    fox::reverse_iterator<int*> it(arr + 4);

    EXPECT_EQ(it.base(), arr + 4);
    EXPECT_EQ(*it, 40);

    // indexing: it[n] == *(it + n)
    EXPECT_EQ(it[0], 40);
    EXPECT_EQ((it + 1)[0], 30);
    EXPECT_EQ(it[1], 30);
    EXPECT_EQ(*(it + 2), 20);
    EXPECT_EQ(*(it + 3), 10);

    auto it2 = it + 2;
    EXPECT_EQ(*it2, 20);
    it2 += 1;
    EXPECT_EQ(*it2, 10);
    it2 -= 1;
    EXPECT_EQ(*it2, 20);

    // differences
    fox::reverse_iterator<int*> it_end(arr + 0);
    EXPECT_EQ(it - it_end, 4); // distance in reverse space
}

TEST(FoxIterator, ReverseIteratorComparisonsInvertOrder) 
{
    int arr[3] = { 1,2,3 };
    fox::reverse_iterator<int*> a(arr + 3); // 3
    fox::reverse_iterator<int*> b(arr + 2); // 2

    EXPECT_TRUE(a < b);   // since 3 (a) comes before 2 (b) in reverse traversal
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(b >= a);

    // equality
    fox::reverse_iterator<int*> a2(arr + 3);
    EXPECT_TRUE(a == a2);
    EXPECT_FALSE(a != a2);
}

TEST(FoxIterator, ReverseIteratorArrowOperator) 
{
    Node nodes[3] = { Node{7}, Node{8}, Node{9} };
    fox::reverse_iterator<Node*> it(nodes + 3); // refers to nodes[2]
    EXPECT_EQ(it->v, 9);

    ++it; // now nodes[1]
    EXPECT_EQ(it->v, 8);
}
