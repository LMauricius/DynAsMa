#pragma once
#ifndef INCLUDED_DYNASMA_CACHER_ABSTRACT_H
#define INCLUDED_DYNASMA_CACHER_ABSTRACT_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/pointer.hpp"
#include "dynasma/util/helpful_concepts.hpp"

namespace dynasma {

/**
 * @brief Abstract base class for cachers - asset managers that cache and reuse
 * objects with the same seeds
 */
template <CacheableSeedLike Seed> class AbstractCacher {
  public:
    using Asset = typename Seed::Asset;

    virtual ~AbstractCacher(){};

    /**
     * @brief Registers a seed for constructing an instance of Seed::Asset,
     * only if the asset is not already cached
     * @param seed an object containing the `kernel` member whose value will be
     * passed to the Seed::Asset class' constructor
     * @returns A LazyPtr to the (to-be-)constructed asset. Cast it to FirmPtr
     * to retrieve the asset
     */
    virtual LazyPtr<Asset> retrieve_asset(const Seed &seed) {
        return retrieve_asset(Seed(seed));
    }
    virtual LazyPtr<Asset> retrieve_asset(Seed &&seed) {
        return retrieve_asset((const Seed &)seed);
    }

    /**
     * @brief constructs and registers a seed from the given kernel value
     * @tparam ValueT the type of the kernel value
     * @param value the kernel value
     * @returns A LazyPtr to the (to-be-)constructed asset. Cast it to FirmPtr
     * to retrieve the asset
     */
    template <class ValueT>
        requires SeedConstructibleFromKernelValue<Seed, ValueT>
    LazyPtr<Asset> retrieve_asset_k(const ValueT &value) {
        return retrieve_asset(Seed{.kernel = value});
    }
    template <class ValueT>
        requires SeedConstructibleFromKernelValue<Seed, ValueT>
    LazyPtr<Asset> retrieve_asset_k(ValueT &&value) {
        return retrieve_asset(Seed{.kernel = std::forward<ValueT>(value)});
    }

    /**
     * @brief Attempts to unload not-firmly-referenced assets to free memory
     * @param bytenum the number of bytes to attempt to free from memory
     */
    virtual void clean(std::size_t bytenum) = 0;
};
} // namespace dynasma

#endif // INCLUDED_DYNASMA_CACHER_ABSTRACT_H