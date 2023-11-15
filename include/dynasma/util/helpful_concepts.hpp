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
concept ConstQualified = std::same_as<T, std::remove_const_t<T>>;

} // namespace dynasma

#endif // INCLUDED_DYNASMA_HELPFUL_CONCEPTS_H