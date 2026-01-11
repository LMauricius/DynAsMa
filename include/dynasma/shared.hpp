#pragma once
#ifndef INCLUDED_DYNASMA_SHARED_POINTER_H
#define INCLUDED_DYNASMA_SHARED_POINTER_H

#include "dynasma/pointer.hpp"

#include <type_traits>

namespace dynasma {

/**
 * @brief A firm reference to an object whose lifetime can be shared with other
 * objects. Ensures the object is loaded. Unlike FirmPtr cannot be cast to a
 * LazyPtr
 * @note Can be used to point to members of or data owned by an object
 * controlled through a FirmPtr
 */
template <class T> class IndirectPtr {
    friend std::hash<IndirectPtr>;

    // type-erased reference counter.
    using RefCtr = TypeErasedReferenceCounter<T>;

    RefCtr *m_p_ctr;
    T *m_p_object;

    // Internal constructor for when we know both the counter and the object
    // The object must be owned by the ctr, otherwise causes U.B.
    IndirectPtr(RefCtr &ctr, T *p_object)
        : m_p_ctr(&ctr), m_p_object(p_object) {
        // still need to reference count
        m_p_ctr->hold();
    }

  public:
    // Default constructor
    IndirectPtr() : m_p_ctr(nullptr) {}

    // Internal constructor for managers
    // The ctr must produce instances derived from T, otherwise causes U.B.
    IndirectPtr(RefCtr &ctr)
        : m_p_ctr(&ctr),
          m_p_object(
              // assume the casting is possible, i.e. we have a valid pointer
              // can be dereferenced -> is not nullptr
              // this could be used to optimize for non-virtual inheritances
              &*dynamic_cast<T *>(&ctr.hold())) {}

    // Copy & move constructors for IndirectPtr

    // const IndirectPtr<O> &
    IndirectPtr(const IndirectPtr<T> &other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    IndirectPtr(const IndirectPtr<O> &other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    IndirectPtr(const IndirectPtr<O> &other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(&*dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr->hold();
    }

    // IndirectPtr<O> &&
    IndirectPtr(IndirectPtr<T> &&other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    IndirectPtr(IndirectPtr<O> &&other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    IndirectPtr(IndirectPtr<O> &&other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr = nullptr;
    }

    // Copy & move constructors for subobjects

    // const IndirectPtr<O> &
    template <class M>
    IndirectPtr(const IndirectPtr<M> &other, T *p_subobject)
        : m_p_ctr(other.m_p_ctr), m_p_object(p_subobject) {
        other.m_p_ctr->hold();
    }

    template <class O, class M>
    IndirectPtr(const IndirectPtr<M> &other, O *p_subobject)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(p_subobject) {
        other.m_p_ctr->hold();
    }

    template <class O, class M>
    IndirectPtr(const IndirectPtr<M> &other, O *p_subobject)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(&*dynamic_cast<T *>(p_subobject)) {
        other.m_p_ctr->hold();
    }

    // IndirectPtr<O> &&
    template <class M>
    IndirectPtr(IndirectPtr<M> &&other, T *p_subobject)
        : m_p_ctr(other.m_p_ctr), m_p_object(p_subobject) {
        other.m_p_ctr = nullptr;
    }

    template <class O, class M>
    IndirectPtr(IndirectPtr<M> &&other, O *p_subobject)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(p_subobject) {
        other.m_p_ctr = nullptr;
    }

    template <class O, class M>
    IndirectPtr(IndirectPtr<M> &&other, O *p_subobject)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(dynamic_cast<T *>(p_subobject)) {
        other.m_p_ctr = nullptr;
    }

    // Copy & move constructor for FirmPtr

    // const FirmPtr<O> &
    IndirectPtr(const FirmPtr<T> &other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    IndirectPtr(const FirmPtr<O> &other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    IndirectPtr(const FirmPtr<O> &other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(&*dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr->hold();
    }

    // FirmPtr<O> &&
    IndirectPtr(FirmPtr<T> &&other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    IndirectPtr(FirmPtr<O> &&other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    IndirectPtr(FirmPtr<O> &&other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr = nullptr;
    }

    // Destructor

    ~IndirectPtr() {
        if (m_p_ctr)
            m_p_ctr->release();
    }

    // copy & move assignment for IndirectPtr

    // const IndirectPtr<O> &
    IndirectPtr &operator=(const IndirectPtr<T> &other) {
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
    IndirectPtr &operator=(const IndirectPtr<O> &other)
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
    IndirectPtr &operator=(const IndirectPtr<O> &other)
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

    // IndirectPtr<O> &&
    IndirectPtr &operator=(IndirectPtr<T> &&other) {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_object = other.m_p_object;

        return *this;
    }

    template <class O>
    IndirectPtr &operator=(IndirectPtr<O> &&other)
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
    IndirectPtr &operator=(IndirectPtr<O> &&other)
        requires PointerDynamicCastNeeded<O, T>
    {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_object = &*dynamic_cast<T *>(other.m_p_object);

        return *this;
    }

    // copy & move assignment for FirmPtr

    // const FirmPtr<O> &
    IndirectPtr &operator=(const FirmPtr<T> &other) {
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
    IndirectPtr &operator=(const FirmPtr<O> &other)
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
    IndirectPtr &operator=(const FirmPtr<O> &other)
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
    IndirectPtr &operator=(FirmPtr<T> &&other) {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_object = other.m_p_object;

        return *this;
    }

    template <class O>
    IndirectPtr &operator=(FirmPtr<O> &&other)
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
    IndirectPtr &operator=(FirmPtr<O> &&other)
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
    IndirectPtr &operator=(const LazyPtr<T> &other) {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_object = &*dynamic_cast<T *>(&m_p_ctr->hold());

        return *this;
    }

    // LazyPtr&<O> &
    template <class O>
    IndirectPtr &operator=(const LazyPtr<O> &other)
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

    template <class O> bool operator==(const IndirectPtr<O> &other) const {
        return (void *)this->m_p_object == (void *)other.m_p_object;
    }
    template <class O> auto operator<=>(const IndirectPtr<O> &other) const {
        return (void *)this->m_p_object <=> (void *)other.m_p_object;
    }

    // Dereferencing

    T &operator*() const { return *m_p_object; }
    T *operator->() const { return m_p_object; }

    // Pointer casting functions

    template <class To, class Fr>
    friend IndirectPtr<To> static_pointer_cast(const IndirectPtr<Fr> &);
    template <class To, class Fr>
    friend IndirectPtr<To> static_pointer_cast(IndirectPtr<Fr> &&);
    template <class To, class Fr>
    friend IndirectPtr<To> dynamic_pointer_cast(const IndirectPtr<Fr> &);
    template <class To, class Fr>
    friend IndirectPtr<To> dynamic_pointer_cast(IndirectPtr<Fr> &&);
    template <class To, class Fr>
    friend IndirectPtr<To> const_pointer_cast(const IndirectPtr<Fr> &);
    template <class To, class Fr>
    friend IndirectPtr<To> const_pointer_cast(IndirectPtr<Fr> &&);
    template <class To, class Fr>
    friend IndirectPtr<To> reinterpret_pointer_cast(const IndirectPtr<Fr> &);
    template <class To, class Fr>
    friend IndirectPtr<To> reinterpret_pointer_cast(IndirectPtr<Fr> &&);
};

template <class To, class From>
IndirectPtr<To> static_pointer_cast(const IndirectPtr<From> &from) {
    return IndirectPtr<To>(*from.m_p_ctr, static_cast<To *>(from.m_p_object));
}
template <class To, class From>
IndirectPtr<To> static_pointer_cast(IndirectPtr<From> &&from) {
    auto ret =
        IndirectPtr<To>(*from.m_p_ctr, static_cast<To *>(from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}
template <class To, class From>
IndirectPtr<To> dynamic_pointer_cast(const IndirectPtr<From> &from) {
    // we cast the reference, not a pointer, so it throws on errors
    return IndirectPtr<To>(*from.m_p_ctr,
                           &dynamic_cast<To &>(*from.m_p_object));
}
template <class To, class From>
IndirectPtr<To> dynamic_pointer_cast(IndirectPtr<From> &&from) {
    // we cast the reference, not a pointer, so it throws on errors
    auto ret =
        IndirectPtr<To>(*from.m_p_ctr, &dynamic_cast<To &>(*from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}
template <class To, class From>
IndirectPtr<To> const_pointer_cast(const IndirectPtr<From> &from) {
    return IndirectPtr<To>(*from.m_p_ctr, const_cast<To *>(from.m_p_object));
}
template <class To, class From>
IndirectPtr<To> const_pointer_cast(IndirectPtr<From> &&from) {
    auto ret =
        IndirectPtr<To>(*from.m_p_ctr, const_cast<To *>(from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}
template <class To, class From>
IndirectPtr<To> reinterpret_pointer_cast(const IndirectPtr<From> &from) {
    return IndirectPtr<To>(*from.m_p_ctr,
                           reinterpret_cast<To *>(from.m_p_object));
}
template <class To, class From>
IndirectPtr<To> reinterpret_pointer_cast(IndirectPtr<From> &&from) {
    auto ret =
        IndirectPtr<To>(*from.m_p_ctr, reinterpret_cast<To *>(from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}

} // namespace dynasma

namespace std {
template <class T> struct hash<dynasma::IndirectPtr<T>> {
    size_t operator()(const dynasma::IndirectPtr<T> &x) const {
        return (size_t)x.m_p_ctr;
    }
};
} // namespace std

#endif // INCLUDED_DYNASMA_SHARED_POINTER_H