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

template <class T> class FirmPtr;

/**
 * @brief A lazy reference to an object. Doesn't ensure the object is loaded.
 * @note must be cast to a FirmPtr to access the object.
 */
template <class T> class LazyPtr {
    // type-erased reference counter.
    using RefCtr = TypeErasedReferenceCounter<T>;

    template <class O> friend class LazyPtr;
    template <class O> friend class FirmPtr;

    RefCtr *m_p_ctr;

  public:
    // Default constructor
    LazyPtr() : m_p_ctr(nullptr) {}

    // Internal constructor for managers
    // The ctr must produce instances derived from T, otherwise causes U.B.
    LazyPtr(RefCtr &ctr)
        requires StandardPolymorphic<T>
        : m_p_ctr(&ctr) {
        ctr.lazy_hold();
    }

    // Copy & Move constructors for LazyPtr

    // LazyPtr<T> &
    template <class O>
    LazyPtr(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
        : LazyPtr(*other.m_p_ctr) {}

    // LazyPtr<T> &&
    template <class O>
    LazyPtr(LazyPtr<O> &&other)
        requires PointerCastable<T, O>
        : m_p_ctr(other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }

    // Copy & Move constructors for FirmPtr

    // FirmPtr<T> &
    template <class O>
    LazyPtr(const FirmPtr<O> &other)
        requires PointerCastable<T, O>
        : LazyPtr(*other.m_p_ctr) {}

    ~LazyPtr() {
        if (m_p_ctr)
            m_p_ctr->lazy_release();
    }

    // Copy & Move assignment for LazyPtr

    // LazyPtr<T> &
    template <class O>
    LazyPtr &operator=(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
    {
        if (m_p_ctr)
            m_p_ctr.lazy_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr.lazy_take();

        return *this;
    }

    // LazyPtr<T> &&
    template <class O>
    LazyPtr &operator=(LazyPtr<O> &&other)
        requires PointerCastable<T, O>
    {
        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        return *this;
    }

    // Copy & Move assignment for FirmPtr

    // FirmPtr<T> &
    template <class O>
    LazyPtr &operator=(const FirmPtr<O> &other)
        requires PointerCastable<T, O>
    {
        if (m_p_ctr)
            m_p_ctr->lazy_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr->lazy_hold();

        return *this;
    }

    /**
     * @brief Ensures the object is loaded before storing it into a FirmPtr
     * @returns a FirmPtr to the object
     */
    FirmPtr<T> getLoaded() const {
        if (m_p_ctr)
            return FirmPtr<T>(*m_p_ctr);
        else
            return FirmPtr<T>();
    }

    // Comparison operators

    template <class O> bool operator==(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr == (void *)other.m_p_ctr;
    }
    template <class O> bool operator!=(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr != (void *)other.m_p_ctr;
    }
    template <class O> bool operator<(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr < (void *)other.m_p_ctr;
    }
    template <class O> bool operator<=(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr <= (void *)other.m_p_ctr;
    }
    template <class O> bool operator>(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr > (void *)other.m_p_ctr;
    }
    template <class O> bool operator>=(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr >= (void *)other.m_p_ctr;
    }

    template <class O> bool operator==(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr == (void *)other.m_p_ctr;
    }
    template <class O> bool operator!=(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr != (void *)other.m_p_ctr;
    }
    template <class O> bool operator<(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr < (void *)other.m_p_ctr;
    }
    template <class O> bool operator<=(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr <= (void *)other.m_p_ctr;
    }
    template <class O> bool operator>(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr > (void *)other.m_p_ctr;
    }
    template <class O> bool operator>=(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr >= (void *)other.m_p_ctr;
    }
};

/**
 * @brief A firm reference to an object. Ensures the object is loaded.
 * @note Can be used like a pointer to the object.
 */
template <class T> class FirmPtr {
    // type-erased reference counter.
    using RefCtr = TypeErasedReferenceCounter<T>;

    template <class O> friend class LazyPtr;
    template <class O> friend class FirmPtr;

    RefCtr *m_p_ctr;
    T *m_p_object;

  public:
    // Default constructor
    FirmPtr() : m_p_ctr(nullptr) {}

    // Internal constructor for managers
    // The ctr must produce instances derived from T, otherwise causes U.B.
    FirmPtr(RefCtr &ctr)
        requires StandardPolymorphic<T>
        : m_p_ctr(&ctr),
          m_p_object(
              // assume the casting is possible, i.e. we have a valid pointer
              // can be dereferenced -> is not nullptr
              // this could be used to optimize for non-virtual inheritances
              &*dynamic_cast<T *>(&ctr.hold())) {}

    // Copy & move constructors for FirmPtr

    // FirmPtr<T> &
    template <class O>
    FirmPtr(const FirmPtr<O> &other)
        requires PointerCastable<T, O>
        : FirmPtr(*other.m_p_ctr) {}

    // FirmPtr<T> &&
    template <class O>
    FirmPtr(FirmPtr<O> &&other)
        requires PointerCastable<T, O>
        : m_p_ctr(other.m_p_ctr), m_p_object(dynamic_cast<T *>(other.m_p_ctr)) {
        other.m_p_ctr = nullptr;
    }

    // Copy & move constructor for LazyPtr

    // LazyPtr<T> &
    template <class O>
    FirmPtr(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
        : FirmPtr(*other.m_p_ctr) {}

    ~FirmPtr() {
        if (m_p_ctr)
            m_p_ctr->release();
    }

    // copy & move assignment for FirmPtr

    // FirmPtr<T> &
    template <class O>
    FirmPtr &operator=(const FirmPtr<O> &other)
        requires PointerCastable<T, O>
    {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_object = m_p_ctr.take();

        return *this;
    }

    // FirmPtr<T> &&
    template <class O>
    FirmPtr &operator=(FirmPtr<O> &&other)
        requires PointerCastable<T, O>
    {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_object = other.m_p_object;

        return *this;
    }

    // Copy & move assignment for LazyPtr

    // LazyPtr&<T> &
    template <class O>
    FirmPtr &operator=(const LazyPtr<T> &other)
        requires PointerCastable<T, O>
    {
        if (m_p_ctr)
            m_p_ctr.release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_object = m_p_ctr.take();

        return *this;
    }

    // Comparison operators

    template <class O> bool operator==(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr == (void *)other.m_p_ctr;
    }
    template <class O> bool operator!=(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr != (void *)other.m_p_ctr;
    }
    template <class O> bool operator<(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr < (void *)other.m_p_ctr;
    }
    template <class O> bool operator<=(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr <= (void *)other.m_p_ctr;
    }
    template <class O> bool operator>(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr > (void *)other.m_p_ctr;
    }
    template <class O> bool operator>=(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr >= (void *)other.m_p_ctr;
    }

    template <class O> bool operator==(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr == (void *)other.m_p_ctr;
    }
    template <class O> bool operator!=(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr != (void *)other.m_p_ctr;
    }
    template <class O> bool operator<(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr < (void *)other.m_p_ctr;
    }
    template <class O> bool operator<=(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr <= (void *)other.m_p_ctr;
    }
    template <class O> bool operator>(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr > (void *)other.m_p_ctr;
    }
    template <class O> bool operator>=(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr >= (void *)other.m_p_ctr;
    }

    // Dereferencing

    T &operator*() const { return *m_p_object; }
    T *operator->() const { return m_p_object; }
};

} // namespace dynasma

#endif // INCLUDED_DYNASMA_POINTER_H