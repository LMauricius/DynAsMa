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
template <class T> class PinPtr {
    friend std::hash<PinPtr>;

    template <class O> friend class FirmPtr;
    template <class O> friend class PinPtr;

    // type-erased reference counter.
    using RefCtr = PolymorphicReferenceCounter;

    RefCtr *m_p_ctr;
    T *m_p_object;

    // Internal constructor for when we know both the counter and the object
    // The object must be owned by the ctr, otherwise causes U.B.
    PinPtr(RefCtr &ctr, T *p_object) : m_p_ctr(&ctr), m_p_object(p_object) {
        // still need to reference count
        m_p_ctr->hold();
    }

  public:
    // Default constructor
    PinPtr() : m_p_ctr(nullptr) {}

    // Internal constructor for managers
    // The ctr must produce instances derived from T, otherwise causes U.B.
    PinPtr(RefCtr &ctr)
        : m_p_ctr(&ctr),
          m_p_object(
              // assume the casting is possible, i.e. we have a valid pointer
              // can be dereferenced -> is not nullptr
              // this could be used to optimize for non-virtual inheritances
              &*dynamic_cast<T *>(&ctr.hold())) {}

    // Copy & move constructors for PinPtr

    // const PinPtr<O> &
    PinPtr(const PinPtr<T> &other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    PinPtr(const PinPtr<O> &other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    PinPtr(const PinPtr<O> &other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(&*dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr->hold();
    }

    // PinPtr<O> &&
    PinPtr(PinPtr<T> &&other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    PinPtr(PinPtr<O> &&other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    PinPtr(PinPtr<O> &&other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr = nullptr;
    }

    // Copy & move constructors for subobjects

    // const PinPtr<O> &
    template <class M>
    PinPtr(const PinPtr<M> &other, T *p_subobject)
        : m_p_ctr(other.m_p_ctr), m_p_object(p_subobject) {
        other.m_p_ctr->hold();
    }

    template <class O, class M>
    PinPtr(const PinPtr<M> &other, O *p_subobject)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(p_subobject) {
        other.m_p_ctr->hold();
    }

    template <class O, class M>
    PinPtr(const PinPtr<M> &other, O *p_subobject)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(&*dynamic_cast<T *>(p_subobject)) {
        other.m_p_ctr->hold();
    }

    // PinPtr<O> &&
    template <class M>
    PinPtr(PinPtr<M> &&other, T *p_subobject)
        : m_p_ctr(other.m_p_ctr), m_p_object(p_subobject) {
        other.m_p_ctr = nullptr;
    }

    template <class O, class M>
    PinPtr(PinPtr<M> &&other, O *p_subobject)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(p_subobject) {
        other.m_p_ctr = nullptr;
    }

    template <class O, class M>
    PinPtr(PinPtr<M> &&other, O *p_subobject)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(dynamic_cast<T *>(p_subobject)) {
        other.m_p_ctr = nullptr;
    }

    // Copy & move constructor for FirmPtr

    // const FirmPtr<O> &
    PinPtr(const FirmPtr<T> &other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    PinPtr(const FirmPtr<O> &other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    PinPtr(const FirmPtr<O> &other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(&*dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr->hold();
    }

    // FirmPtr<O> &&
    PinPtr(FirmPtr<T> &&other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    PinPtr(FirmPtr<O> &&other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    PinPtr(FirmPtr<O> &&other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr = nullptr;
    }

    // Destructor

    ~PinPtr() {
        if (m_p_ctr)
            m_p_ctr->release();
    }

    // copy & move assignment for PinPtr

    // const PinPtr<O> &
    PinPtr &operator=(const PinPtr<T> &other) {
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
    PinPtr &operator=(const PinPtr<O> &other)
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
    PinPtr &operator=(const PinPtr<O> &other)
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

    // PinPtr<O> &&
    PinPtr &operator=(PinPtr<T> &&other) {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_object = other.m_p_object;

        return *this;
    }

    template <class O>
    PinPtr &operator=(PinPtr<O> &&other)
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
    PinPtr &operator=(PinPtr<O> &&other)
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
    PinPtr &operator=(const FirmPtr<T> &other) {
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
    PinPtr &operator=(const FirmPtr<O> &other)
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
    PinPtr &operator=(const FirmPtr<O> &other)
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
    PinPtr &operator=(FirmPtr<T> &&other) {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_object = other.m_p_object;

        return *this;
    }

    template <class O>
    PinPtr &operator=(FirmPtr<O> &&other)
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
    PinPtr &operator=(FirmPtr<O> &&other)
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
    PinPtr &operator=(const LazyPtr<T> &other) {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_object = &*dynamic_cast<T *>(&m_p_ctr->hold());

        return *this;
    }

    // LazyPtr&<O> &
    template <class O>
    PinPtr &operator=(const LazyPtr<O> &other)
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

    template <class O> bool operator==(const PinPtr<O> &other) const {
        return (void *)this->m_p_object == (void *)other.m_p_object;
    }
    template <class O> auto operator<=>(const PinPtr<O> &other) const {
        return (void *)this->m_p_object <=> (void *)other.m_p_object;
    }

    // Dereferencing

    T &operator*() const { return *m_p_object; }
    T *operator->() const { return m_p_object; }

    // Pointer casting functions

    template <class To, class Fr>
    friend PinPtr<To> static_pointer_cast(const PinPtr<Fr> &);
    template <class To, class Fr>
    friend PinPtr<To> static_pointer_cast(PinPtr<Fr> &&);
    template <class To, class Fr>
    friend PinPtr<To> dynamic_pointer_cast(const PinPtr<Fr> &);
    template <class To, class Fr>
    friend PinPtr<To> dynamic_pointer_cast(PinPtr<Fr> &&);
    template <class To, class Fr>
    friend PinPtr<To> const_pointer_cast(const PinPtr<Fr> &);
    template <class To, class Fr>
    friend PinPtr<To> const_pointer_cast(PinPtr<Fr> &&);
    template <class To, class Fr>
    friend PinPtr<To> reinterpret_pointer_cast(const PinPtr<Fr> &);
    template <class To, class Fr>
    friend PinPtr<To> reinterpret_pointer_cast(PinPtr<Fr> &&);
};

template <class To, class From>
PinPtr<To> static_pointer_cast(const PinPtr<From> &from) {
    return PinPtr<To>(*from.m_p_ctr, static_cast<To *>(from.m_p_object));
}
template <class To, class From>
PinPtr<To> static_pointer_cast(PinPtr<From> &&from) {
    auto ret = PinPtr<To>(*from.m_p_ctr, static_cast<To *>(from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}
template <class To, class From>
PinPtr<To> dynamic_pointer_cast(const PinPtr<From> &from) {
    // we cast the reference, not a pointer, so it throws on errors
    return PinPtr<To>(*from.m_p_ctr, &dynamic_cast<To &>(*from.m_p_object));
}
template <class To, class From>
PinPtr<To> dynamic_pointer_cast(PinPtr<From> &&from) {
    // we cast the reference, not a pointer, so it throws on errors
    auto ret = PinPtr<To>(*from.m_p_ctr, &dynamic_cast<To &>(*from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}
template <class To, class From>
PinPtr<To> const_pointer_cast(const PinPtr<From> &from) {
    return PinPtr<To>(*from.m_p_ctr, const_cast<To *>(from.m_p_object));
}
template <class To, class From>
PinPtr<To> const_pointer_cast(PinPtr<From> &&from) {
    auto ret = PinPtr<To>(*from.m_p_ctr, const_cast<To *>(from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}
template <class To, class From>
PinPtr<To> reinterpret_pointer_cast(const PinPtr<From> &from) {
    return PinPtr<To>(*from.m_p_ctr, reinterpret_cast<To *>(from.m_p_object));
}
template <class To, class From>
PinPtr<To> reinterpret_pointer_cast(PinPtr<From> &&from) {
    auto ret =
        PinPtr<To>(*from.m_p_ctr, reinterpret_cast<To *>(from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}

} // namespace dynasma

namespace std {
template <class T> struct hash<dynasma::PinPtr<T>> {
    size_t operator()(const dynasma::PinPtr<T> &x) const {
        return (size_t)x.m_p_ctr;
    }
};
} // namespace std

#endif // INCLUDED_DYNASMA_SHARED_POINTER_H