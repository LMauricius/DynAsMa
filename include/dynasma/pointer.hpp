#pragma once
#ifndef INCLUDED_DYNASMA_POINTER_H
#define INCLUDED_DYNASMA_POINTER_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/util/ref_management.hpp"

#include <type_traits>

namespace dynasma {

template <AssetLike Asset> class StrongPtr;

/**
 * @brief A weak reference to an asset. Doesn't ensure the asset is loaded.
 * @note must be cast to a StrongPtr to access the asset.
 */
template <AssetLike Asset> class WeakPtr {
    using DecAsset = std::decay_t<Asset>;
    using MutAsset = std::remove_const_t<Asset>;
    using Manager = ReferenceCounter<DecAsset>;

    friend class StrongPtr<DecAsset>;
    friend class StrongPtr<MutAsset>;

    Manager *m_p_manager;

  public:
    WeakPtr() = delete;
    // standalone
    WeakPtr(Manager &manager) : m_p_manager(&manager) {
        manager.weak_hold();
    } // WeakPtr<Asset> &
    WeakPtr(const WeakPtr<Asset> &other) : WeakPtr(*other.m_p_manager) {}
    // WeakPtr<MutAsset> &
    WeakPtr(const WeakPtr<MutAsset> &other)
        requires ConstQualified<Asset>
        : WeakPtr(*other.m_p_manager) {}
    // WeakPtr<Asset>&&
    WeakPtr(WeakPtr<Asset> &&other) : m_p_manager(other.m_p_manager) {
        other->m_p_manager = nullptr;
    }
    // WeakPtr<MutAsset>&&
    WeakPtr(WeakPtr<MutAsset> &&other)
        requires ConstQualified<Asset>
        : m_p_manager(other.m_p_manager) {
        other->m_p_manager = nullptr;
    }

    WeakPtr(const StrongPtr<Asset> &other) : WeakPtr(*other.m_p_manager) {}
    WeakPtr(const StrongPtr<MutAsset> &other)
        requires ConstQualified<Asset>
        : WeakPtr(*other.m_p_manager) {}

    ~WeakPtr() {
        if (m_p_manager)
            m_p_manager->weak_release();
    }

    // WeakPtr&<Asset> &
    WeakPtr &operator=(const WeakPtr<Asset> &other) {
        if (m_p_manager)
            m_p_manager.weak_release();

        m_p_manager = other.m_p_manager;

        if (m_p_manager)
            m_p_manager.weak_take();

        return *this;
    }
    // WeakPtr<MutAsset> &
    WeakPtr &operator=(const WeakPtr<MutAsset> &other)
        requires ConstQualified<Asset>
    {
        if (m_p_manager)
            m_p_manager.weak_release();

        m_p_manager = other.m_p_manager;

        if (m_p_manager)
            m_p_manager.weak_take();

        return *this;
    }
    // WeakPtr<Asset>&&
    WeakPtr &operator=(WeakPtr<Asset> &&other) {
        m_p_manager = other.m_p_manager;
        other->m_p_manager = nullptr;

        return *this;
    }
    // WeakPtr<MutAsset>&&
    WeakPtr &operator=(WeakPtr<MutAsset> &&other)
        requires ConstQualified<Asset>
    {
        m_p_manager = other.m_p_manager;
        other->m_p_manager = nullptr;

        return *this;
    }
};

/**
 * @brief A strong reference to an asset. Ensures the asset is loaded.
 * @note Can be used like a pointer to the asset.
 */
template <AssetLike Asset> class StrongPtr {
    using DecAsset = std::decay_t<Asset>;
    using MutAsset = std::remove_const_t<Asset>;
    using Manager = ReferenceCounter<DecAsset>;

    friend class WeakPtr<DecAsset>;
    friend class WeakPtr<MutAsset>;

    Manager *m_p_manager;
    Asset *m_p_asset;

  public:
    StrongPtr() = delete;
    // standalone
    StrongPtr(Manager &manager)
        : m_p_manager(&manager), m_p_asset(&manager.hold()) {}
    // StrongPtr<Asset> &
    StrongPtr(const StrongPtr<Asset> &other) : StrongPtr(*other.m_p_manager) {}
    // StrongPtr<MutAsset> &
    StrongPtr(const StrongPtr<MutAsset> &other)
        requires ConstQualified<Asset>
        : StrongPtr(*other.m_p_manager) {}
    // StrongPtr<Asset>&&
    StrongPtr(StrongPtr<Asset> &&other)
        : m_p_manager(other.m_p_manager), m_p_asset(other.m_p_manager) {
        other->m_p_manager = nullptr;
    }
    // StrongPtr<MutAsset>&&
    StrongPtr(StrongPtr<MutAsset> &&other)
        requires ConstQualified<Asset>
        : m_p_manager(other.m_p_manager), m_p_asset(other.m_p_manager) {
        other->m_p_manager = nullptr;
    }

    StrongPtr(const WeakPtr<Asset> &other) : StrongPtr(*other.m_p_manager) {}
    StrongPtr(const WeakPtr<MutAsset> &other)
        requires ConstQualified<Asset>
        : StrongPtr(*other.m_p_manager) {}

    ~StrongPtr() {
        if (m_p_manager)
            m_p_manager->release();
    }

    // StrongPtr&<Asset> &
    StrongPtr &operator=(const StrongPtr<Asset> &other) {
        if (m_p_manager)
            m_p_manager.release();

        m_p_manager = other.m_p_manager;

        if (m_p_manager)
            m_p_asset = m_p_manager.take();

        return *this;
    }
    // StrongPtr<MutAsset> &
    StrongPtr &operator=(const StrongPtr<MutAsset> &other)
        requires ConstQualified<Asset>
    {
        if (m_p_manager)
            m_p_manager.release();

        m_p_manager = other.m_p_manager;

        if (m_p_manager)
            m_p_asset = m_p_manager.take();

        return *this;
    }
    // StrongPtr<Asset>&&
    StrongPtr &operator=(StrongPtr<Asset> &&other) {
        m_p_manager = other.m_p_manager;
        m_p_asset = other.m_p_asset;
        other->m_p_manager = nullptr;

        return *this;
    }
    // StrongPtr<MutAsset>&&
    StrongPtr &operator=(StrongPtr<MutAsset> &&other)
        requires ConstQualified<Asset>
    {
        m_p_manager = other.m_p_manager;
        m_p_asset = other.m_p_asset;
        other->m_p_manager = nullptr;

        return *this;
    }

    Asset &operator*() const { return *m_p_asset; }
    Asset &operator->() const { return *m_p_asset; }
};

} // namespace dynasma

#endif // INCLUDED_DYNASMA_POINTER_H