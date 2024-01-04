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
    using RefCtr = ReferenceCounter<DecAsset>;

    friend class StrongPtr<DecAsset>;
    friend class StrongPtr<MutAsset>;

    RefCtr *m_p_ctr;

  public:
    // Default constructor
    WeakPtr() : m_p_ctr(nullptr) {}

    // Internal constructor for managers
    WeakPtr(RefCtr &ctr) : m_p_ctr(&ctr) { ctr.weak_hold(); }

    // Copy & Move constructors

    // WeakPtr<Asset> &
    WeakPtr(const WeakPtr<Asset> &other) : WeakPtr(*other.m_p_ctr) {}
    // WeakPtr<MutAsset> &
    WeakPtr(const WeakPtr<MutAsset> &other)
        requires ConstQualified<Asset>
        : WeakPtr(*other.m_p_ctr) {}
    // WeakPtr<Asset>&&
    WeakPtr(WeakPtr<Asset> &&other) : m_p_ctr(other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }
    // WeakPtr<MutAsset>&&
    WeakPtr(WeakPtr<MutAsset> &&other)
        requires ConstQualified<Asset>
        : m_p_ctr(other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }

    // Copy & Move constructors for StrongPtr

    WeakPtr(const StrongPtr<Asset> &other) : WeakPtr(*other.m_p_ctr) {}
    WeakPtr(const StrongPtr<MutAsset> &other)
        requires ConstQualified<Asset>
        : WeakPtr(*other.m_p_ctr) {}
    WeakPtr(StrongPtr<Asset> &&other) : WeakPtr(*other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }
    WeakPtr(StrongPtr<MutAsset> &&other)
        requires ConstQualified<Asset>
        : WeakPtr(*other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }

    ~WeakPtr() {
        if (m_p_ctr)
            m_p_ctr->weak_release();
    }

    // Copy & Move assignment

    // WeakPtr&<Asset> &
    WeakPtr &operator=(const WeakPtr<Asset> &other) {
        if (m_p_ctr)
            m_p_ctr.weak_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr.weak_take();

        return *this;
    }
    // WeakPtr<MutAsset> &
    WeakPtr &operator=(const WeakPtr<MutAsset> &other)
        requires ConstQualified<Asset>
    {
        if (m_p_ctr)
            m_p_ctr.weak_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr.weak_take();

        return *this;
    }
    // WeakPtr<Asset>&&
    WeakPtr &operator=(WeakPtr<Asset> &&other) {
        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        return *this;
    }
    // WeakPtr<MutAsset>&&
    WeakPtr &operator=(WeakPtr<MutAsset> &&other)
        requires ConstQualified<Asset>
    {
        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        return *this;
    }

    // Copy & Move assignment for StrongPtr

    // WeakPtr&<Asset> &
    WeakPtr &operator=(const StrongPtr<Asset> &other) {
        if (m_p_ctr)
            m_p_ctr.weak_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr.weak_take();

        return *this;
    }
    // StrongPtr<MutAsset> &
    WeakPtr &operator=(const StrongPtr<MutAsset> &other)
        requires ConstQualified<Asset>
    {
        if (m_p_ctr)
            m_p_ctr.weak_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr.weak_take();

        return *this;
    }
    // StrongPtr<Asset>&&
    WeakPtr &operator=(StrongPtr<Asset> &&other) {
        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        return *this;
    }
    // StrongPtr<MutAsset>&&
    WeakPtr &operator=(StrongPtr<MutAsset> &&other)
        requires ConstQualified<Asset>
    {
        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

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
    using RefCtr = ReferenceCounter<DecAsset>;

    friend class WeakPtr<DecAsset>;
    friend class WeakPtr<MutAsset>;

    RefCtr *m_p_ctr;
    Asset *m_p_asset;

  public:
    // Default constructor
    StrongPtr() : m_p_ctr(nullptr) {}

    // Internal constructor for managers
    StrongPtr(RefCtr &ctr) : m_p_ctr(&ctr), m_p_asset(&ctr.hold()) {}

    // Copy & move constructors
    // StrongPtr<Asset> &
    StrongPtr(const StrongPtr<Asset> &other) : StrongPtr(*other.m_p_ctr) {}
    // StrongPtr<MutAsset> &
    StrongPtr(const StrongPtr<MutAsset> &other)
        requires ConstQualified<Asset>
        : StrongPtr(*other.m_p_ctr) {}
    // StrongPtr<Asset>&&
    StrongPtr(StrongPtr<Asset> &&other)
        : m_p_ctr(other.m_p_ctr), m_p_asset(other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }
    // StrongPtr<MutAsset>&&
    StrongPtr(StrongPtr<MutAsset> &&other)
        requires ConstQualified<Asset>
        : m_p_ctr(other.m_p_ctr), m_p_asset(other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }

    // Copy & move constructors for WeakPtr
    StrongPtr(const WeakPtr<Asset> &other) : StrongPtr(*other.m_p_ctr) {}
    StrongPtr(const WeakPtr<MutAsset> &other)
        requires ConstQualified<Asset>
        : StrongPtr(*other.m_p_ctr) {}
    StrongPtr(WeakPtr<Asset> &&other) : StrongPtr(*other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }
    StrongPtr(WeakPtr<MutAsset> &&other)
        requires ConstQualified<Asset>
        : StrongPtr(*other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }

    ~StrongPtr() {
        if (m_p_ctr)
            m_p_ctr->release();
    }

    // StrongPtr&<Asset> &
    StrongPtr &operator=(const StrongPtr<Asset> &other) {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_asset = m_p_ctr.take();

        return *this;
    }
    // StrongPtr<MutAsset> &
    StrongPtr &operator=(const StrongPtr<MutAsset> &other)
        requires ConstQualified<Asset>
    {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_asset = m_p_ctr.take();

        return *this;
    }
    // StrongPtr<Asset>&&
    StrongPtr &operator=(StrongPtr<Asset> &&other) {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_asset = other.m_p_asset;

        return *this;
    }
    // StrongPtr<MutAsset>&&
    StrongPtr &operator=(StrongPtr<MutAsset> &&other)
        requires ConstQualified<Asset>
    {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_asset = other.m_p_asset;

        return *this;
    }

    // StrongPtr&<Asset> &
    StrongPtr &operator=(const WeakPtr<Asset> &other) {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_asset = m_p_ctr.take();

        return *this;
    }
    // WeakPtr<MutAsset> &
    StrongPtr &operator=(const WeakPtr<MutAsset> &other)
        requires ConstQualified<Asset>
    {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_asset = m_p_ctr.take();

        return *this;
    }
    // WeakPtr<Asset>&&
    StrongPtr &operator=(WeakPtr<Asset> &&other) {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_asset = &m_p_ctr.hold();

        return *this;
    }
    // WeakPtr<MutAsset>&&
    StrongPtr &operator=(WeakPtr<MutAsset> &&other)
        requires ConstQualified<Asset>
    {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_asset = &m_p_ctr.hold();

        return *this;
    }

    Asset &operator*() const { return *m_p_asset; }
    Asset &operator->() const { return *m_p_asset; }
};

} // namespace dynasma

#endif // INCLUDED_DYNASMA_POINTER_H