#pragma once
#ifndef INCLUDED_DYNASMA_POOL_H
#define INCLUDED_DYNASMA_POOL_H

#include <cstddef>
#include <limits>

namespace dynasma {

/**
 * @brief An abstract class for any kind of asset pool.
 * @note Cachers, Keepers and Managers all inherit from this asset type agnostic
 * class
 */
class AbstractPool {
  public:
    /**
     * @brief Attempts to unload not-firmly-referenced assets to free memory
     * @param bytenum the number of bytes to attempt to free from memory
     * @returns the number of bytes freed (according to memory_cost() functions)
     */
    virtual std::size_t clean(std::size_t bytenum) = 0;

    /**
     * @brief Attempts to unload all not-firmly-referenced assets to free memory
     * @returns the number of bytes freed (according to memory_cost() functions)
     */
    inline std::size_t cleanAll() {
        return clean(std::numeric_limits<std::size_t>::max());
    }
};
} // namespace dynasma

#endif // INCLUDED_DYNASMA_POOL_H