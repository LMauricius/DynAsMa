#pragma once
#ifndef INCLUDED_DYNASMA_POINTER_H
#define INCLUDED_DYNASMA_POINTER_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/util/dynamic_typing.hpp"
#include "dynasma/util/ref_management.hpp"
#include "dynasma/util/type_modification.hpp"

#include <type_traits>

namespace dynasma {

template <class To, class From>
concept PointerCastable =
    std::derived_from<std::decay_t<From>, std::decay_t<To>> &&
    MoreOrEquallyCVQualified<To, From>;

template <class T>
using TypeErasedReferenceCounter =
    ReferenceCounter<typename CopyCV<T, PolymorphicBase>::type>;

template <AssetLike Asset> class FirmPtr;

/**
 * @brief A lazy reference to an asset. Doesn't ensure the asset is loaded.
 * @note must be cast to a FirmPtr to access the asset.
 */
template <AssetLike Asset> class LazyPtr {
    // type-erased reference counter.
    using RefCtr = TypeErasedReferenceCounter<Asset>;

    template <AssetLike T> friend class LazyPtr;
    template <AssetLike T> friend class FirmPtr;

    RefCtr *m_p_ctr;

  public:
    // Default constructor
    LazyPtr() : m_p_ctr(nullptr) {}

    // Internal constructor for managers
    // The ctr must produce instances derived from Asset
    LazyPtr(RefCtr &ctr) : m_p_ctr(&ctr) { ctr.lazy_hold(); }

    // Copy & Move constructors for LazyPtr

    // LazyPtr<Asset> &
    template <AssetLike OthersAsset>
    LazyPtr(const LazyPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
        : LazyPtr(*other.m_p_ctr) {}

    // LazyPtr<Asset> &&
    template <AssetLike OthersAsset>
    LazyPtr(LazyPtr<OthersAsset> &&other)
        requires PointerCastable<Asset, OthersAsset>
        : m_p_ctr(other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }

    // Copy & Move constructors for FirmPtr

    // FirmPtr<Asset> &
    template <AssetLike OthersAsset>
    LazyPtr(const FirmPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
        : LazyPtr(*other.m_p_ctr) {}

    ~LazyPtr() {
        if (m_p_ctr)
            m_p_ctr->lazy_release();
    }

    // Copy & Move assignment for LazyPtr

    // LazyPtr<Asset> &
    template <AssetLike OthersAsset>
    LazyPtr &operator=(const LazyPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
    {
        if (m_p_ctr)
            m_p_ctr.lazy_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr.lazy_take();

        return *this;
    }

    // LazyPtr<Asset> &&
    template <AssetLike OthersAsset>
    LazyPtr &operator=(LazyPtr<OthersAsset> &&other)
        requires PointerCastable<Asset, OthersAsset>
    {
        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        return *this;
    }

    // Copy & Move assignment for FirmPtr

    // FirmPtr<Asset> &
    template <AssetLike OthersAsset>
    LazyPtr &operator=(const FirmPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
    {
        if (m_p_ctr)
            m_p_ctr->lazy_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr->lazy_hold();

        return *this;
    }

    /**
     * @brief Ensures the asset is loaded before storing it into a FirmPtr
     * @returns a FirmPtr to the asset
     */
    FirmPtr<Asset> getLoaded() const {
        if (m_p_ctr)
            return FirmPtr<Asset>(*m_p_ctr);
        else
            return FirmPtr<Asset>();
    }
};

/**
 * @brief A firm reference to an asset. Ensures the asset is loaded.
 * @note Can be used like a pointer to the asset.
 */
template <AssetLike Asset> class FirmPtr {
    // type-erased reference counter.
    using RefCtr = TypeErasedReferenceCounter<Asset>;

    template <AssetLike T> friend class LazyPtr;
    template <AssetLike T> friend class FirmPtr;

    RefCtr *m_p_ctr;
    Asset *m_p_asset;

  public:
    // Default constructor
    FirmPtr() : m_p_ctr(nullptr) {}

    // Internal constructor for managers
    // The ctr must produce instances derived from Asset
    FirmPtr(RefCtr &ctr)
        : m_p_ctr(&ctr),
          m_p_asset(
              // assume the casting is possible, i.e. we have a valid pointer
              // can be dereferenced -> is not nullptr
              // this could be used to optimize for non-virtual inheritances
              &*dynamic_cast<Asset *>(&ctr.hold())) {}

    // Copy & move constructors for FirmPtr

    // FirmPtr<Asset> &
    template <AssetLike OthersAsset>
    FirmPtr(const FirmPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
        : FirmPtr(*other.m_p_ctr) {}

    // FirmPtr<Asset> &&
    template <AssetLike OthersAsset>
    FirmPtr(FirmPtr<OthersAsset> &&other)
        requires PointerCastable<Asset, OthersAsset>
        : m_p_ctr(other.m_p_ctr),
          m_p_asset(dynamic_cast<Asset *>(other.m_p_ctr)) {
        other.m_p_ctr = nullptr;
    }

    // Copy & move constructor for LazyPtr

    // LazyPtr<Asset> &
    template <AssetLike OthersAsset>
    FirmPtr(const LazyPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
        : FirmPtr(*other.m_p_ctr) {}

    ~FirmPtr() {
        if (m_p_ctr)
            m_p_ctr->release();
    }

    // copy & move assignment for FirmPtr

    // FirmPtr<Asset> &
    template <AssetLike OthersAsset>
    FirmPtr &operator=(const FirmPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
    {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_asset = m_p_ctr.take();

        return *this;
    }

    // FirmPtr<Asset> &&
    template <AssetLike OthersAsset>
    FirmPtr &operator=(FirmPtr<OthersAsset> &&other)
        requires PointerCastable<Asset, OthersAsset>
    {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_asset = other.m_p_asset;

        return *this;
    }

    // Copy & move assignment for LazyPtr

    // LazyPtr&<Asset> &
    template <AssetLike OthersAsset>
    FirmPtr &operator=(const LazyPtr<Asset> &other)
        requires PointerCastable<Asset, OthersAsset>
    {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_asset = m_p_ctr.take();

        return *this;
    }

    Asset &operator*() const { return *m_p_asset; }
    Asset *operator->() const { return m_p_asset; }
};

} // namespace dynasma

#endif // INCLUDED_DYNASMA_POINTER_H