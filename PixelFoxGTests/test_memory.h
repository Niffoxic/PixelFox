#pragma once

#include "pch.h"

#include <type_traits>
#include <new>
#include <cstddef>

#include "core/utility.h"
#include "core/memory.h"

namespace
{

    struct Counts 
    {
        static inline int ctor          = 0;
        static inline int dtor          = 0;
        static inline int copy_ctor     = 0;
        static inline int move_ctor     = 0;
        static inline int copy_assign   = 0;
        static inline int move_assign   = 0;

        static void reset() 
        {
            ctor = dtor = copy_ctor = move_ctor = copy_assign = move_assign = 0;
        }
    };

    struct ProbeMemory 
    {
        int v{ 0 };

        ProbeMemory() noexcept { ++Counts::ctor; }
        explicit ProbeMemory(int x) noexcept : v(x) { ++Counts::ctor; }

        ProbeMemory(const ProbeMemory& rhs) noexcept : v(rhs.v) { ++Counts::copy_ctor; }
        ProbeMemory(ProbeMemory&& rhs) noexcept : v(rhs.v) { ++Counts::move_ctor; }

        ProbeMemory& operator=(const ProbeMemory& rhs) noexcept { v = rhs.v; ++Counts::copy_assign; return *this; }
        ProbeMemory& operator=(ProbeMemory&& rhs) noexcept { v = rhs.v; ++Counts::move_assign; return *this; }

        ~ProbeMemory() { ++Counts::dtor; }
    };

    struct OverloadedAddr 
    {
        int x{ 0 };
        OverloadedAddr* operator&() noexcept { return nullptr; }
    };

} // namespace

TEST(FoxMemory, AddressofBypassesOverloadedOperatorAmp)
{
    OverloadedAddr obj;
    auto p_fake = &obj;
    auto p_real = fox::addressof(obj);
    EXPECT_EQ(p_fake, nullptr);
    EXPECT_NE(p_real, nullptr);
}

TEST(FoxMemory, ConstructAtAndDestroyAt)
{
    Counts::reset();

    alignas(ProbeMemory) unsigned char buffer[sizeof(ProbeMemory)];
    auto* p = reinterpret_cast<ProbeMemory*>(buffer);

    fox::construct_at(p, 42);
    EXPECT_EQ(p->v, 42);
    EXPECT_EQ(Counts::ctor, 1);

    fox::destroy_at(p);
    EXPECT_EQ(Counts::dtor, 1);
}

TEST(FoxMemory, DestroyRange) {
    Counts::reset();

    constexpr std::size_t N = 3;
    alignas(ProbeMemory) unsigned char buf[sizeof(ProbeMemory) * N];
    auto* first = reinterpret_cast<ProbeMemory*>(buf);

    for (std::size_t i = 0; i < N; ++i)
        fox::construct_at(first + i, static_cast<int>(i + 1));

    EXPECT_EQ(Counts::ctor, 3);

    fox::destroy(first, first + N);
    EXPECT_EQ(Counts::dtor, 3);
}

TEST(FoxMemory, UninitializedCopyBuildsRange)
{
    Counts::reset();

    ProbeMemory src[3] = { ProbeMemory{1}, ProbeMemory{2}, ProbeMemory{3} };
    Counts::ctor = 0;

    void* raw = ::operator new(sizeof(ProbeMemory) * 3);
    auto* dst = static_cast<ProbeMemory*>(raw);

    auto* end_it = fox::uninitialized_copy(src, src + 3, dst);
    EXPECT_EQ(end_it, dst + 3);

    EXPECT_EQ(Counts::copy_ctor, 3);
    fox::destroy(dst, dst + 3);
    ::operator delete(dst);
    EXPECT_EQ(Counts::dtor, 3);
}

TEST(FoxMemory, UninitializedMoveBuildsRange) 
{
    Counts::reset();

    ProbeMemory src[2] = { ProbeMemory{10}, ProbeMemory{20} };
    Counts::ctor = 0;

    void* raw = ::operator new(sizeof(ProbeMemory) * 2);
    auto* dst = static_cast<ProbeMemory*>(raw);

    static_assert(std::is_same_v<decltype(src + 0), ProbeMemory*>, "src decays to pointer");
    static_assert(std::is_same_v<decltype(dst), ProbeMemory*>, "dst is pointer");

    auto* end_it = fox::uninitialized_move(src, src + 2, dst);

    auto* expected = dst + (src + 2 - src);
    EXPECT_EQ(end_it, expected) << "end_it must be one-past-last";

    EXPECT_EQ(Counts::move_ctor, 2);

    fox::destroy(dst, dst + 2);
    ::operator delete(dst);
    EXPECT_EQ(Counts::dtor, 2);
}

TEST(FoxMemory, AllocatorAllocateDeallocate)
{
    fox::allocator<int> a;

    int* p0 = a.allocate(0);
    EXPECT_EQ(p0, nullptr);

    int* p = a.allocate(4);
    ASSERT_NE(p, nullptr);
    p[0] = 1; p[1] = 2; p[2] = 3; p[3] = 4;

    a.deallocate(p, 4);
}

TEST(FoxMemory, AllocatorTraitsConstructDestroy)
{
    Counts::reset();

    using A = fox::allocator<ProbeMemory>;
    using AT = fox::allocator_traits<A>;

    A a;
    ProbeMemory* p = AT::allocate(a, 3);
    ASSERT_NE(p, nullptr);

    AT::construct(a, p + 0, 7);
    AT::construct(a, p + 1, 8);
    AT::construct(a, p + 2, 9);

    EXPECT_EQ(p[0].v, 7);
    EXPECT_EQ(p[1].v, 8);
    EXPECT_EQ(p[2].v, 9);
    EXPECT_EQ(Counts::ctor, 3);

    AT::destroy(a, p + 0);
    AT::destroy(a, p + 1);
    AT::destroy(a, p + 2);
    EXPECT_EQ(Counts::dtor, 3);

    AT::deallocate(a, p, 3);
}

TEST(FoxMemory, AllocatorTraitsRebindAlloc)
{
    using A = fox::allocator<int>;
    using Rebound = fox::allocator_traits<A>::rebind_alloc<double>;
    Rebound b;
    double* p = b.allocate(2);
    ASSERT_NE(p, nullptr);
    p[0] = 3.14;
    p[1] = 2.71;
    b.deallocate(p, 2);
}
