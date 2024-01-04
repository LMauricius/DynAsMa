#pragma once
#ifndef INCLUDED_DYNASMA_MAN_ABSTRACT_H
#define INCLUDED_DYNASMA_MAN_ABSTRACT_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/pointer.hpp"
#include "dynasma/util/helpful_concepts.hpp"

namespace dynasma {

template <SeedLike Seed> class AbstractManager {

  public:
    virtual ~AbstractManager(){};

    /**
     * @brief Registers a seed for constructing an instance of Seed::Asset
     * @param seed an object containing the `kernel` member whose value will be
     * passed to the Seed::Asset class' constructor
     * @returns A WeakPtr to the (to-be-)constructed asset. Cast it to StrongPtr
     * to retrieve the asset
     */
    virtual WeakPtr<typename Seed::Asset> register_asset(const Seed &seed) {
        return register_asset(Seed(seed));
    }
    virtual WeakPtr<typename Seed::Asset> register_asset(Seed &&seed) {
        return register_asset((const Seed &)seed);
    }

    /**
     * @brief Attempts to unload not-strongly-referenced assets to free memory
     * @param bytenum the number of bytes to attempt to free from memory
     */
    virtual void clean(std::size_t bytenum) = 0;
};
} // namespace dynasma

#endif // INCLUDED_DYNASMA_MAN_ABSTRACT_H