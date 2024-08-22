#pragma once
#ifndef INCLUDED_DYNASMA_KEEPER_ABSTRACT_H
#define INCLUDED_DYNASMA_KEEPER_ABSTRACT_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/pointer.hpp"
#include "dynasma/pool.hpp"
#include "dynasma/util/helpful_concepts.hpp"

namespace dynasma {

/**
 * @brief Abstract base class for keepers - simpler asset managers that
 * construct the asset immediately, unlike managers that remember a copy of the
 * seed. Keep the asset loaded until ALL references are dropped, including
 * LazyPtrs and never reload them.
 * @param Seed A SeedLike type describing everything we need to know about the
 * asset. NOT necessarily a ReloadableSeedLike
 */
template <SeedLike Seed> class AbstractKeeper : public AbstractPool {
  public:
    using Asset = typename Seed::Asset;

    virtual ~AbstractKeeper(){};

    /**
     * @brief Constructs an instance of Seed::Asset with a given seed
     * @param seed an object containing the `kernel` member whose value will be
     * passed to the Seed::Asset class' constructor
     * @returns A LazyPtr to the (to-be-)constructed asset. Cast it to FirmPtr
     * to retrieve the asset
     */
    virtual LazyPtr<Asset> new_asset(const Seed &seed) {
        return new_asset(Seed(seed));
    }
    virtual LazyPtr<Asset> new_asset(Seed &&seed) {
        return new_asset((const Seed &)seed);
    }

    /**
     * @brief Constructs an instance of Seed::Asset with a seed from the given
     * kernel value
     * @tparam ValueT the type of the kernel value
     * @param value the kernel value
     * @returns A LazyPtr to the (to-be-)constructed asset. Cast it to FirmPtr
     * to retrieve the asset
     */
    template <class ValueT>
        requires SeedConstructibleFromKernelValue<Seed, ValueT>
    LazyPtr<Asset> new_asset_k(const ValueT &value) {
        return new_asset(Seed{.kernel = value});
    }
    template <class ValueT>
        requires SeedConstructibleFromKernelValue<Seed, ValueT>
    LazyPtr<Asset> new_asset_k(ValueT &&value) {
        return new_asset(Seed{.kernel = std::forward<ValueT>(value)});
    }
};
} // namespace dynasma

#endif // INCLUDED_DYNASMA_KEEPER_ABSTRACT_H