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

template <AssetLike Asset> class StrongPtr;

/**
 * @brief A weak reference to an asset. Doesn't ensure the asset is loaded.
 * @note must be cast to a StrongPtr to access the asset.
 */
template <AssetLike Asset> class WeakPtr {
    // type-erased reference counter.
    using RefCtr = TypeErasedReferenceCounter<Asset>;

    template <AssetLike T> friend class WeakPtr;
    template <AssetLike T> friend class StrongPtr;

    RefCtr *m_p_ctr;

  public:
    // Default constructor
    WeakPtr() : m_p_ctr(nullptr) {}

    // Internal constructor for managers
    // The ctr must produce instances derived from Asset
    WeakPtr(RefCtr &ctr) : m_p_ctr(&ctr) { ctr.weak_hold(); }

    // Copy & Move constructors for WeakPtr

    // WeakPtr<Asset> &
    template <AssetLike OthersAsset>
    WeakPtr(const WeakPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
        : WeakPtr(*other.m_p_ctr) {}

    // WeakPtr<Asset> &&
    template <AssetLike OthersAsset>
    WeakPtr(WeakPtr<OthersAsset> &&other)
        requires PointerCastable<Asset, OthersAsset>
        : m_p_ctr(other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }

    // Copy & Move constructors for StrongPtr

    // StrongPtr<Asset> &
    template <AssetLike OthersAsset>
    WeakPtr(const StrongPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
        : WeakPtr(*other.m_p_ctr) {}

    ~WeakPtr() {
        if (m_p_ctr)
            m_p_ctr->weak_release();
    }

    // Copy & Move assignment for WeakPtr

    // WeakPtr<Asset> &
    template <AssetLike OthersAsset>
    WeakPtr &operator=(const WeakPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
    {
        if (m_p_ctr)
            m_p_ctr.weak_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr.weak_take();

        return *this;
    }

    // WeakPtr<Asset> &&
    template <AssetLike OthersAsset>
    WeakPtr &operator=(WeakPtr<OthersAsset> &&other)
        requires PointerCastable<Asset, OthersAsset>
    {
        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        return *this;
    }

    // Copy & Move assignment for StrongPtr

    // StrongPtr<Asset> &
    template <AssetLike OthersAsset>
    WeakPtr &operator=(const StrongPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
    {
        if (m_p_ctr)
            m_p_ctr.weak_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr.weak_take();

        return *this;
    }

    /**
     * @brief Ensures the asset is loaded before storing it into a StrongPtr
     * @returns a StrongPtr to the asset
     */
    StrongPtr<Asset> getLoaded() const {
        if (m_p_ctr)
            return StrongPtr<Asset>(*m_p_ctr);
        else
            return StrongPtr<Asset>();
    }
};

/**
 * @brief A strong reference to an asset. Ensures the asset is loaded.
 * @note Can be used like a pointer to the asset.
 */
template <AssetLike Asset> class StrongPtr {
    // type-erased reference counter.
    using RefCtr = TypeErasedReferenceCounter<Asset>;

    template <AssetLike T> friend class WeakPtr;
    template <AssetLike T> friend class StrongPtr;

    RefCtr *m_p_ctr;
    Asset *m_p_asset;

  public:
    // Default constructor
    StrongPtr() : m_p_ctr(nullptr) {}

    // Internal constructor for managers
    // The ctr must produce instances derived from Asset
    StrongPtr(RefCtr &ctr)
        : m_p_ctr(&ctr),
          m_p_asset(
              // assume the casting is possible, i.e. we have a valid pointer
              // can be dereferenced -> is not nullptr
              // this could be used to optimize for non-virtual inheritances
              &*dynamic_cast<Asset *>(&ctr.hold())) {}

    // Copy & move constructors for StrongPtr

    // StrongPtr<Asset> &
    template <AssetLike OthersAsset>
    StrongPtr(const StrongPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
        : StrongPtr(*other.m_p_ctr) {}

    // StrongPtr<Asset> &&
    template <AssetLike OthersAsset>
    StrongPtr(StrongPtr<OthersAsset> &&other)
        requires PointerCastable<Asset, OthersAsset>
        : m_p_ctr(other.m_p_ctr),
          m_p_asset(dynamic_cast<Asset *>(other.m_p_ctr)) {
        other.m_p_ctr = nullptr;
    }

    // Copy & move constructor for WeakPtr

    // WeakPtr<Asset> &
    template <AssetLike OthersAsset>
    StrongPtr(const WeakPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
        : StrongPtr(*other.m_p_ctr) {}

    ~StrongPtr() {
        if (m_p_ctr)
            m_p_ctr->release();
    }

    // copy & move assignment for StrongPtr

    // StrongPtr<Asset> &
    template <AssetLike OthersAsset>
    StrongPtr &operator=(const StrongPtr<OthersAsset> &other)
        requires PointerCastable<Asset, OthersAsset>
    {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_asset = m_p_ctr.take();

        return *this;
    }

    // StrongPtr<Asset> &&
    template <AssetLike OthersAsset>
    StrongPtr &operator=(StrongPtr<OthersAsset> &&other)
        requires PointerCastable<Asset, OthersAsset>
    {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_asset = other.m_p_asset;

        return *this;
    }

    // Copy & move assignment for WeakPtr

    // WeakPtr&<Asset> &
    template <AssetLike OthersAsset>
    StrongPtr &operator=(const WeakPtr<Asset> &other)
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
    Asset &operator->() const { return *m_p_asset; }
};

} // namespace dynasma

#endif // INCLUDED_DYNASMA_POINTER_H