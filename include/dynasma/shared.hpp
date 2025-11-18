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
template <class T> class SharedPtr {
    friend std::hash<SharedPtr>;

    // type-erased reference counter.
    using RefCtr = TypeErasedReferenceCounter<T>;

    RefCtr *m_p_ctr;
    T *m_p_object;

    // Internal constructor for when we know both the counter and the object
    // The object must be owned by the ctr, otherwise causes U.B.
    SharedPtr(RefCtr &ctr, T *p_object) : m_p_ctr(&ctr), m_p_object(p_object) {
        // still need to reference count
        m_p_ctr->hold();
    }

  public:
    // Default constructor
    SharedPtr() : m_p_ctr(nullptr) {}

    // Internal constructor for managers
    // The ctr must produce instances derived from T, otherwise causes U.B.
    SharedPtr(RefCtr &ctr)
        : m_p_ctr(&ctr),
          m_p_object(
              // assume the casting is possible, i.e. we have a valid pointer
              // can be dereferenced -> is not nullptr
              // this could be used to optimize for non-virtual inheritances
              &*dynamic_cast<T *>(&ctr.hold())) {}

    // Copy & move constructors for SharedPtr

    // const SharedPtr<O> &
    SharedPtr(const SharedPtr<T> &other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    SharedPtr(const SharedPtr<O> &other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    SharedPtr(const SharedPtr<O> &other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(&*dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr->hold();
    }

    // SharedPtr<O> &&
    SharedPtr(SharedPtr<T> &&other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    SharedPtr(SharedPtr<O> &&other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    SharedPtr(SharedPtr<O> &&other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr = nullptr;
    }

    // Copy & move constructors for subobjects

    // const SharedPtr<O> &
    template <class M>
    SharedPtr(const SharedPtr<M> &other, T *p_subobject)
        : m_p_ctr(other.m_p_ctr), m_p_object(p_subobject) {
        other.m_p_ctr->hold();
    }

    template <class O, class M>
    SharedPtr(const SharedPtr<M> &other, O *p_subobject)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(p_subobject) {
        other.m_p_ctr->hold();
    }

    template <class O, class M>
    SharedPtr(const SharedPtr<M> &other, O *p_subobject)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(&*dynamic_cast<T *>(p_subobject)) {
        other.m_p_ctr->hold();
    }

    // SharedPtr<O> &&
    template <class M>
    SharedPtr(SharedPtr<M> &&other, T *p_subobject)
        : m_p_ctr(other.m_p_ctr), m_p_object(p_subobject) {
        other.m_p_ctr = nullptr;
    }

    template <class O, class M>
    SharedPtr(SharedPtr<M> &&other, O *p_subobject)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(p_subobject) {
        other.m_p_ctr = nullptr;
    }

    template <class O, class M>
    SharedPtr(SharedPtr<M> &&other, O *p_subobject)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(dynamic_cast<T *>(p_subobject)) {
        other.m_p_ctr = nullptr;
    }

    // Copy & move constructor for FirmPtr

    // const FirmPtr<O> &
    SharedPtr(const FirmPtr<T> &other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    SharedPtr(const FirmPtr<O> &other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr->hold();
    }

    template <class O>
    SharedPtr(const FirmPtr<O> &other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(&*dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr->hold();
    }

    // FirmPtr<O> &&
    SharedPtr(FirmPtr<T> &&other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    SharedPtr(FirmPtr<O> &&other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = nullptr;
    }

    template <class O>
    SharedPtr(FirmPtr<O> &&other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr = nullptr;
    }

    // Destructor

    ~SharedPtr() {
        if (m_p_ctr)
            m_p_ctr->release();
    }

    // copy & move assignment for SharedPtr

    // const SharedPtr<O> &
    SharedPtr &operator=(const SharedPtr<T> &other) {
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
    SharedPtr &operator=(const SharedPtr<O> &other)
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
    SharedPtr &operator=(const SharedPtr<O> &other)
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

    // SharedPtr<O> &&
    SharedPtr &operator=(SharedPtr<T> &&other) {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_object = other.m_p_object;

        return *this;
    }

    template <class O>
    SharedPtr &operator=(SharedPtr<O> &&other)
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
    SharedPtr &operator=(SharedPtr<O> &&other)
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
    SharedPtr &operator=(const FirmPtr<T> &other) {
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
    SharedPtr &operator=(const FirmPtr<O> &other)
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
    SharedPtr &operator=(const FirmPtr<O> &other)
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
    SharedPtr &operator=(FirmPtr<T> &&other) {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = nullptr;

        m_p_object = other.m_p_object;

        return *this;
    }

    template <class O>
    SharedPtr &operator=(FirmPtr<O> &&other)
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
    SharedPtr &operator=(FirmPtr<O> &&other)
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
    SharedPtr &operator=(const LazyPtr<T> &other) {
        if (m_p_ctr)
            m_p_ctr->release();

        m_p_ctr = other.m_p_ctr;

        if (m_p_ctr)
            m_p_object = &*dynamic_cast<T *>(&m_p_ctr->hold());

        return *this;
    }

    // LazyPtr&<O> &
    template <class O>
    SharedPtr &operator=(const LazyPtr<O> &other)
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

    template <class O> bool operator==(const SharedPtr<O> &other) const {
        return (void *)this->m_p_object == (void *)other.m_p_object;
    }
    template <class O> auto operator<=>(const SharedPtr<O> &other) const {
        return (void *)this->m_p_object <=> (void *)other.m_p_object;
    }

    // Dereferencing

    T &operator*() const { return *m_p_object; }
    T *operator->() const { return m_p_object; }

    // Pointer casting functions

    template <class To, class Fr>
    friend SharedPtr<To> static_pointer_cast(const SharedPtr<Fr> &);
    template <class To, class Fr>
    friend SharedPtr<To> static_pointer_cast(SharedPtr<Fr> &&);
    template <class To, class Fr>
    friend SharedPtr<To> dynamic_pointer_cast(const SharedPtr<Fr> &);
    template <class To, class Fr>
    friend SharedPtr<To> dynamic_pointer_cast(SharedPtr<Fr> &&);
    template <class To, class Fr>
    friend SharedPtr<To> const_pointer_cast(const SharedPtr<Fr> &);
    template <class To, class Fr>
    friend SharedPtr<To> const_pointer_cast(SharedPtr<Fr> &&);
    template <class To, class Fr>
    friend SharedPtr<To> reinterpret_pointer_cast(const SharedPtr<Fr> &);
    template <class To, class Fr>
    friend SharedPtr<To> reinterpret_pointer_cast(SharedPtr<Fr> &&);
};

template <class To, class From>
SharedPtr<To> static_pointer_cast(const SharedPtr<From> &from) {
    return SharedPtr<To>(*from.m_p_ctr, static_cast<To *>(from.m_p_object));
}
template <class To, class From>
SharedPtr<To> static_pointer_cast(SharedPtr<From> &&from) {
    auto ret = SharedPtr<To>(*from.m_p_ctr, static_cast<To *>(from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}
template <class To, class From>
SharedPtr<To> dynamic_pointer_cast(const SharedPtr<From> &from) {
    // we cast the reference, not a pointer, so it throws on errors
    return SharedPtr<To>(*from.m_p_ctr, &dynamic_cast<To &>(*from.m_p_object));
}
template <class To, class From>
SharedPtr<To> dynamic_pointer_cast(SharedPtr<From> &&from) {
    // we cast the reference, not a pointer, so it throws on errors
    auto ret =
        SharedPtr<To>(*from.m_p_ctr, &dynamic_cast<To &>(*from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}
template <class To, class From>
SharedPtr<To> const_pointer_cast(const SharedPtr<From> &from) {
    return SharedPtr<To>(*from.m_p_ctr, const_cast<To *>(from.m_p_object));
}
template <class To, class From>
SharedPtr<To> const_pointer_cast(SharedPtr<From> &&from) {
    auto ret = SharedPtr<To>(*from.m_p_ctr, const_cast<To *>(from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}
template <class To, class From>
SharedPtr<To> reinterpret_pointer_cast(const SharedPtr<From> &from) {
    return SharedPtr<To>(*from.m_p_ctr,
                         reinterpret_cast<To *>(from.m_p_object));
}
template <class To, class From>
SharedPtr<To> reinterpret_pointer_cast(SharedPtr<From> &&from) {
    auto ret =
        SharedPtr<To>(*from.m_p_ctr, reinterpret_cast<To *>(from.m_p_object));
    from.m_p_ctr = nullptr;
    return ret;
}

} // namespace dynasma

namespace std {
template <class T> struct hash<dynasma::SharedPtr<T>> {
    size_t operator()(const dynasma::SharedPtr<T> &x) const {
        return (size_t)x.m_p_ctr;
    }
};
} // namespace std

#endif // INCLUDED_DYNASMA_SHARED_POINTER_H