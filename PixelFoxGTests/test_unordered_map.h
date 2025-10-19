#pragma once
/*
 * LLM Generated TestCode
 * reason: limited time I can't implement and write its google test within time constraints
*/
#pragma once
#include "pch.h"
#include "core/list.h"

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

// Adjust this include to your actual header file:
#include "core/unordered_map.h"   // contains namespace fox { template<class Key, class T, ...> class unordered_map }

using fox::unordered_map;

// ---------- Helpers ----------
struct NoDefault {
    int v;
    explicit NoDefault(int x) : v(x) {}
    NoDefault() = delete;
};

struct CaseInsensitiveHash {
    size_t operator()(const std::string& s) const noexcept {
        size_t h = 1469598103934665603ull; // FNV-1a 64
        for (unsigned char c : s) {
            unsigned char lc = (unsigned char)std::tolower(c);
            h ^= lc;
            h *= 1099511628211ull;
        }
        return (size_t)h;
    }
};
struct CaseInsensitiveEq {
    bool operator()(const std::string& a, const std::string& b) const noexcept {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::tolower((unsigned char)a[i]) != std::tolower((unsigned char)b[i])) return false;
        }
        return true;
    }
};

// ---------- Tests ----------

TEST(FoxUnorderedMap, InsertOrAssign_Basics) {
    unordered_map<std::string, int> m(8);
    EXPECT_TRUE(m.insert_or_assign("a", 1));
    EXPECT_TRUE(m.insert_or_assign("b", 2));
    EXPECT_EQ(m.size(), 2u);

    // assign existing should return false, not increase size
    EXPECT_FALSE(m.insert_or_assign("a", 42));
    EXPECT_EQ(m.size(), 2u);

    auto* pa = m.find("a");
    ASSERT_NE(pa, nullptr);
    EXPECT_EQ(*pa, 42);

    EXPECT_TRUE(m.contains("b"));
    EXPECT_FALSE(m.contains("c"));
}

TEST(FoxUnorderedMap, TryEmplaceGet_ConstructsInPlace) {
    unordered_map<int, std::string> m(4);
    std::string big = "hello-world";
    auto& ref = m.try_emplace_get(7, big); // copies
    EXPECT_EQ(ref, "hello-world");
    EXPECT_EQ(m.size(), 1u);

    // second call returns reference to existing element (no insert)
    auto& ref2 = m.try_emplace_get(7, "ignored");
    EXPECT_EQ(&ref, &ref2);
    EXPECT_EQ(m.size(), 1u);
}

TEST(FoxUnorderedMap, OperatorBracket_DefaultConstructs) {
    unordered_map<int, int> m(2);
    EXPECT_EQ(m.size(), 0u);

    // operator[] inserts default-constructed value
    int& v = m[10];
    EXPECT_EQ(v, 0);
    EXPECT_EQ(m.size(), 1u);

    v = 123;
    EXPECT_EQ(m.at(10), 123);

    // second [] on same key returns the same reference
    int& v2 = m[10];
    EXPECT_EQ(&v, &v2);
    EXPECT_EQ(m.size(), 1u);
}

TEST(FoxUnorderedMap, FindContainsAt_ConstAccess) {
    unordered_map<std::string, int> m(4);
    m.insert_or_assign("x", 9);
    m.insert_or_assign("y", 8);

    const auto& cm = m;
    const int* py = cm.find("y");
    ASSERT_NE(py, nullptr);
    EXPECT_EQ(*py, 8);
    EXPECT_TRUE(cm.contains("x"));
    EXPECT_EQ(cm.at("x"), 9);
}

TEST(FoxUnorderedMap, Erase_RemovesAndReturns) {
    unordered_map<int, int> m(4);
    m.insert_or_assign(1, 10);
    m.insert_or_assign(2, 20);
    EXPECT_EQ(m.size(), 2u);

    EXPECT_TRUE(m.erase(2));
    EXPECT_EQ(m.size(), 1u);
    EXPECT_FALSE(m.contains(2));

    EXPECT_FALSE(m.erase(2)); // already gone
    EXPECT_EQ(m.size(), 1u);
}

TEST(FoxUnorderedMap, ReserveAndRehash_PreservesElements) {
    unordered_map<int, int> m(2);
    for (int i = 0; i < 50; ++i) {
        m.insert_or_assign(i, i * 3);
    }
    EXPECT_EQ(m.size(), 50u);

    const auto beforeBuckets = m.bucket_count();
    m.reserve(200); // should expand
    const auto afterBuckets = m.bucket_count();

    EXPECT_GE(afterBuckets, beforeBuckets);

    // Minimum buckets needed before power-of-two rounding:
    const double lf = std::max(0.0001, (double)m.max_load_factor());
    const size_t min_required = (size_t)std::ceil(200.0 / lf);

    // Your map rounds up to next power-of-two internally, so afterBuckets
    // should be >= min_required (often much larger due to pow2 rounding).
    EXPECT_GE(afterBuckets, min_required);

    for (int i = 0; i < 50; ++i) {
        auto* p = m.find(i);
        ASSERT_NE(p, nullptr);
        EXPECT_EQ(*p, i * 3);
    }
}

TEST(FoxUnorderedMap, MoveCtorAndMoveAssign) {
    unordered_map<std::string, int> a(8);
    a.insert_or_assign("k1", 11);
    a.insert_or_assign("k2", 22);
    EXPECT_EQ(a.size(), 2u);

    // move construct
    unordered_map<std::string, int> b(std::move(a));
    EXPECT_EQ(b.size(), 2u);
    EXPECT_EQ(b.at("k1"), 11);
    EXPECT_EQ(b.at("k2"), 22);

    // a should be empty (moved-from but valid)
    EXPECT_EQ(a.size(), 0u);
    EXPECT_EQ(a.bucket_count(), 0u);

    // move assign
    unordered_map<std::string, int> c(4);
    c.insert_or_assign("z", 999);
    c = std::move(b);

    EXPECT_EQ(c.size(), 2u);
    EXPECT_TRUE(c.contains("k1"));
    EXPECT_TRUE(c.contains("k2"));
    EXPECT_EQ(c.at("k2"), 22);

    EXPECT_EQ(b.size(), 0u);
    EXPECT_EQ(b.bucket_count(), 0u);
}

TEST(FoxUnorderedMap, CustomHasherAndEq_CaseInsensitive) {
    unordered_map<std::string, int, CaseInsensitiveHash, CaseInsensitiveEq> m(4);
    m.insert_or_assign("Hello", 7);
    EXPECT_TRUE(m.contains("hello"));
    EXPECT_TRUE(m.contains("HELLO"));

    EXPECT_EQ(m.at("hElLo"), 7);

    // Assign through different case
    EXPECT_FALSE(m.insert_or_assign("heLLo", 42)); // same key by Eq
    EXPECT_EQ(m.at("HELLO"), 42);
}

TEST(FoxUnorderedMap, NonDefaultConstructible_TryEmplaceGet) {
    unordered_map<int, NoDefault> m(2);

    // try_emplace_get constructs in place; operator[] would not compile for NoDefault
    auto& ref = m.try_emplace_get(5, 123);
    EXPECT_EQ(ref.v, 123);
    EXPECT_EQ(m.size(), 1u);

    // second call returns same element
    auto& ref2 = m.try_emplace_get(5, 999);
    EXPECT_EQ(&ref, &ref2);
    EXPECT_EQ(ref2.v, 123);
}

TEST(FoxUnorderedMap, ManyInsertions_Stress) {
    unordered_map<uint32_t, uint32_t> m(8);
    const uint32_t N = 1000;

    for (uint32_t i = 0; i < N; ++i) {
        m.insert_or_assign(i, i + 1);
    }
    EXPECT_EQ(m.size(), (size_t)N);

    // random-ish probing
    for (uint32_t i = 0; i < N; i += 7) {
        auto* p = m.find(i);
        ASSERT_NE(p, nullptr);
        EXPECT_EQ(*p, i + 1);
    }

    // erase some
    size_t removed = 0;
    for (uint32_t i = 0; i < N; i += 5) {
        removed += m.erase(i) ? 1 : 0;
    }
    EXPECT_EQ(m.size(), (size_t)N - removed);
}
