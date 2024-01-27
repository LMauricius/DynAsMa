#pragma once
#ifndef INCLUDED_DYNASMA_CONCEPTS_H
#define INCLUDED_DYNASMA_CONCEPTS_H

#include "dynasma/util/dynamic_typing.hpp"
#include "dynasma/util/helpful_concepts.hpp"

#include <concepts>
#include <functional>
#include <variant>

namespace dynasma {

/**
 * An asset, constructible by an asset manager, from a seed kernel.
 * Must have a method memory_cost() returning the size (in bytes) in memory.
 * @example @code
 *  struct MyAsset: public PolymorphicBase {
 *      MyAsset(std::filename);
 *      MyAsset(json::object);
 *
 *      // The (estimated) number of bytes taken until
 *      // the destruction of this asset
 *      std::size_t memory_cost() const;
 *  }
 * @endcode
 */
template <typename T>
concept AssetLike =
    std::is_base_of<PolymorphicBase, T>::value && requires(const T &a) {
        { a.memory_cost() } -> std::convertible_to<std::size_t>;
    };

/**
 * An asset seed, used to construct an asset.
 * Must have an Asset typedef.
 * Must have a variant member `kernel`.
 * The Asset must be constructible from each kernel value.
 * Must have a method load_cost() returning the cost of loading.
 * @details
 * @example @code
 *  struct MySeed {
 *      using Asset = MyAsset;
 *
 *      std::variant<std::filename, json::object> kernel;
 *
 *      // The (estimated) cost of loading the asset, expressed in an
 *      // arbitrary measure relative to other seeds
 *      // (i.e. time to load or file size)
 *      std::size_t load_cost() const;
 *  }
 * @endcode
 */
template <class T>
concept SeedLike = requires(T seed) {
    // we need to have an Asset type defined
    typename T::Asset;

    // we need to have a kernel member
    seed.kernel;

    // Asset needs to be constructible via each of the possible kernel value
    // types
    std::visit([](const auto &kerval) { typename T::Asset a(kerval); },
               seed.kernel);
};

/**
 * An asset seed, used to construct an asset. Allows for sorting.
 * Must have an Asset typedef.
 * Must have a variant member `kernel`.
 * The Asset must be constructible from each kernel value.
 * Must have a method load_cost() returning the cost of loading.
 * Must be sortable using less_than operator
 * @details
 * @example @code
 *  struct MySortableSeed {
 *      using Asset = MyAsset;
 *
 *      std::variant<std::filename, json::object> kernel;
 *
 *      // The (estimated) cost of loading the asset, expressed in an
 *      // arbitrary measure relative to other seeds
 *      // (i.e. time to load or file size)
 *      std::size_t load_cost() const;
 *
 *      // The comparator of the seed
 *      bool operator<() const;
 *  }
 * @endcode
 */
template <class T>
concept SortableSeedLike = SeedLike<T> && requires(T seed1, T seed2) {
    // can be compared
    { seed1 < seed2 } -> std::convertible_to<bool>;
};

/**
 * @brief Allocator for type derived from Seed::Asset. The allocator's
 * value_type must be constructible from each of the possible kernel values,
 * just like Seed::Asset.
 * @param Seed the seed type
 */
template <class A, class Seed>
concept SeededAllocatorLike =
    DerivedAllocatorLike<A, typename Seed::Asset> && requires(Seed seed) {
        // Asset needs to be constructible via each of the possible kernel value
        // types
        std::visit([](const auto &kerval) { typename A::value_type a(kerval); },
                   seed.kernel);
    };
}; // namespace dynasma

#endif // INCLUDED_DYNASMA_CONCEPTS_H