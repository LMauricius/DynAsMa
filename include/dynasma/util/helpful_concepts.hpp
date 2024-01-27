#pragma once
#ifndef INCLUDED_DYNASMA_HELPFUL_CONCEPTS_H
#define INCLUDED_DYNASMA_HELPFUL_CONCEPTS_H

#include <concepts>
#include <functional>

namespace dynasma {

/**
 * @brief Whether it is a hashable type
 */
template <typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

/**
 * @brief Whether it is a (key)comparable type
 */
template <typename T>
concept KeyComparable = requires(T l, T r) {
    { std::less<T>(l, r) } -> std::convertible_to<bool>;
};

/**
 * @brief Whether it can be used as a map key
 */
template <typename T>
concept KeyUsable = Hashable<T> || KeyComparable<T>;

/**
 * @brief Whether it is const qualified
 */
template <typename T>
concept ConstQualified = std::same_as<T, const T>;

/**
 * @brief Whether it is volatile qualified
 */
template <typename T>
concept VolatileQualified = std::same_as<T, volatile T>;

/**
 * @brief Whether it is cv qualified
 */
template <typename T>
concept CVQualified = ConstQualified<T> || VolatileQualified<T>;
template <typename T>
concept NotCVQualified = !CVQualified<T>;

/**
 * @brief Whether type A is less or equally cv qualified than type B
 */
template <typename A, typename B>
concept MoreOrEquallyCVQualified = (ConstQualified<A> || !ConstQualified<B>)&&(
    VolatileQualified<A> || !VolatileQualified<B>);

/**
 * @brief Allocator
 */
template <class A, typename T = A::value_type,
          typename pointer = std::allocator_traits<A>::pointer,
          typename const_pointer = std::allocator_traits<A>::const_pointer,
          typename void_pointer = std::allocator_traits<A>::void_pointer,
          typename const_void_pointer =
              std::allocator_traits<A>::const_void_pointer,
          typename size_type = std::allocator_traits<A>::size_type,
          typename difference_type = std::allocator_traits<A>::difference_type>
concept AllocatorLike =
    requires(A a, pointer p, const_pointer cp, void_pointer vp,
             const_void_pointer cvp, decltype(*p) r, size_type n) {
        typename T;

        // *p
        { *p } -> std::same_as<T &>;

        // *cp
        { *cp } -> std::same_as<const T &>;

        // static_cast<Alloc::pointer>(vp)
        { static_cast<pointer>(vp) } -> std::same_as<pointer>;

        // static_cast<Alloc::const_pointer>(cvp)
        { static_cast<const_pointer>(cvp) } -> std::same_as<const_pointer>;

        // std::pointer_traits<Alloc::pointer>::pointer_to(r)
        { std::pointer_traits<pointer>::pointer_to(r) };

        { a.allocate(n) } -> std::same_as<pointer>;

        { a.deallocate(p, n) } -> std::same_as<void>;
    };

template <class A, typename T = A::value_type,
          typename pointer = std::allocator_traits<A>::pointer,
          typename const_void_pointer =
              std::allocator_traits<A>::const_void_pointer,
          typename size_type = std::allocator_traits<A>::size_type>
concept AllocatorWLocality =
    AllocatorLike<A> && requires(A a, const_void_pointer cvp, size_type n) {
        { a.allocate(n, cvp) } -> std::same_as<pointer>;
    };

/**
 * @brief Allocator for type derived from T
 */
template <
    class A, typename T, typename pointer = std::allocator_traits<A>::pointer,
    typename const_pointer = std::allocator_traits<A>::const_pointer,
    typename void_pointer = std::allocator_traits<A>::void_pointer,
    typename const_void_pointer = std::allocator_traits<A>::const_void_pointer,
    typename size_type = std::allocator_traits<A>::size_type,
    typename difference_type = std::allocator_traits<A>::difference_type>
concept DerivedAllocatorLike =
    AllocatorLike<A> && std::derived_from<typename A::value_type, T>;

template <
    class A, typename T, typename pointer = std::allocator_traits<A>::pointer,
    typename const_void_pointer = std::allocator_traits<A>::const_void_pointer,
    typename size_type = std::allocator_traits<A>::size_type>
concept DerivedAllocatorWLocality =
    DerivedAllocatorLike<A, T> &&
    requires(A a, const_void_pointer cvp, size_type n) {
        { a.allocate(n, cvp) } -> std::same_as<pointer>;
    };

} // namespace dynasma

#endif // INCLUDED_DYNASMA_HELPFUL_CONCEPTS_H