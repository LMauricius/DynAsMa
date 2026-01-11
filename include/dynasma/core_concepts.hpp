#pragma once
#ifndef INCLUDED_DYNASMA_CONCEPTS_H
#define INCLUDED_DYNASMA_CONCEPTS_H

#include "dynasma/util/dynamic_typing.hpp"
#include "dynasma/util/helpful_concepts.hpp"

#include <concepts>
#include <variant>

namespace dynasma {

/**
 * Any type derived from PolymorphicBase. Such types can be cast to
 * PolymorphicBase for type erasure and safely cast back to the original type
 */
template <typename T>
concept StandardPolymorphic = std::is_base_of<PolymorphicBase, T>::value;

/**
 * Any type derived from PtrAwareBase.
 * Raw pointers to such types can be cast to FirmPtr and LazyPtr
 */
template <typename T>
concept RawConvertibleToPtr = std::is_base_of<PtrAwareBase, T>::value;

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
concept AssetLike = StandardPolymorphic<T> && requires(const T &a) {
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
};

/**
 * An asset seed, used to construct an asset later.
 * Must be copy constructible in addition to being SeedLike.
 */
template <class T>
concept ReloadableSeedLike = SeedLike<T> && std::is_copy_constructible_v<T>;

/**
 * A sortable class, which can be compared using less_than operator.
 */
template <class T>
concept Sortable = requires(T a, T b) {
    // can be compared
    { a < b } -> std::convertible_to<bool>;
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
concept CacheableSeedLike = ReloadableSeedLike<T> && Sortable<T>;

namespace internal {

template <class T, class... Args> struct ConstructibleFromVariantOptions_type;
template <class T, class... Args>
struct ConstructibleFromVariantOptions_type<T, std::variant<Args...>> {
    static constexpr bool value = (ConstructibleFrom<T, Args> && ...);
};

} // namespace internal

/**
 * Concept checking if an asset type is constructible from each variant value.
 * @tparam T the asset type
 * @tparam VariantT the variant type
 */
template <class T, class VariantT>
concept ConstructibleFromVariantOptions =
    internal::ConstructibleFromVariantOptions_type<T, VariantT>::value;

/**
 * @brief Allocator for type derived from Seed::Asset. The allocator's
 * value_type must be constructible from each of the possible kernel values,
 * just like Seed::Asset.
 * @param Seed the seed type
 */
template <class A, class Seed>
concept SeededAllocatorLike =
    DerivedAllocatorLike<A, typename Seed::Asset> &&
    ConstructibleFromVariantOptions<typename A::value_type,
                                    decltype(Seed::kernel)>;

/**
 * @brief A seed can be constructed by assigning only its kernel to a value
 * @param Seed the seed type
 * @param KernelValueT the kernel value type
 */
template <class Seed, class KernelValueT>
concept SeedConstructibleFromKernelValue = requires(KernelValueT v) {
    { new Seed{.kernel = v} };
};

}; // namespace dynasma

#endif // INCLUDED_DYNASMA_CONCEPTS_H