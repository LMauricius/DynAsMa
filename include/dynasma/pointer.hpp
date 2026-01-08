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

template <class From, class To>
concept PointerNoCastNeeded =
    std::same_as<std::remove_cv_t<std::decay_t<From>>,
                 std::remove_cv_t<std::decay_t<To>>> &&
    MoreOrEquallyCVQualified<To, From>;

template <class From, class To>
concept PointerDynamicCastNeeded =
    !std::same_as<std::remove_cv_t<std::decay_t<From>>,
                  std::remove_cv_t<std::decay_t<To>>> &&
    std::derived_from<std::decay_t<From>, std::decay_t<To>> &&
    MoreOrEquallyCVQualified<To, From>;

template <class T>
using TypeErasedReferenceCounter = ReferenceCounter<PolymorphicBase>;

template <class T> class FirmPtr;

/**
 * @brief A lazy reference to an object. Doesn't ensure the object is loaded.
 * @note must be cast to a FirmPtr to access the object.
 */
template <class T> class LazyPtr
{
    friend std::hash<LazyPtr>;

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
    LazyPtr(RefCtr &ctr) : m_p_ctr(&ctr) { ctr.lazy_hold(); }

    // Copy & Move constructors for LazyPtr

    // LazyPtr<T> &
    LazyPtr(const LazyPtr<T> &other) : LazyPtr(*other.m_p_ctr) {}

    // LazyPtr<T> &&
    LazyPtr(LazyPtr<T> &&other) : m_p_ctr(other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }

    // LazyPtr<O> &
    template <class O>
    LazyPtr(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
        : LazyPtr(*other.m_p_ctr) {}

    // LazyPtr<O> &&
    template <class O>
    LazyPtr(LazyPtr<O> &&other)
        requires PointerCastable<T, O>
        : m_p_ctr(other.m_p_ctr) {
        other.m_p_ctr = nullptr;
    }

    // Copy & Move constructors for FirmPtr

    // FirmPtr<T> &
    LazyPtr(const FirmPtr<T> &other) : LazyPtr(*other.m_p_ctr) {}

    // FirmPtr<O> &
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
    LazyPtr &operator=(const LazyPtr<T> &other) {
        if (m_p_ctr)
            m_p_ctr->lazy_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr->lazy_hold();

        return *this;
    }

    // LazyPtr<T> &&
    LazyPtr &operator=(LazyPtr<T> &&other) {
        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        return *this;
    }

    // LazyPtr<O> &
    template <class O>
    LazyPtr &operator=(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
    {
        if (m_p_ctr)
            m_p_ctr->lazy_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr->lazy_hold();

        return *this;
    }

    // LazyPtr<O> &&
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
    LazyPtr &operator=(const FirmPtr<T> &other) {
        if (m_p_ctr)
            m_p_ctr->lazy_release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_ctr->lazy_hold();

        return *this;
    }

    // FirmPtr<O> &
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
    template <class O> auto operator<=>(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr <=> (void *)other.m_p_ctr;
    }

    template <class O> bool operator==(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr == (void *)other.m_p_ctr;
    }
    template <class O> auto operator<=>(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr <=> (void *)other.m_p_ctr;
    }
};

/**
 * @brief A firm reference to an object. Ensures the object is loaded.
 * @note Can be used like a pointer to the object.
 */
template <class T> class FirmPtr
{
    friend std::hash<FirmPtr>;

    // type-erased reference counter.
    using RefCtr = TypeErasedReferenceCounter<T>;

    template <class O> friend class LazyPtr;
    template <class O> friend class FirmPtr;
    template <class O> friend class SharedPtr;

    RefCtr *m_p_ctr;
    T *m_p_object;

    // Internal constructor for when we know both the counter and the object
    // The object must have been produced by the ctr, otherwise causes U.B.
    FirmPtr(RefCtr &ctr, T *p_object) : m_p_ctr(&ctr), m_p_object(p_object) {
        // still need to reference count
        m_p_ctr->hold();
    }

  public:
    // Default constructor
    FirmPtr() : m_p_ctr(nullptr) {}

    // Internal constructor for managers
    // The ctr must produce instances derived from T, otherwise causes U.B.
    FirmPtr(RefCtr &ctr)
        : m_p_ctr(&ctr),
          m_p_object(
              // assume the casting is possible, i.e. we have a valid pointer
              // can be dereferenced -> is not nullptr
              // this could be used to optimize for non-virtual inheritances
              &*dynamic_cast<T *>(&ctr.hold())) {}

    // Copy & move constructors for FirmPtr

    // const FirmPtr<O> &
    FirmPtr(const FirmPtr<T> &other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    FirmPtr(const FirmPtr<O> &other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    FirmPtr(const FirmPtr<O> &other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(&*dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr->hold();
    }

    // FirmPtr<O> &&
    FirmPtr(FirmPtr<T> &&other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    FirmPtr(FirmPtr<O> &&other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    FirmPtr(FirmPtr<O> &&other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr = nullptr;
    }

    // Copy & move constructor for LazyPtr

    // LazyPtr<T> &
    FirmPtr(const LazyPtr<T> &other) : FirmPtr(*other.m_p_ctr) {}

    // LazyPtr<O> &
    template <class O>
    FirmPtr(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
        : FirmPtr(*other.m_p_ctr) {}

    ~FirmPtr() {
        if (m_p_ctr)
            m_p_ctr->release();
    }

    // copy & move assignment for FirmPtr

    // const FirmPtr<O> &
    FirmPtr &operator=(const FirmPtr<T> &other) {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr) {
            m_p_object = other.m_p_object;
            m_p_ctr->hold();
        }

        return *this;
    }

    template <class O>
    FirmPtr &operator=(const FirmPtr<O> &other)
        requires PointerNoCastNeeded<O, T>
    {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr) {
            m_p_object = other.m_p_object;
            m_p_ctr->hold();
        }

        return *this;
    }

    template <class O>
    FirmPtr &operator=(const FirmPtr<O> &other)
        requires PointerDynamicCastNeeded<O, T>
    {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr) {
            m_p_object = &*dynamic_cast<T *>(other.m_p_object);
            m_p_ctr->hold();
        }

        return *this;
    }

    // FirmPtr<O> &&
    FirmPtr &operator=(FirmPtr<T> &&other) {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_object = other.m_p_object;

        return *this;
    }

    template <class O>
    FirmPtr &operator=(FirmPtr<O> &&other)
        requires PointerNoCastNeeded<O, T>
    {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_object = other.m_p_object;

        return *this;
    }

    template <class O>
    FirmPtr &operator=(FirmPtr<O> &&other)
        requires PointerDynamicCastNeeded<O, T>
    {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_object = &*dynamic_cast<T *>(other.m_p_object);

        return *this;
    }

    // Copy & move assignment for LazyPtr

    // LazyPtr&<T> &
    FirmPtr &operator=(const LazyPtr<T> &other) {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_object = &*dynamic_cast<T *>(&m_p_ctr->hold());

        return *this;
    }

    // LazyPtr&<O> &
    template <class O>
    FirmPtr &operator=(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
    {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_object = &*dynamic_cast<T *>(&m_p_ctr->hold());

        return *this;
    }

    // Comparison operators

    template <class O> bool operator==(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr == (void *)other.m_p_ctr;
    }
    template <class O> auto operator<=>(const FirmPtr<O> &other) const {
        return (void *)this->m_p_ctr <=> (void *)other.m_p_ctr;
    }

    template <class O> bool operator==(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr == (void *)other.m_p_ctr;
    }
    template <class O> auto operator<=>(const LazyPtr<O> &other) const {
        return (void *)this->m_p_ctr <=> (void *)other.m_p_ctr;
    }

    // Dereferencing

    T &operator*() const { return *m_p_object; }
    T *operator->() const { return m_p_object; }

    // Pointer casting functions

    template <class To, class Fr>
    friend FirmPtr<To> static_pointer_cast(const FirmPtr<Fr> &);
    template <class To, class Fr>
    friend FirmPtr<To> static_pointer_cast(FirmPtr<Fr> &&);
    template <class To, class Fr>
    friend FirmPtr<To> dynamic_pointer_cast(const FirmPtr<Fr> &);
    template <class To, class Fr>
    friend FirmPtr<To> dynamic_pointer_cast(FirmPtr<Fr> &&);
    template <class To, class Fr>
    friend FirmPtr<To> const_pointer_cast(const FirmPtr<Fr> &);
    template <class To, class Fr>
    friend FirmPtr<To> const_pointer_cast(FirmPtr<Fr> &&);
    template <class To, class Fr>
    friend FirmPtr<To> reinterpret_pointer_cast(const FirmPtr<Fr> &);
    template <class To, class Fr>
    friend FirmPtr<To> reinterpret_pointer_cast(FirmPtr<Fr> &&);
};

template <class To, class From>
FirmPtr<To> static_pointer_cast(const FirmPtr<From> &from) {
    return FirmPtr<To>(*from.m_p_ctr, static_cast<To *>(from.m_p_object));
}
template <class To, class From>
FirmPtr<To> static_pointer_cast(FirmPtr<From> &&from) {
    auto ret = FirmPtr<To>(*from.m_p_ctr, static_cast<To *>(from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}
template <class To, class From>
FirmPtr<To> dynamic_pointer_cast(const FirmPtr<From> &from) {
    // we cast the reference, not a pointer, so it throws on errors
    return FirmPtr<To>(*from.m_p_ctr, &dynamic_cast<To &>(*from.m_p_object));
}
template <class To, class From>
FirmPtr<To> dynamic_pointer_cast(FirmPtr<From> &&from) {
    // we cast the reference, not a pointer, so it throws on errors
    auto ret =
        FirmPtr<To>(*from.m_p_ctr, &dynamic_cast<To &>(*from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}
template <class To, class From>
FirmPtr<To> const_pointer_cast(const FirmPtr<From> &from) {
    return FirmPtr<To>(*from.m_p_ctr, const_cast<To *>(from.m_p_object));
}
template <class To, class From>
FirmPtr<To> const_pointer_cast(FirmPtr<From> &&from) {
    auto ret = FirmPtr<To>(*from.m_p_ctr, const_cast<To *>(from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}
template <class To, class From>
FirmPtr<To> reinterpret_pointer_cast(const FirmPtr<From> &from) {
    return FirmPtr<To>(*from.m_p_ctr, reinterpret_cast<To *>(from.m_p_object));
}
template <class To, class From>
FirmPtr<To> reinterpret_pointer_cast(FirmPtr<From> &&from) {
    auto ret =
        FirmPtr<To>(*from.m_p_ctr, reinterpret_cast<To *>(from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}

} // namespace dynasma

namespace std
{
template <class T> struct hash<dynasma::LazyPtr<T>>
{
    size_t operator()(const dynasma::LazyPtr<T> &x) const { return (size_t)x.m_p_ctr; }
};
template <class T> struct hash<dynasma::FirmPtr<T>>
{
    size_t operator()(const dynasma::FirmPtr<T> &x) const { return (size_t)x.m_p_ctr; }
};
} // namespace std

#endif // INCLUDED_DYNASMA_POINTER_H