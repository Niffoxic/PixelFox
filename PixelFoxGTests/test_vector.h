/*
 * LLM Generated TestCode
 * reason: limited time I can't implement and write its google test within time constraints
*/

#pragma once

#include "pch.h"
#include "core/vector.h"

#include <array>

using fox::vector;

// ------------------------------------------------------------
// Helper type to track copy/move/ctor/dtor counts
// ------------------------------------------------------------
struct Tracked {
    static int live;
    static int ctor_default;
    static int ctor_value;
    static int ctor_copy;
    static int ctor_move;
    static int dtor;
    static void reset() {
        live = ctor_default = ctor_value = ctor_copy = ctor_move = dtor = 0;
    }

    int v{0};
    Tracked() { ++live; ++ctor_default; }
    explicit Tracked(int x) : v(x) { ++live; ++ctor_value; }
    Tracked(const Tracked& o) : v(o.v) { ++live; ++ctor_copy; }
    Tracked(Tracked&& o) noexcept : v(o.v) { ++live; ++ctor_move; }
    Tracked& operator=(const Tracked& o) { v = o.v; return *this; }
    Tracked& operator=(Tracked&& o) noexcept { v = o.v; return *this; }
    ~Tracked() { --live; ++dtor; }

    bool operator==(const Tracked& rhs) const { return v == rhs.v; }
    bool operator<(const Tracked& rhs) const { return v < rhs.v; }
};

int Tracked::live = 0;
int Tracked::ctor_default = 0;
int Tracked::ctor_value = 0;
int Tracked::ctor_copy = 0;
int Tracked::ctor_move = 0;
int Tracked::dtor = 0;

// ------------------------------------------------------------
// Basic construction
// ------------------------------------------------------------
TEST(FoxVector, DefaultConstructedIsEmpty) {
    vector<int> v;
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.begin(), v.end());
    EXPECT_EQ(v.rbegin(), v.rend());
}

TEST(FoxVector, CountConstructorDefaultValues) {
    vector<int> v(5);
    EXPECT_EQ(v.size(), 5u);
    for (auto x : v) EXPECT_EQ(x, 0);
}

TEST(FoxVector, CountValueConstructor) {
    vector<int> v(4, 42);
    ASSERT_EQ(v.size(), 4u);
    for (auto x : v) EXPECT_EQ(x, 42);
}

TEST(FoxVector, InitializerListConstructor) {
    vector<std::string> v({"a","bb","ccc"});
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "a");
    EXPECT_EQ(v[1], "bb");
    EXPECT_EQ(v[2], "ccc");
}

TEST(FoxVector, RangeConstructorFromStdArray) {
    std::array<int,4> arr{1,2,3,4};
    vector<int> v(arr.begin(), arr.end());
    ASSERT_EQ(v.size(), 4u);
    EXPECT_EQ(v[0],1); EXPECT_EQ(v[1],2); EXPECT_EQ(v[2],3); EXPECT_EQ(v[3],4);
}

// ------------------------------------------------------------
// Element access
// ------------------------------------------------------------
TEST(FoxVector, ElementAccessAndAtBounds) {
    vector<int> v({10,20,30});
    EXPECT_EQ(v.front(), 10);
    EXPECT_EQ(v.back(), 30);
    EXPECT_EQ(v[1], 20);
    EXPECT_NO_THROW(v.at(2));
    EXPECT_THROW(v.at(3), std::out_of_range);
    const auto& cv = v;
    EXPECT_EQ(cv.front(), 10);
    EXPECT_EQ(cv.back(), 30);
    EXPECT_EQ(cv[0], 10);
}

// ------------------------------------------------------------
// Iterators (const and reverse)
// ------------------------------------------------------------
TEST(FoxVector, IterationOrders) {
    vector<int> v({1,2,3,4});
    int sum = 0;
    for (auto it = v.begin(); it != v.end(); ++it) sum += *it;
    EXPECT_EQ(sum, 10);

    const auto& cv = v;
    int sum2 = 0;
    for (auto it = cv.cbegin(); it != cv.cend(); ++it) sum2 += *it;
    EXPECT_EQ(sum2, 10);

    std::vector<int> rev;
    for (auto it = v.rbegin(); it != v.rend(); ++it) rev.push_back(*it);
    ASSERT_EQ(rev.size(), 4u);
    EXPECT_EQ(rev[0], 4);
    EXPECT_EQ(rev[1], 3);
    EXPECT_EQ(rev[2], 2);
    EXPECT_EQ(rev[3], 1);
}

// ------------------------------------------------------------
// Capacity / reserve / resize / shrink_to_fit
// ------------------------------------------------------------
TEST(FoxVector, ReserveAndCapacity) {
    vector<int> v;
    v.reserve(100);
    EXPECT_GE(v.capacity(), 100u);
    EXPECT_EQ(v.size(), 0u);
}

TEST(FoxVector, ResizeDefaultAndWithValue) {
    vector<int> v;
    v.resize(3);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0],0); EXPECT_EQ(v[1],0); EXPECT_EQ(v[2],0);

    v.resize(5, 7);
    ASSERT_EQ(v.size(), 5u);
    EXPECT_EQ(v[3],7); EXPECT_EQ(v[4],7);

    v.resize(2);
    ASSERT_EQ(v.size(), 2u);
}

TEST(FoxVector, ShrinkToFitReducesCapacity) {
    vector<int> v(50, 1);
    auto cap_before = v.capacity();
    v.resize(5);
    v.shrink_to_fit();
    EXPECT_EQ(v.size(), 5u);
    EXPECT_LE(v.capacity(), cap_before);
    EXPECT_GE(v.capacity(), v.size());
}

// ------------------------------------------------------------
// Modifiers: push_back / emplace_back / pop_back
// ------------------------------------------------------------
TEST(FoxVector, PushBackCopyMoveEmplaceBack) {
    Tracked::reset();
    {
        vector<Tracked> v;
        v.push_back(Tracked(1));      // rvalue -> temporary destroyed after stmt
        v.push_back(Tracked(2));      // rvalue -> temporary destroyed after stmt
        {                             // <-- limit lifetime of t3
            Tracked t3(3);
            v.push_back(t3);          // lvalue copy; t3 destroyed at end of this inner scope
        }
        auto& ref = v.emplace_back(4); // in-place
        EXPECT_EQ(ref.v, 4);

        ASSERT_EQ(v.size(), 4u);
        EXPECT_EQ(v[0].v, 1);
        EXPECT_EQ(v[1].v, 2);
        EXPECT_EQ(v[2].v, 3);
        EXPECT_EQ(v[3].v, 4);
        EXPECT_EQ(Tracked::live, 4);  // now correct: only the 4 inside v are alive
    }
    EXPECT_EQ(Tracked::live, 0);      // all destroyed after v’s scope ends
}

TEST(FoxVector, PopBackReducesSizeAndDestroys) {
    Tracked::reset();
    {
        vector<Tracked> v;
        v.emplace_back(10);
        v.emplace_back(20);
        EXPECT_EQ(Tracked::live, 2);
        v.pop_back();
        EXPECT_EQ(v.size(), 1u);
        EXPECT_EQ(Tracked::live, 1);
    }
    EXPECT_EQ(Tracked::live, 0);
}

// ------------------------------------------------------------
// Insert / Erase
// ------------------------------------------------------------
TEST(FoxVector, InsertSingleAndCount) {
    vector<int> v({1,4});
    auto it = v.insert(v.begin() + 1, 2);   // [1,2,4]
    ASSERT_EQ(*it, 2);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0],1); EXPECT_EQ(v[1],2); EXPECT_EQ(v[2],4);

    v.insert(v.begin() + 2, 2, 3);          // [1,2,3,3,4]
    ASSERT_EQ(v.size(), 5u);
    EXPECT_EQ(v[0],1); EXPECT_EQ(v[1],2); EXPECT_EQ(v[2],3); EXPECT_EQ(v[3],3); EXPECT_EQ(v[4],4);
}

TEST(FoxVector, InsertRangeAndIList) {
    vector<int> v({1,5});
    int arr[] = {2,3,4};
    v.insert(v.begin() + 1, std::begin(arr), std::end(arr)); // [1,2,3,4,5]
    ASSERT_EQ(v.size(), 5u);
    for (int i=0;i<5;++i) EXPECT_EQ(v[i], i+1);

    v.insert(v.end(), {6,7}); // [1..7]
    ASSERT_EQ(v.size(), 7u);
    EXPECT_EQ(v.back(), 7);
}

TEST(FoxVector, EraseSingleAndRange) {
    vector<int> v({1,2,3,4,5});
    auto it = v.erase(v.begin() + 2);       // remove 3 -> [1,2,4,5]
    ASSERT_EQ(*it, 4);
    ASSERT_EQ(v.size(), 4u);
    EXPECT_EQ(v[2], 4);

    auto it2 = v.erase(v.begin() + 1, v.begin() + 3); // remove 2,4 -> [1,5]
    ASSERT_EQ(*it2, 5);
    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0],1); EXPECT_EQ(v[1],5);
}

// ------------------------------------------------------------
// Assign
// ------------------------------------------------------------
TEST(FoxVector, AssignCountRangeIList) {
    vector<int> v;
    v.assign(3, 9);
    ASSERT_EQ(v.size(), 3u);
    for (auto x : v) EXPECT_EQ(x, 9);

    int arr[] = {1,2,3,4};
    v.assign(std::begin(arr), std::end(arr));
    ASSERT_EQ(v.size(), 4u);
    for (int i=0;i<4;++i) EXPECT_EQ(v[i], i+1);

    v.assign({7,8});
    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0],7); EXPECT_EQ(v[1],8);
}

// ------------------------------------------------------------
// Copy / Move semantics
// ------------------------------------------------------------
TEST(FoxVector, CopyConstructorAndAssignment) {
    vector<std::string> a({"x","y","z"});
    vector<std::string> b(a);
    EXPECT_EQ(a, b);

    vector<std::string> c;
    c = a;
    EXPECT_EQ(a, c);
}

TEST(FoxVector, MoveConstructorAndAssignment) {
    vector<int> a({1,2,3});
    vector<int> b(std::move(a));
    EXPECT_EQ(b.size(), 3u);
    EXPECT_TRUE(a.empty());

    vector<int> c;
    c = std::move(b);
    EXPECT_EQ(c.size(), 3u);
    EXPECT_TRUE(b.empty());
}

// ------------------------------------------------------------
// Comparisons & swap & append
// ------------------------------------------------------------
TEST(FoxVector, ComparisonsAndSwapAppend) {
    vector<int> a({1,2,3});
    vector<int> b({1,2,3});
    vector<int> c({1,2,4});
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a != c);
    EXPECT_TRUE(a < c);
    EXPECT_TRUE(c > a);

    a.swap(c);
    EXPECT_EQ(a[2], 4);
    EXPECT_EQ(c[2], 3);

    c.append(vector<int>({5,6}));
    ASSERT_EQ(c.size(), 5u);
    EXPECT_EQ(c[3],5);
    EXPECT_EQ(c[4],6);
}

// ------------------------------------------------------------
// Data / max_size smoke
// ------------------------------------------------------------
TEST(FoxVector, DataAndMaxSize) {
    vector<int> v({10,20});
    ASSERT_NE(v.data(), nullptr);
    EXPECT_GE(v.max_size(), v.size());
}