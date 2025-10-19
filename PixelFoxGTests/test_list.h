/*
 * LLM Generated TestCode
 * reason: limited time I can't implement and write it google test within time constraints
*/
#pragma once
#include "pch.h"
#include "core/list.h"

using fox::list;

namespace {

    // A move-only type to test emplace/move paths.
    struct MoveOnlyListTest {
        int v{ 0 };
        MoveOnlyListTest() = default;
        explicit MoveOnlyListTest(int x) : v(x) {}
        MoveOnlyListTest(const MoveOnlyListTest&) = delete;
        MoveOnlyListTest& operator=(const MoveOnlyListTest&) = delete;
        MoveOnlyListTest(MoveOnlyListTest&& other) noexcept : v(other.v) { other.v = -7777; }
        MoveOnlyListTest& operator=(MoveOnlyListTest&& other) noexcept {
            if (this != &other) { v = other.v; other.v = -7777; }
            return *this;
        }
        bool operator==(int x) const noexcept { return v == x; }
    };

} // namespace

// Helper: remove a single value from list<int>.
static bool remove_value(list<int>& L, int x) {
    return L.remove_if([&](const int& v) { return v == x; });
}

// Helper: remove a single value from list<MoveOnly>.
static bool remove_value(list<MoveOnlyListTest>& L, int x) {
    return L.remove_if([&](const MoveOnlyListTest& mo) { return mo.v == x; });
}

// Helper: remove-all count with a predicate (consumes the matches).
template<class T, class Pred>
static int remove_all_count(list<T>& L, Pred p) {
    int c = 0;
    while (L.remove_if(p)) ++c;
    return c;
}

// -------------------- BASIC INSERT / REMOVE --------------------

TEST(FoxList, PushFrontAndRemoveIf_Int) {
    list<int> L;
    // push_front adds to head; order not observable without iterators,
    // but individual presence/removal is testable.
    L.push_front(1);
    L.push_front(2);
    L.push_front(3);

    EXPECT_TRUE(remove_value(L, 3));
    EXPECT_FALSE(remove_value(L, 3)); // already gone

    EXPECT_TRUE(remove_value(L, 2));
    EXPECT_TRUE(remove_value(L, 1));
    EXPECT_FALSE(remove_value(L, 1)); // now empty
}

TEST(FoxList, EmplaceFront_MoveOnly) {
    list<MoveOnlyListTest> L;
    auto& r1 = L.emplace_front(10);
    auto& r2 = L.emplace_front(20);
    // references remain valid here; check payloads
    EXPECT_EQ(r1.v, 10);
    EXPECT_EQ(r2.v, 20);

    EXPECT_TRUE(remove_value(L, 20));
    EXPECT_TRUE(remove_value(L, 10));
    EXPECT_FALSE(remove_value(L, 10));
}

// -------------------- COPY / MOVE SEMANTICS --------------------

TEST(FoxList, CopyConstructor_Independence) {
    list<int> a;
    a.push_front(1);
    a.push_front(2);
    a.push_front(3);

    list<int> b(a); // deep copy

    // Remove '2' from a; b should still have it
    EXPECT_TRUE(remove_value(a, 2));
    EXPECT_FALSE(remove_value(a, 2)); // already gone in a
    EXPECT_TRUE(remove_value(b, 2));  // must still exist in b
}

TEST(FoxList, MoveConstructor_SourceEmptied) {
    list<int> a;
    a.push_front(11);
    a.push_front(22);
    a.push_front(33);

    list<int> b(std::move(a));

    // Source 'a' should be empty now
    EXPECT_FALSE(remove_value(a, 11));
    EXPECT_FALSE(remove_value(a, 22));
    EXPECT_FALSE(remove_value(a, 33));

    // 'b' contains the elements
    EXPECT_TRUE(remove_value(b, 22));
    EXPECT_TRUE(remove_value(b, 11));
    EXPECT_TRUE(remove_value(b, 33));
    EXPECT_FALSE(remove_value(b, 33));
}

TEST(FoxList, CopyAssignment_Independence) {
    list<int> a;
    a.push_front(5);
    a.push_front(6);

    list<int> b;
    b.push_front(100);

    b = a; // deep copy overwrite

    // b now has 6 and 5; 100 is gone
    EXPECT_TRUE(remove_value(b, 6));
    EXPECT_TRUE(remove_value(b, 5));
    EXPECT_FALSE(remove_value(b, 100));

    // a remains intact
    EXPECT_TRUE(remove_value(a, 6));
    EXPECT_TRUE(remove_value(a, 5));
}

TEST(FoxList, MoveAssignment_SourceEmptied) {
    list<int> a;
    a.push_front(1);
    a.push_front(2);
    a.push_front(3);

    list<int> b;
    b.push_front(42);

    b = std::move(a);

    // a should be empty
    EXPECT_FALSE(remove_value(a, 1));
    EXPECT_FALSE(remove_value(a, 2));
    EXPECT_FALSE(remove_value(a, 3));

    // b should have 1,2,3, and not 42
    int removed = 0;
    removed += remove_value(b, 1) ? 1 : 0;
    removed += remove_value(b, 2) ? 1 : 0;
    removed += remove_value(b, 3) ? 1 : 0;
    EXPECT_EQ(removed, 3);
    EXPECT_FALSE(remove_value(b, 42));
}

// -------------------- CLEAR / BULK REMOVAL --------------------

TEST(FoxList, ClearEmptiesTheList) {
    list<int> L;
    for (int i = 0; i < 10; ++i) L.push_front(i);
    L.clear();

    // Nothing should be removable
    for (int i = 0; i < 10; ++i) {
        EXPECT_FALSE(remove_value(L, i));
    }
}

TEST(FoxList, RemoveAllCountViaPredicate) {
    list<int> L;
    // Insert 0..9
    for (int i = 0; i < 10; ++i) L.push_front(i);

    // Remove all even numbers
    auto even = [](int x) { return (x % 2) == 0; };
    int evens_removed = remove_all_count(L, even);
    EXPECT_EQ(evens_removed, 5);

    // Remove all odds
    auto odd = [](int x) { return (x % 2) != 0; };
    int odds_removed = remove_all_count(L, odd);
    EXPECT_EQ(odds_removed, 5);

    // Now empty
    EXPECT_FALSE(remove_value(L, 0));
    EXPECT_FALSE(remove_value(L, 9));
}

// -------------------- MOVE-ONLY STRESS --------------------

TEST(FoxList, MoveOnly_CopyFreePaths) {
    list<MoveOnlyListTest> L;
    for (int i = 1; i <= 8; ++i) {
        MoveOnlyListTest m(i);
        L.push_front(std::move(m)); // exercise move push_front
    }

    // Remove a few specific values
    EXPECT_TRUE(remove_value(L, 3));
    EXPECT_TRUE(remove_value(L, 6));
    EXPECT_FALSE(remove_value(L, 999)); // not present
}

// -------------------- OPTIONAL: ITERATION TESTS --------------------
// Enable these once you add:
// iterator begin(); iterator end();
// const_iterator begin() const; const_iterator end() const;
// const_iterator cbegin() const; const_iterator cend() const;

#if 0 // Set to 1 after you implement begin()/end()
TEST(FoxList, IteratorTraversal) {
    list<int> L;
    for (int i = 1; i <= 4; ++i) L.push_front(i); // head gets latest

    int sum = 0;
    for (auto it = L.begin(); it != L.end(); ++it) {
        sum += *it;
        // operator->()
        auto p = it.operator->();
        ASSERT_NE(p, nullptr);
    }
    // Because push_front adds to head, contents are {4,3,2,1}; sum = 10
    EXPECT_EQ(sum, 10);
}

TEST(FoxList, ConstIteratorTraversal) {
    list<int> L;
    for (int i = 1; i <= 3; ++i) L.push_front(i);

    const list<int>& CL = L;
    int count = 0;
    for (auto it = CL.begin(); it != CL.end(); ++it) {
        // check *it is const int&
        static_assert(std::is_const<std::remove_reference_t<decltype(*it)>>::value, "must be const");
        ++count;
    }
    EXPECT_EQ(count, 3);
}
#endif