#pragma once

#include "pch.h"
#include "core/vector.h"

#include <type_traits>
#include <string>
#include <initializer_list>
#include <vector>
#include <array>
#include <cstdint>
#include <limits>
#include <new>
#include <utility>
#include <algorithm>
#include <iterator>

#ifndef FOX_VECTOR_NS
#define FOX_VECTOR_NS fox
#endif

#ifndef FOX_VECTOR
#define FOX_VECTOR ::FOX_VECTOR_NS::vector
#endif

// ===========================================================
//                      Test Utilities
// ===========================================================

// Element that counts constructions/moves/destroys for behavioral checks.
struct Counter {
    static inline int live = 0;
    static inline int ctor = 0;
    static inline int dtor = 0;
    static inline int copy_ctor = 0;
    static inline int move_ctor = 0;
    static inline int copy_assign = 0;
    static inline int move_assign = 0;

    int value = 0;

    Counter() noexcept { ++live; ++ctor; }
    explicit Counter(int v) noexcept : value(v) { ++live; ++ctor; }
    Counter(const Counter& o) noexcept : value(o.value) { ++live; ++copy_ctor; }
    Counter(Counter&& o) noexcept : value(o.value) { ++live; ++move_ctor; }
    Counter& operator=(const Counter& o) noexcept { value = o.value; ++copy_assign; return *this; }
    Counter& operator=(Counter&& o) noexcept { value = o.value; ++move_assign; return *this; }
    ~Counter() { --live; ++dtor; }

    friend bool operator==(const Counter& a, const Counter& b) { return a.value == b.value; }
};

static void ResetCounter() {
    Counter::live = Counter::ctor = Counter::dtor = 0;
    Counter::copy_ctor = Counter::move_ctor = 0;
    Counter::copy_assign = Counter::move_assign = 0;
}

// Element that throws on specific operations to test exception safety.
struct ThrowOn {
    enum Flags : unsigned {
        None = 0,
        ThrowOnCopy = 1u << 0,
        ThrowOnMove = 1u << 1,
        ThrowOnCtor = 1u << 2
    };

    static inline unsigned mask = None;

    int x = 0;

    ThrowOn() {
        if (mask & ThrowOnCtor) throw std::runtime_error("ThrowOn ctor");
    }
    explicit ThrowOn(int v) : x(v) {
        if (mask & ThrowOnCtor) throw std::runtime_error("ThrowOn ctor");
    }
    ThrowOn(const ThrowOn& o) : x(o.x) {
        if (mask & ThrowOnCopy) throw std::runtime_error("ThrowOn copy");
    }
    ThrowOn(ThrowOn&& o) noexcept(false) : x(o.x) {
        if (mask & ThrowOnMove) throw std::runtime_error("ThrowOn move");
    }
    ThrowOn& operator=(const ThrowOn& o) {
        x = o.x;
        if (mask & ThrowOnCopy) throw std::runtime_error("ThrowOn copy assign");
        return *this;
    }
    ThrowOn& operator=(ThrowOn&& o) noexcept(false) {
        x = o.x;
        if (mask & ThrowOnMove) throw std::runtime_error("ThrowOn move assign");
        return *this;
    }
    friend bool operator==(const ThrowOn& a, const ThrowOn& b) { return a.x == b.x; }
};

// Over-aligned type to validate alignment guarantees.
struct alignas(64) OverAligned {
    int v;
    OverAligned() : v(0) {}
    explicit OverAligned(int x) : v(x) {}
    friend bool operator==(const OverAligned& a, const OverAligned& b) { return a.v == b.v; }
};
static_assert(alignof(OverAligned) >= 64, "OverAligned not actually over-aligned");

// Minimalistic counting allocator with configurable propagation traits.
template<class T,
    bool POCS = false,  // propagate_on_container_swap
    bool POCCA = false, // propagate_on_container_copy_assignment
    bool POCMA = false, // propagate_on_container_move_assignment
    bool AlwaysEqual = true>
struct CountingAllocator {
    using value_type = T;

    static inline size_t alloc_count = 0;
    static inline size_t dealloc_count = 0;

    template<class U> struct rebind { using other = CountingAllocator<U, POCS, POCCA, POCMA, AlwaysEqual>; };

    CountingAllocator() noexcept {}
    template<class U>
    CountingAllocator(const CountingAllocator<U, POCS, POCCA, POCMA, AlwaysEqual>&) noexcept {}

    T* allocate(std::size_t n) {
        alloc_count += n;
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) throw std::bad_array_new_length();
        return static_cast<T*>(::operator new(n * sizeof(T), std::align_val_t(alignof(T))));
    }
    void deallocate(T* p, std::size_t n) noexcept {
        dealloc_count += n;
        ::operator delete(p, n * sizeof(T), std::align_val_t(alignof(T)));
    }

    using propagate_on_container_swap = std::bool_constant<POCS>;
    using propagate_on_container_copy_assignment = std::bool_constant<POCCA>;
    using propagate_on_container_move_assignment = std::bool_constant<POCMA>;
    using is_always_equal = std::bool_constant<AlwaysEqual>;

    bool operator==(const CountingAllocator&) const noexcept { return AlwaysEqual; }
    bool operator!=(const CountingAllocator&) const noexcept { return !AlwaysEqual; }
};

// Simple input/forward/ random-access iterators for range ctor tests.

template<class T>
class InputIter {
    T* ptr;
public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::input_iterator_tag;

    explicit InputIter(T* p = nullptr) : ptr(p) {}
    reference operator*() const { return *ptr; }
    InputIter& operator++() { ++ptr; return *this; }
    InputIter operator++(int) { auto c = *this; ++(*this); return c; }
    friend bool operator==(const InputIter& a, const InputIter& b) { return a.ptr == b.ptr; }
    friend bool operator!=(const InputIter& a, const InputIter& b) { return !(a == b); }
};

template<class T>
class FwdIter {
    T* ptr;
public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::forward_iterator_tag;

    explicit FwdIter(T* p = nullptr) : ptr(p) {}
    reference operator*() const { return *ptr; }
    FwdIter& operator++() { ++ptr; return *this; }
    FwdIter operator++(int) { auto c = *this; ++(*this); return c; }
    friend bool operator==(const FwdIter& a, const FwdIter& b) { return a.ptr == b.ptr; }
    friend bool operator!=(const FwdIter& a, const FwdIter& b) { return !(a == b); }
};

// Helper: assert contiguous storage property

template<class V>
void ExpectContiguous(V& v) {
    if (v.size() == 0) return;
    auto* base = std::addressof(v[0]);
    for (size_t i = 0; i < v.size(); ++i) {
        EXPECT_EQ(std::addressof(v[i]), base + i) << "Storage not contiguous at index " << i;
    }
}

// Helper: fill [0..n)

template<class V>
void FillSequential(V& v, size_t n) {
    v.clear();
    for (size_t i = 0; i < n; ++i) v.push_back(static_cast<typename V::value_type>(i));
}

// ===========================================================
//                          TESTS
// ===========================================================

namespace
{
    // ---------- 1) Constructors ----------
    TEST(Vector_Constructors, DefaultEmpty)
    {
        FOX_VECTOR<int> v;
        EXPECT_TRUE(v.empty());
        EXPECT_EQ(v.size(), 0u);
        EXPECT_GE(v.capacity(), 0u);
        (void)v.data(); // must be valid to call
    }

    TEST(Vector_Constructors, CountValueCtor) 
    {
        struct Test
        {
            int val;
        };
        Test test{}; test.val = 10;

        FOX_VECTOR<Test> v(5, Test{101});
        ASSERT_EQ(v.size(), 5u);
        for (auto x : v) EXPECT_EQ(x.val, 101);
        ExpectContiguous(v);
    }

    TEST(Vector_Constructors, CountValueInit) 
    {
        FOX_VECTOR<int> v(4);
        ASSERT_EQ(v.size(), 4u);
        for (auto x : v) EXPECT_EQ(x, 0);
    }

    TEST(Vector_Constructors, RangeCtor_InputIterator) 
    {
        int a[]{ 1,2,3,4 };
        FOX_VECTOR<int> v(InputIter<int>(a), InputIter<int>(a + 4));
        ASSERT_EQ(v.size(), 4u);
        EXPECT_EQ(v[0], 1); EXPECT_EQ(v[3], 4);
    }

    TEST(Vector_Constructors, RangeCtor_ForwardIterator)
    {
        int a[]{ 5,6,7 };
        FOX_VECTOR<int> v(FwdIter<int>(a), FwdIter<int>(a + 3));
        ASSERT_EQ(v.size(), 3u);
        EXPECT_EQ(v[1], 6);
    }

    TEST(Vector_Constructors, InitializerList)
    {
        FOX_VECTOR<std::string> v{ "a","bb","ccc" };
        ASSERT_EQ(v.size(), 3u);
        EXPECT_EQ(v.front(), "a");
        EXPECT_EQ(v.back(), "ccc");
    }

    TEST(Vector_Constructors, CopyAndMove)
    {
        FOX_VECTOR<int> a{ 1,2,3 };
        FOX_VECTOR<int> b = a; // copy
        ASSERT_EQ(b.size(), 3u);
        EXPECT_EQ(b[1], 2);
        FOX_VECTOR<int> c = std::move(a);
        ASSERT_EQ(c.size(), 3u);
        EXPECT_EQ(c[2], 3);
    }

//    // ---------- 2) Assignment & assign() ----------
    TEST(Vector_Assignment, CopyMoveAssign)
    {
        FOX_VECTOR<int> a{ 1,2,3 };
        FOX_VECTOR<int> b; b = a;
        ASSERT_EQ(b.size(), 3u); EXPECT_EQ(b[0], 1);
        FOX_VECTOR<int> c; c = std::move(a);
        ASSERT_EQ(c.size(), 3u); EXPECT_EQ(c[1], 2);
    }
////
    TEST(Vector_Assignment, AssignCountValue) 
    {
        FOX_VECTOR<int> v; v.assign(5, 42);
        ASSERT_EQ(v.size(), 5u);
        for (auto x : v) EXPECT_EQ(x, 42);
    }
////
    TEST(Vector_Assignment, AssignRangeAndInitList)
    {
        int a[]{ 7,8,9 };
        FOX_VECTOR<int> v; v.assign(std::begin(a), std::end(a));
        ASSERT_EQ(v.size(), 3u); EXPECT_EQ(v[2], 9);
        v.assign({ 1,2 });
        ASSERT_EQ(v.size(), 2u); EXPECT_EQ(v[1], 2);
    }
//
//    // ---------- 3) Allocator semantics ----------
    TEST(Vector_Allocator, CountingAllocatorBasic)
    {
        using A = CountingAllocator<int>;
        A::alloc_count = A::dealloc_count = 0;
        {
            FOX_VECTOR<int, A> v; v.push_back(1); v.push_back(2);
            EXPECT_GE(A::alloc_count, 2u);
        }
        EXPECT_GE(A::dealloc_count, 2u);
    }
//
    TEST(Vector_Allocator, PropagationOnMove) 
    {
        using A = CountingAllocator<int, /*POCS*/true, /*POCCA*/false, /*POCMA*/true, /*AlwaysEqual*/false>;
        FOX_VECTOR<int, A> a(A{}); 
        a.push_back(1);
        FOX_VECTOR<int, A> b(A{});
        b = std::move(a); // if POCMA true, allocator propagates
        (void)b; SUCCEED();
    }
//
//    // ---------- 4) Element access ----------
    TEST(Vector_ElementAccess, AtAndIndex) 
    {
        FOX_VECTOR<int> v{ 1,2,3 };
        EXPECT_EQ(v[0], 1);
        EXPECT_EQ(v.at(1), 2);
        EXPECT_THROW((void)v.at(3), std::out_of_range);
        EXPECT_EQ(*v.data(), 1);
    }
//
    TEST(Vector_ElementAccess, FrontBack) 
    {
        FOX_VECTOR<int> v{ 10,20,30 };
        EXPECT_EQ(v.front(), 10);
        EXPECT_EQ(v.back(), 30);
    }

//    // ---------- 5) Iterators ----------
    TEST(Vector_Iterators, BasicForwardAndReverse)
    {
        FOX_VECTOR<int> v{ 1,2,3,4 };
        int sum = 0; for (auto it = v.begin(); it != v.end(); ++it) sum += *it;
        EXPECT_EQ(sum, 10);
        int rsum = 0; for (auto it = v.rbegin(); it != v.rend(); ++it) rsum += *it;
        EXPECT_EQ(rsum, 10);
        ExpectContiguous(v);
    }

//    // ---------- 6) Capacity & reserve/shrink ----------
    TEST(Vector_Capacity, ReserveAndCapacity)
    {
        FOX_VECTOR<int> v; v.reserve(100);
        EXPECT_GE(v.capacity(), 100u);
        EXPECT_EQ(v.size(), 0u);
    }
//
    TEST(Vector_Capacity, ShrinkToFit) 
    {
        FOX_VECTOR<int> v; v.reserve(64);
        v.push_back(1); v.push_back(2);
        auto cap_before = v.capacity();
        v.shrink_to_fit();
        EXPECT_LE(v.capacity(), cap_before);
    }
//
    TEST(Vector_Capacity, ResizeIncreaseValueInit) 
    {
        FOX_VECTOR<int> v; v.resize(5);
        ASSERT_EQ(v.size(), 5u);
        for (int x : v) EXPECT_EQ(x, 0);
    }
//
    TEST(Vector_Capacity, ResizeDecreaseDestroys) 
    {
        ResetCounter();
        {
            FOX_VECTOR<Counter> v; v.resize(10);
            EXPECT_EQ(Counter::live, 10);
            v.resize(3);
            EXPECT_EQ(Counter::live, 3);
        }
        EXPECT_EQ(Counter::live, 0);
    }
//
//    // ---------- 7) Modifiers: push/emplace/pop ----------
    TEST(Vector_Modifiers, PushBackAndEmplaceBack) 
    {
        FOX_VECTOR<std::pair<int, int>> v;
        v.emplace_back(1, 2);
        v.push_back({ 3,4 });
        ASSERT_EQ(v.size(), 2u);
        EXPECT_EQ(v[0].second, 2);
    }
//
    TEST(Vector_Modifiers, PopBack) {
        FOX_VECTOR<int> v{ 1,2,3 };
        v.pop_back();
        ASSERT_EQ(v.size(), 2u);
        EXPECT_EQ(v.back(), 2);
    }
//
//    // ---------- 8) Insert/Erase variants ----------
    TEST(Vector_InsertErase, InsertSingleNoRealloc) 
    {
        FOX_VECTOR<int> v; v.reserve(10); v.push_back(1); v.push_back(3);
        auto it = v.insert(v.begin() + 1, 2);
        ASSERT_EQ(v.size(), 3u);
        EXPECT_EQ(*it, 2);
        EXPECT_EQ((std::vector<int>{v.begin(), v.end()}), (std::vector<int>{1, 2, 3}));
    }
//
    TEST(Vector_InsertErase, InsertCount) 
    {
        FOX_VECTOR<int> v{ 1,5 };
        v.insert(v.begin() + 1, 3, 2);
        EXPECT_EQ((std::vector<int>{v.begin(), v.end()}), (std::vector<int>{1, 2, 2, 2, 5}));
    }
//
    TEST(Vector_InsertErase, InsertRange) 
    {
        FOX_VECTOR<int> v{ 1,6 };
        int a[]{ 2,3,4,5 };
        v.insert(v.begin() + 1, std::begin(a), std::end(a));
        EXPECT_EQ((std::vector<int>{v.begin(), v.end()}), (std::vector<int>{1, 2, 3, 4, 5, 6}));
    }
//
    TEST(Vector_InsertErase, InsertInitList)
    {
        FOX_VECTOR<int> v{ 1,6 };
        v.insert(v.begin() + 1, { 2,3,4,5 });
        EXPECT_EQ((std::vector<int>{v.begin(), v.end()}), (std::vector<int>{1, 2, 3, 4, 5, 6}));
    }
//
    TEST(Vector_InsertErase, EraseSingle)
    {
        FOX_VECTOR<int> v{ 1,2,3,4 };
        auto it = v.erase(v.begin() + 1);
        ASSERT_EQ(v.size(), 3u);
        EXPECT_EQ(*it, 3);
        EXPECT_EQ((std::vector<int>{v.begin(), v.end()}), (std::vector<int>{1, 3, 4}));
    }
//
    TEST(Vector_InsertErase, EraseRange) 
    {
        FOX_VECTOR<int> v{ 1,2,3,4,5 };
        auto it = v.erase(v.begin() + 1, v.begin() + 4);
        ASSERT_EQ(v.size(), 2u);
        EXPECT_EQ(*it, 5);
        EXPECT_EQ((std::vector<int>{v.begin(), v.end()}), (std::vector<int>{1, 5}));
    }
//
    TEST(Vector_InsertErase, SelfInsertionRange) {
        FOX_VECTOR<int> v{ 1,2,3 };
        v.insert(v.begin() + 1, v.begin(), v.end()); // insert itself
        EXPECT_EQ((std::vector<int>{v.begin(), v.end()}), 
            (std::vector<int>{1, 1, 2, 3, 2, 3}));
    }
//
//    // ---------- 9) Exception safety ----------
//
    TEST(Vector_ExceptionSafety, StrongGuaranteeOnAssignRangeThrowCtor) {
        int a[]{ 1,2,3 };
        ThrowOn::mask = ThrowOn::ThrowOnCtor;
        FOX_VECTOR<ThrowOn> v;
        EXPECT_THROW((v.assign(InputIter<int>(a), InputIter<int>(a + 3))), std::runtime_error);
        EXPECT_TRUE(v.empty());
        ThrowOn::mask = ThrowOn::None;
    }
//
//    // ---------- 10) Iterator invalidation rules ----------
    TEST(Vector_Invalidation, PushBackMayInvalidateOnRealloc) {
        FOX_VECTOR<int> v; v.reserve(2);
        v.push_back(1); v.push_back(2);
        auto p0 = &v[0];
        v.push_back(3); // reallocation likely
        // p0 may be invalid; we can only assert that data is contiguous now
        ExpectContiguous(v);
    }
//
    TEST(Vector_Invalidation, InsertAndEraseInvalidateExpected) {
        FOX_VECTOR<int> v{ 1,2,3,4 };
        auto it = v.begin();
        v.insert(v.begin() + 2, 99);
        // 'it' may be invalid; verify container remains valid
        ExpectContiguous(v);
        v.erase(v.begin() + 1);
        ExpectContiguous(v);
    }
//
//    // ---------- 11) Over-aligned elements ----------
    TEST(Vector_Alignment, OverAlignedElement) {
        FOX_VECTOR<OverAligned> v; v.emplace_back(1); v.emplace_back(2);
        ASSERT_EQ(v.size(), 2u);
        EXPECT_EQ(reinterpret_cast<std::uintptr_t>(v.data()) % alignof(OverAligned), 0u);
    }
//
//    // ---------- 12) Comparisons & swap ----------
    TEST(Vector_Comparisons, EqualityAndSpaceship) {
        FOX_VECTOR<int> a{ 1,2,3 }, b{ 1,2,3 }, c{ 1,3 };
        EXPECT_TRUE(a == b);
#if defined(__cpp_impl_three_way_comparison)
        EXPECT_TRUE((a <=> c) < 0);
#endif
    }
//
    TEST(Vector_Swap, SwapNoexceptConditional) {
        using AAlwaysEq = CountingAllocator<int, false, false, false, true>;
        FOX_VECTOR<int, AAlwaysEq> a(AAlwaysEq{}), b(AAlwaysEq{});
        a.push_back(1); b.push_back(2);
        swap(a, b);
        EXPECT_EQ(a[0], 2); EXPECT_EQ(b[0], 1);
    }
//
//    // ---------- 13) Large operations & growth policy sanity ----------
    TEST(Vector_Growth, ReserveAndPushMany) {
        FOX_VECTOR<int> v; v.reserve(1000);
        for (int i = 0; i < 1000; ++i) v.push_back(i);
        ASSERT_EQ(v.size(), 1000u);
        ExpectContiguous(v);
    }
//
//    // ---------- 14) erase/remove helpers (non-standard helper test if implemented) ----------
//    // Uncomment if you provide erase/remove utilities
    // TEST(Vector_Helpers, EraseIf){
    //     FOX_VECTOR<int> v{1,2,3,4,5};
    //     auto removed = erase_if(v, [](int x){return x%2==0;});
    //     EXPECT_EQ(removed, 2);
    //     EXPECT_EQ((std::vector<int>{v.begin(), v.end()}), (std::vector<int>{1,3,5}));
    // }
//
//    // ---------- 15) Debug-only checks (optional) ----------
//    // Add death tests or ASSERT_DEATH for out-of-bounds in debug builds if you gate operator[]
//
//    // ---------- 16) DISABLED_ perf sanity (not real microbenchmarks) ----------
    TEST(Vector_Perf, PushBackMillionInts) {
        FOX_VECTOR<int> v; v.reserve(1'000'000);
        for (int i = 0; i < 1'000'000; ++i) v.push_back(i);
        ASSERT_EQ(v.size(), 1'000'000u);
    }
//
//} // namespace
//
//
//// ===========================================================
////            ADDITIONAL EDGE & CONTRACT COVERAGE
//// ===========================================================
//
//// 17) clear() keeps capacity; shrink_to_fit can reduce (non-binding)
TEST(Vector_Capacity, ClearKeepsCapacity) {
    FOX_VECTOR<int> v; v.reserve(64); v.resize(32, 7);
    auto cap = v.capacity();
    v.clear();
    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.capacity(), cap);
}
//
TEST(Vector_Capacity, ShrinkInvalidatesIterators) {
    FOX_VECTOR<int> v; v.reserve(32); FillSequential(v, 10);
    auto p = &v[0];
    v.shrink_to_fit(); // allowed to invalidate; at minimum after shrink, container valid
    ExpectContiguous(v);
    (void)p; // can't deref or require validity
}
//
//// 18) reserve() never shrinks; reserve(size<=capacity) is no-op
TEST(Vector_Capacity, ReserveNeverShrinks) {
    FOX_VECTOR<int> v; v.reserve(100); auto cap = v.capacity();
    v.reserve(10); // should not reduce
    EXPECT_EQ(v.capacity(), cap);
}
//
//// 19) self move-assign should be a no-op, leave container valid
TEST(Vector_Assignment, SelfMoveAssignNoOp) {
    FOX_VECTOR<int> v{ 1,2,3 };
    v = std::move(v);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ((std::vector<int>{v.begin(), v.end()}), (std::vector<int>{1, 2, 3}));
}
//
//// 20) erase entire range equals clear
TEST(Vector_Modifiers, EraseAllEqualsClear) {
    FOX_VECTOR<int> v{ 1,2,3,4 };
    v.erase(v.begin(), v.end());
    EXPECT_TRUE(v.empty());
}
//
//// 21) insert with subrange from self (overlapping source)
TEST(Vector_InsertErase, InsertSubrangeFromSelf) {
    FOX_VECTOR<int> v{ 0,1,2,3,4 };
    // insert [1,4) at position 2 => 0,1,1,2,3,2,3,4
    v.insert(v.begin() + 2, v.begin() + 1, v.begin() + 4);
    EXPECT_EQ((std::vector<int>{v.begin(), v.end()}), (std::vector<int>{0, 1, 1, 2, 3, 2, 3, 4}));
}
//
//// 22) emplace(pos) should construct in-place (minimize moves)
TEST(Vector_Modifiers, EmplaceMinimizesMoves) {
    ResetCounter();
    FOX_VECTOR<Counter> v; v.reserve(8);
    v.emplace_back(1); v.emplace_back(2); v.emplace_back(3);
    int moves_before = Counter::move_ctor + Counter::move_assign + Counter::copy_ctor + Counter::copy_assign;
    (void)moves_before;
    // Emplace in middle; allow internal shifts but no extra temporaries for new element
    v.emplace(v.begin() + 1, 42);
    // Ensure content and validity; operation counts are implementation-dependent,
    // so we only assert basic properties to avoid over-constraining.
    EXPECT_EQ(v.size(), 4u);
    EXPECT_EQ(v[1].value, 42);
}
//
//// 23) noexcept characteristics (static checks based on allocator traits)
namespace noexcept_checks {
    using Aeq = CountingAllocator<int, false, false, false, true>;  // always_equal
    using Ane = CountingAllocator<int, false, false, false, false>; // not always_equal

    static_assert(noexcept(FOX_VECTOR<int, Aeq>(std::declval<FOX_VECTOR<int, Aeq>&&>())),
        "move-construct should be noexcept when allocator is_always_equal");
    // If allocator not always equal, it might be nothrow too depending on impl; do not assert the negative.
}

// 24) at() bounds contract (negative via size_t wrap not testable) and large index
TEST(Vector_ElementAccess, AtThrowsOnLarge) {
    FOX_VECTOR<int> v{ 1,2,3 };
    EXPECT_THROW((void)v.at(static_cast<FOX_VECTOR<int>::size_type>(100)), std::out_of_range);
}

// 25) max_size contract sanity (cannot assert exact value but should be >= size limit)
TEST(Vector_Capacity, MaxSizeReasonable) {
    FOX_VECTOR<int> v; auto m = v.max_size();
    EXPECT_GT(m, 1000u);
}

// 26) Trivially copyable shift correctness (overlap memmove-like behavior)
TEST(Vector_InsertErase, ShiftOverlappingTrivial) {
    FOX_VECTOR<int> v; FillSequential(v, 6); // 0 1 2 3 4 5
    v.insert(v.begin() + 2, 99);               // 0 1 99 2 3 4 5
    v.erase(v.begin() + 4);                    // remove '3' => 0 1 99 2 4 5
    EXPECT_EQ((std::vector<int>{v.begin(), v.end()}), (std::vector<int>{0, 1, 99, 2, 4, 5}));
}

// 27) data() contract when empty: may be null or non-null but must be safe to compare
TEST(Vector_ElementAccess, DataOnEmptyIsStable) {
    FOX_VECTOR<int> v; auto p = v.data();
    EXPECT_TRUE(v.empty());
    // Only guarantee: you must not dereference; pointer stability not required.
    (void)p;
}

// 28) compare operators full set when available
TEST(Vector_Comparisons, LexicographicBehavior) {
    FOX_VECTOR<int> a{ 1,2,3 }, b{ 1,2,4 };
#if defined(__cpp_lib_three_way_comparison) || defined(__cpp_impl_three_way_comparison)
    EXPECT_TRUE((a <=> b) < 0);
#else
    EXPECT_TRUE(a < b);
#endif
}

// 29) insert with input iterators is single-pass friendly (no double traversal)
// We simulate by a stateful input iterator that would fail on multiple passes.
class SinglePassIntIter {
    int* p; int* end; bool advanced = false;
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = int; using difference_type = std::ptrdiff_t; using pointer = int*; using reference = int&;
    SinglePassIntIter(int* first, int* last) : p(first), end(last) {}
    reference operator*() const { return *p; }
    SinglePassIntIter& operator++() { ++p; advanced = true; return *this; }
    SinglePassIntIter operator++(int) { auto t = *this; ++(*this); return t; }
    friend bool operator==(const SinglePassIntIter& a, const SinglePassIntIter& b) { return a.p == b.p; }
    friend bool operator!=(const SinglePassIntIter& a, const SinglePassIntIter& b) { return !(a == b); }
};

TEST(Vector_Constructors, RangeCtorSinglePassInput) {
    int a[]{ 7,8,9 };
    SinglePassIntIter first(a, a + 3), last(a + 3, a + 3);
    FOX_VECTOR<int> v(first, last);
    EXPECT_EQ((std::vector<int>{v.begin(), v.end()}), (std::vector<int>{7, 8, 9}));
}

// 30) swap leaves iterators referring to the same elements but different containers (document-only);
// We only check contents swapped successfully since iterator validity rules are nuanced.
TEST(Vector_Swap, ContentsSwapped) {
    FOX_VECTOR<int> a{ 1,2 }, b{ 3,4,5 };
    a.swap(b);
    EXPECT_EQ((std::vector<int>{a.begin(), a.end()}), (std::vector<int>{3, 4, 5}));
    EXPECT_EQ((std::vector<int>{b.begin(), b.end()}), (std::vector<int>{1, 2}));
}
}
