#pragma once
#ifndef INCLUDED_DYNASMA_POINTER_H
#define INCLUDED_DYNASMA_POINTER_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/util/cast.hpp"
#include "dynasma/util/dynamic_typing.hpp"
#include "dynasma/util/optional.hpp"
#include "dynasma/util/ref_management.hpp"

#include <functional>
#include <optional>
#include <type_traits>

namespace dynasma {

template <class T> class FirmPtr;
template <class PtrT> class OptionalPtrBase;

/**
 * @brief A lazy reference to an object. Doesn't ensure the object is loaded.
 * @note must be cast to a FirmPtr to access the object.
 */
template <class T> class LazyPtr
{
    friend std::hash<LazyPtr>;

    // type-erased reference counter.
    using RefCtr = PolymorphicReferenceCounter;

    template <class O> friend class LazyPtr;
    template <class O> friend class FirmPtr;
    friend class OptionalPtrBase<LazyPtr<T>>;

    RefCtr *m_p_ctr;

    /// Used internally for common initialization. Sets the members and counts
    void initialize_n_hold(RefCtr *p_ctr) {
        m_p_ctr = p_ctr;
        p_ctr->lazy_hold();
    }

    /// Used internally for making it into 'moved from' state
    void move_from() {
        m_p_ctr = &internal::NULL_REF_CTR;
        internal::NULL_REF_CTR.lazy_hold();
    }

    /// Used internally for common assignment. Sets the members and counts
    LazyPtr &copy_assign(RefCtr *p_ctr) {
        p_ctr->lazy_hold();
        m_p_ctr->lazy_release();
        m_p_ctr = p_ctr;
        return *this;
    }

    /// Used internally for common assignment. Sets the members and counts
    void move_assign(RefCtr *p_ctr) {
        m_p_ctr->lazy_release();
        m_p_ctr = p_ctr;
    }

  public:
    // Internal constructor for managers
    // The ctr must produce instances derived from T, otherwise causes U.B.
    /// @warning Doesn't increase the reference count, must be done manually
    LazyPtr(RefCtr &ctr) : m_p_ctr(&ctr) {}

    // Constructor for casting raw pointers to ConvertibleToPtr objects
    template <class O>
    LazyPtr(O *p_object)
        requires RawPointerCastable<T, O>
        : m_p_ctr(p_object->m_p_counter) {
        m_p_ctr->lazy_hold();
    }

    // Copy & Move constructors for LazyPtr

    // LazyPtr<T> &
    LazyPtr(const LazyPtr<T> &other) { initialize_n_hold(other.m_p_ctr); }

    // LazyPtr<T> &&
    LazyPtr(LazyPtr<T> &&other) : m_p_ctr(other.m_p_ctr) { other.move_from(); }

    // LazyPtr<O> &
    template <class O>
    LazyPtr(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
    {
        initialize_n_hold(other.m_p_ctr);
    }

    // LazyPtr<O> &&
    template <class O>
    LazyPtr(LazyPtr<O> &&other)
        requires PointerCastable<T, O>
        : m_p_ctr(other.m_p_ctr) {
        other.move_from();
    }

    // Copy & Move constructors for FirmPtr

    // FirmPtr<T> &
    LazyPtr(const FirmPtr<T> &other) { initialize_n_hold(other.m_p_ctr); }

    // FirmPtr<O> &
    template <class O>
    LazyPtr(const FirmPtr<O> &other)
        requires PointerCastable<T, O>
    {
        initialize_n_hold(other.m_p_ctr);
    }

    ~LazyPtr() { m_p_ctr->lazy_release(); }

    // Copy & Move assignment for LazyPtr

    // LazyPtr<T> &
    LazyPtr &operator=(const LazyPtr<T> &other) {
        return copy_assign(other.m_p_ctr);
    }

    // LazyPtr<T> &&
    LazyPtr &operator=(LazyPtr<T> &&other) {
        move_assign(other.m_p_ctr);
        other.move_from();
        return *this;
    }

    // LazyPtr<O> &
    template <class O>
    LazyPtr &operator=(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
    {
        return copy_assign(other.m_p_ctr);
    }

    // LazyPtr<O> &&
    template <class O>
    LazyPtr &operator=(LazyPtr<O> &&other)
        requires PointerCastable<T, O>
    {
        move_assign(other.m_p_ctr);
        other.move_from();
        return *this;
    }

    // Copy & Move assignment for FirmPtr

    // FirmPtr<T> &
    LazyPtr &operator=(const FirmPtr<T> &other) {
        return copy_assign(other.m_p_ctr);
    }

    // FirmPtr<O> &
    template <class O>
    LazyPtr &operator=(const FirmPtr<O> &other)
        requires PointerCastable<T, O>
    {
        return copy_assign(other.m_p_ctr);
    }

    /**
     * @brief Ensures the object is loaded before storing it into a FirmPtr
     * @returns a FirmPtr to the object
     */
    FirmPtr<T> getLoaded() const {
        m_p_ctr->hold();
        return FirmPtr<T>(*m_p_ctr);
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
    using RefCtr = PolymorphicReferenceCounter;

    template <class O> friend class LazyPtr;
    template <class O> friend class FirmPtr;
    template <class O> friend class PinPtr;
    friend class OptionalPtrBase<FirmPtr<T>>;

    RefCtr *m_p_ctr;
    T *m_p_object;

    /// Used internally for common initialization. Sets the members and counts
    void initialize_n_hold(RefCtr *p_ctr, T *p_object) {
        m_p_ctr = p_ctr;
        m_p_object = p_object;
        p_ctr->hold();
    }

    /// Used internally for common initialization. Sets the members and counts
    void initialize_n_hold(RefCtr *p_ctr) {
        m_p_ctr = p_ctr;
        m_p_object = p_ctr->p_get();
        p_ctr->hold();
    }

    /// Used internally for making it into 'moved from' state
    void move_from() {
        m_p_ctr = &internal::NULL_REF_CTR;
        internal::NULL_REF_CTR.hold();
    }

    /// Used internally for common assignment. Sets the members and counts
    FirmPtr &copy_assign(RefCtr *p_ctr, T *p_object) {
        p_ctr->hold();
        m_p_ctr->release();
        m_p_ctr = p_ctr;
        m_p_object = p_object;
        return *this;
    }

    /// Used internally for common assignment. Sets the members and counts
    FirmPtr &copy_assign(RefCtr *p_ctr) {
        p_ctr->hold();
        m_p_ctr->release();
        m_p_ctr = p_ctr;
        m_p_object = p_ctr->p_get();
        return *this;
    }

    /// Used internally for common assignment. Sets the members and counts
    void move_assign(RefCtr *p_ctr, T *p_object) {
        m_p_ctr = p_ctr;
        m_p_object = p_object;
        p_ctr->hold();
    }

    /// Used internally for common assignment. Sets the members and counts
    void move_assign(RefCtr *p_ctr) {
        m_p_ctr = p_ctr;
        m_p_object = p_ctr->p_get();
        p_ctr->hold();
    }

  public:
    /// Internal constructor for managers
    /// The ctr must produce instances derived from T, otherwise causes U.B.
    /// @warning Doesn't increase the reference count, must be done manually
    FirmPtr(RefCtr &ctr) : m_p_ctr(&ctr) {
        m_p_object = internal::assume_dynamic_cast<T *>(ctr.p_get());
    }

    // Constructor for casting raw pointers to ConvertibleToPtr objects
    template <class O>
    FirmPtr(O *p_object)
        requires RawPointerCastable<T, O>
        : m_p_ctr(p_object->m_p_counter), m_p_object(p_object) {
        // still need to reference count
        m_p_ctr->hold();
    }

    // Copy & move constructors for FirmPtr

    // const FirmPtr<O> &
    FirmPtr(const FirmPtr<T> &other) {
        initialize_n_hold(other.m_p_ctr, other.m_p_object);
    }

    template <class O>
    FirmPtr(const FirmPtr<O> &other)
        requires PointerCastable<O, T>
    {
        initialize_n_hold(other.m_p_ctr,
                          internal::assume_convert<T>(other.m_p_object));
    }

    // FirmPtr<O> &&
    FirmPtr(FirmPtr<T> &&other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.move_from();
    }

    template <class O>
    FirmPtr(FirmPtr<O> &&other)
        requires PointerCastable<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(internal::assume_convert<T>(other.m_p_object)) {
        other.move_from();
    }

    // Copy & move constructor for LazyPtr

    // LazyPtr<T> &
    FirmPtr(const LazyPtr<T> &other) { initialize_n_hold(other.m_p_ctr); }

    // LazyPtr<O> &
    template <class O>
    FirmPtr(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
    {
        initialize_n_hold(other.m_p_ctr);
    }

    ~FirmPtr() { m_p_ctr->release(); }

    // copy & move assignment for FirmPtr

    // const FirmPtr<O> &
    FirmPtr &operator=(const FirmPtr<T> &other) {
        return copy_assign(other.m_p_ctr, other.m_p_object);
    }

    template <class O>
    FirmPtr &operator=(const FirmPtr<O> &other)
        requires PointerCastable<O, T>
    {
        return copy_assign(other.m_p_ctr,
                           internal::assume_convert<T>(other.m_p_object));
    }

    // FirmPtr<O> &&
    FirmPtr &operator=(FirmPtr<T> &&other) {
        move_assign(other.m_p_ctr, other.m_p_object);
        other.move_from();
        return *this;
    }

    template <class O>
    FirmPtr &operator=(FirmPtr<O> &&other)
        requires PointerCastable<O, T>
    {
        move_assign(other.m_p_ctr,
                    internal::assume_convert<T>(other.m_p_object));
        other.move_from();
        return *this;
    }

    // Copy & move assignment for LazyPtr

    // LazyPtr&<T> &
    FirmPtr &operator=(const LazyPtr<T> &other) {
        return copy_assign(other.m_p_ctr, internal::assume_dynamic_cast<T *>(
                                              other.m_p_ctr->p_get()));
    }

    // LazyPtr&<O> &
    template <class O>
    FirmPtr &operator=(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
    {
        return copy_assign(other.m_p_ctr, internal::assume_dynamic_cast<T *>(
                                              other.m_p_ctr->p_get()));
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
    from.m_p_ctr->hold();
    return FirmPtr<To>(*from.m_p_ctr, static_cast<To *>(from.m_p_object));
}
template <class To, class From>
FirmPtr<To> static_pointer_cast(FirmPtr<From> &&from) {
    auto ret = FirmPtr<To>(*from.m_p_ctr, static_cast<To *>(from.m_p_object));
    from.move_from();
    return ret;
}
template <class To, class From>
FirmPtr<To> dynamic_pointer_cast(const FirmPtr<From> &from) {
    from.m_p_ctr->hold();
    // we cast the reference, not a pointer, so it throws on errors
    return FirmPtr<To>(*from.m_p_ctr, &dynamic_cast<To &>(*from.m_p_object));
}
template <class To, class From>
FirmPtr<To> dynamic_pointer_cast(FirmPtr<From> &&from) {
    // we cast the reference, not a pointer, so it throws on errors
    auto ret =
        FirmPtr<To>(*from.m_p_ctr, &dynamic_cast<To &>(*from.m_p_object));
    from.move_from();
    return ret;
}
template <class To, class From>
FirmPtr<To> const_pointer_cast(const FirmPtr<From> &from) {
    from.m_p_ctr->hold();
    return FirmPtr<To>(*from.m_p_ctr, const_cast<To *>(from.m_p_object));
}
template <class To, class From>
FirmPtr<To> const_pointer_cast(FirmPtr<From> &&from) {
    auto ret = FirmPtr<To>(*from.m_p_ctr, const_cast<To *>(from.m_p_object));
    from.move_from();
    return ret;
}
template <class To, class From>
FirmPtr<To> reinterpret_pointer_cast(const FirmPtr<From> &from) {
    from.m_p_ctr->hold();
    return FirmPtr<To>(*from.m_p_ctr, reinterpret_cast<To *>(from.m_p_object));
}
template <class To, class From>
FirmPtr<To> reinterpret_pointer_cast(FirmPtr<From> &&from) {
    auto ret =
        FirmPtr<To>(*from.m_p_ctr, reinterpret_cast<To *>(from.m_p_object));
    from.move_from();
    return ret;
}

} // namespace dynasma

// Specializations
namespace std
{
template <class T> struct hash<dynasma::LazyPtr<T>>
{
    size_t operator()(const dynasma::LazyPtr<T> &x) const { return (size_t)x.m_p_ctr; }
};

template <class T>
class optional<dynasma::LazyPtr<T>>
    : public dynasma::OptionalPtrBase<dynasma::LazyPtr<T>> {
    using Base = dynasma::OptionalPtrBase<dynasma::LazyPtr<T>>;

  public:
    using value_type = dynasma::LazyPtr<T>;
    using Base::Base;
    using Base::operator=;

    optional() = default;
    optional(const optional &) = default;
    optional(optional &&) = default;
    optional &operator=(const optional &) = default;
    optional &operator=(optional &&) = default;
};

template <class T> struct hash<dynasma::FirmPtr<T>> {
    size_t operator()(const dynasma::FirmPtr<T> &x) const {
        return (size_t)x.m_p_ctr;
    }
};

template <class T>
class optional<dynasma::FirmPtr<T>>
    : public dynasma::OptionalPtrBase<dynasma::FirmPtr<T>> {
    using Base = dynasma::OptionalPtrBase<dynasma::FirmPtr<T>>;

  public:
    using value_type = dynasma::FirmPtr<T>;
    using Base::Base;
    using Base::operator=;

    optional() = default;
    optional(const optional &) = default;
    optional(optional &&) = default;
    optional &operator=(const optional &) = default;
    optional &operator=(optional &&) = default;
};
} // namespace std

#endif // INCLUDED_DYNASMA_POINTER_H