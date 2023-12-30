#pragma once
#ifndef INCLUDED_DYNASMA_REF_MAN_H
#define INCLUDED_DYNASMA_REF_MAN_H

#include "dynasma/core_concepts.hpp"

namespace dynasma {
template <AssetLike Asset> class ReferenceCounter {
    std::size_t m_strongcount;
    std::size_t m_weakcount;

  protected:
    Asset *p_asset;

    /**
     * @brief Ensures that the asset is loaded and its pointer is stored in
     * p_asset
     * @note Called only while the asset is allowed to be unloaded
     */
    virtual void ensure_loaded_impl() = 0;
    /**
     * @brief Allows unloading the asset
     * @note Called while the asset is required to be loaded
     */
    virtual void allow_unload_impl() = 0;
    /**
     * @brief Allows `this` to be deleted
     * @note this instance should not be referenced after this call
     */
    virtual void forget_impl() = 0;

  public:
    ReferenceCounter() : m_strongcount(0), m_weakcount(0), p_asset(nullptr){};
    virtual ~ReferenceCounter(){};

    /**
     * @brief Raises the strong reference count
     * @note If the asset is not loaded, it will be loaded
     * @returns reference to the loaded asset
     */
    Asset &hold() {
        if (m_strongcount == 0) {
            ensure_loaded_impl();
        }
        m_strongcount++;
        return *p_asset;
    }
    /**
     * @brief Reduces the strong reference count
     * @note If the count reaches 0, the asset can be unloaded
     */
    void release() {
        m_strongcount--;
        if (m_strongcount == 0) {
            allow_unload_impl();
        }
    }
    /**
     * @returns a pointer to the loaded asset or nullptr if the asset is
     * not loaded
     */
    Asset *p_get() { return p_asset; }

    /**
     * @brief Increases the weak reference count
     */
    void weak_hold() { m_weakcount++; }
    /**
     * @brief Reduces the weak reference count
     * @note If the count reaches 0, this instance can be deleted
     */
    void weak_release() {
        m_weakcount--;
        if (m_weakcount == 0 && m_strongcount == 0) {
            forget_impl();
        }
    }
};
} // namespace dynasma

#endif // INCLUDED_DYNASMA_REF_MAN_H