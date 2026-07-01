#pragma once
#include <cstddef>
#ifndef INCLUDED_DYNASMA_POINTER_H
#define INCLUDED_DYNASMA_POINTER_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/util/dynamic_typing.hpp"
#include "dynasma/util/ref_management.hpp"
#include "dynasma/util/type_modification.hpp"

#include <functional>
#include <new>
#include <optional>
#include <type_traits>
#include <utility>

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

template <class To, class From>
concept RawPointerCastable =
    PointerCastable<To, From> && RawConvertibleToPtr<From>;

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

  public:
    // Internal constructor for managers
    // The ctr must produce instances derived from T, otherwise causes U.B.
    LazyPtr(RefCtr &ctr) : m_p_ctr(&ctr) { ctr.lazy_hold(); }

    // Constructor for casting raw pointers to ConvertibleToPtr objects
    template <class O>
    LazyPtr(O *p_object)
        requires RawPointerCastable<T, O>
        : m_p_ctr(p_object->m_p_counter) {}

    // Copy & Move constructors for LazyPtr

    // LazyPtr<T> &
    LazyPtr(const LazyPtr<T> &other) : LazyPtr(*other.m_p_ctr) {}

    // LazyPtr<T> &&
    LazyPtr(LazyPtr<T> &&other) : m_p_ctr(other.m_p_ctr) {
        other.m_p_ctr = &internal::NULL_REF_CTR;
        internal::NULL_REF_CTR.lazy_hold();
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
        other.m_p_ctr = &internal::NULL_REF_CTR;
        internal::NULL_REF_CTR.lazy_hold();
    }

    // Copy & Move constructors for FirmPtr

    // FirmPtr<T> &
    LazyPtr(const FirmPtr<T> &other) : LazyPtr(*other.m_p_ctr) {}

    // FirmPtr<O> &
    template <class O>
    LazyPtr(const FirmPtr<O> &other)
        requires PointerCastable<T, O>
        : LazyPtr(*other.m_p_ctr) {}

    ~LazyPtr() { m_p_ctr->lazy_release(); }

    // Copy & Move assignment for LazyPtr

    // LazyPtr<T> &
    LazyPtr &operator=(const LazyPtr<T> &other) {
        other.m_p_ctr->lazy_hold();
        m_p_ctr->lazy_release();
        m_p_ctr = other.m_p_ctr;

        return *this;
    }

    // LazyPtr<T> &&
    LazyPtr &operator=(LazyPtr<T> &&other) {
        m_p_ctr->lazy_release();
        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = &internal::NULL_REF_CTR;
        internal::NULL_REF_CTR.lazy_hold();

        return *this;
    }

    // LazyPtr<O> &
    template <class O>
    LazyPtr &operator=(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
    {
        other.m_p_ctr->lazy_hold();
        m_p_ctr->lazy_release();
        m_p_ctr = other.m_p_ctr;

        return *this;
    }

    // LazyPtr<O> &&
    template <class O>
    LazyPtr &operator=(LazyPtr<O> &&other)
        requires PointerCastable<T, O>
    {
        m_p_ctr->lazy_release();
        m_p_ctr = other.m_p_ctr;
        other.m_p_ctr = &internal::NULL_REF_CTR;
        internal::NULL_REF_CTR.lazy_hold();

        return *this;
    }

    // Copy & Move assignment for FirmPtr

    // FirmPtr<T> &
    LazyPtr &operator=(const FirmPtr<T> &other) {
        other.m_p_ctr->lazy_hold();
        m_p_ctr->lazy_release();
        m_p_ctr = other.m_p_ctr;

        return *this;
    }

    // FirmPtr<O> &
    template <class O>
    LazyPtr &operator=(const FirmPtr<O> &other)
        requires PointerCastable<T, O>
    {
        other.m_p_ctr->lazy_hold();
        m_p_ctr->lazy_release();
        m_p_ctr = other.m_p_ctr;

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
            return FirmPtr<T>(internal::NULL_REF_CTR);
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

    // Internal constructor for when we know both the counter and the object
    // The object must have been produced by the ctr, otherwise causes U.B.
    FirmPtr(RefCtr &ctr, T *p_object) : m_p_ctr(&ctr), m_p_object(p_object) {
        // still need to reference count
        m_p_ctr->hold();
    }

  public:
    // Internal constructor for managers
    // The ctr must produce instances derived from T, otherwise causes U.B.
    FirmPtr(RefCtr &ctr) : m_p_ctr(&ctr) {
        ctr.hold();

        // assume the casting is possible, i.e. we have a valid pointer
        // can be dereferenced -> is not nullptr
        // this could be used to optimize for non-virtual inheritances
        m_p_object = &*dynamic_cast<T *>(ctr.p_get());
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
    FirmPtr(const FirmPtr<T> &other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        m_p_ctr->hold();
    }

    template <class O>
    FirmPtr(const FirmPtr<O> &other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        m_p_ctr->hold();
    }

    template <class O>
    FirmPtr(const FirmPtr<O> &other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(&*dynamic_cast<T *>(other.m_p_object)) {
        m_p_ctr->hold();
    }

    // FirmPtr<O> &&
    FirmPtr(FirmPtr<T> &&other)
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = &internal::NULL_REF_CTR;
        internal::NULL_REF_CTR.hold();
    }

    template <class O>
    FirmPtr(FirmPtr<O> &&other)
        requires PointerNoCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr), m_p_object(other.m_p_object) {
        other.m_p_ctr = &internal::NULL_REF_CTR;
        internal::NULL_REF_CTR.hold();
    }

    template <class O>
    FirmPtr(FirmPtr<O> &&other)
        requires PointerDynamicCastNeeded<O, T>
        : m_p_ctr(other.m_p_ctr),
          m_p_object(dynamic_cast<T *>(other.m_p_object)) {
        other.m_p_ctr = &internal::NULL_REF_CTR;
        internal::NULL_REF_CTR.hold();
    }

    // Copy & move constructor for LazyPtr

    // LazyPtr<T> &
    FirmPtr(const LazyPtr<T> &other) : FirmPtr(*other.m_p_ctr) {}

    // LazyPtr<O> &
    template <class O>
    FirmPtr(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
        : FirmPtr(*other.m_p_ctr) {}

    ~FirmPtr() { m_p_ctr->release(); }

    // copy & move assignment for FirmPtr

    // const FirmPtr<O> &
    FirmPtr &operator=(const FirmPtr<T> &other) {
        other.m_p_ctr->hold();
        m_p_ctr->release();
        m_p_ctr = other.m_p_ctr;
        m_p_object = other.m_p_object;

        return *this;
    }

    template <class O>
    FirmPtr &operator=(const FirmPtr<O> &other)
        requires PointerNoCastNeeded<O, T>
    {
        other.m_p_ctr->hold();
        m_p_ctr->release();
        m_p_ctr = other.m_p_ctr;
        m_p_object = other.m_p_object;

        return *this;
    }

    template <class O>
    FirmPtr &operator=(const FirmPtr<O> &other)
        requires PointerDynamicCastNeeded<O, T>
    {
        other.m_p_ctr->hold();
        m_p_ctr->release();
        m_p_ctr = other.m_p_ctr;
        m_p_object = &*dynamic_cast<T *>(other.m_p_object);

        return *this;
    }

    // FirmPtr<O> &&
    FirmPtr &operator=(FirmPtr<T> &&other) {
        m_p_ctr->release();
        m_p_ctr = other.m_p_ctr;
        m_p_object = other.m_p_object;

        other.m_p_ctr = &internal::NULL_REF_CTR;
        internal::NULL_REF_CTR.hold();

        return *this;
    }

    template <class O>
    FirmPtr &operator=(FirmPtr<O> &&other)
        requires PointerNoCastNeeded<O, T>
    {
        m_p_ctr->release();
        m_p_ctr = other.m_p_ctr;
        m_p_object = other.m_p_object;

        other.m_p_ctr = &internal::NULL_REF_CTR;
        internal::NULL_REF_CTR.hold();

        return *this;
    }

    template <class O>
    FirmPtr &operator=(FirmPtr<O> &&other)
        requires PointerDynamicCastNeeded<O, T>
    {
        m_p_ctr->release();
        m_p_ctr = other.m_p_ctr;
        m_p_object = &*dynamic_cast<T *>(other.m_p_object);

        other.m_p_ctr = &internal::NULL_REF_CTR;
        internal::NULL_REF_CTR.hold();

        return *this;
    }

    // Copy & move assignment for LazyPtr

    // LazyPtr&<T> &
    FirmPtr &operator=(const LazyPtr<T> &other) {
        other.m_p_ctr->hold();
        m_p_ctr->release();
        m_p_ctr = other.m_p_ctr;
        m_p_object = &*dynamic_cast<T *>(m_p_ctr->p_get());

        return *this;
    }

    // LazyPtr&<O> &
    template <class O>
    FirmPtr &operator=(const LazyPtr<O> &other)
        requires PointerCastable<T, O>
    {
        other.m_p_ctr->hold();
        m_p_ctr->release();
        m_p_ctr = other.m_p_ctr;
        m_p_object = &*dynamic_cast<T *>(m_p_ctr->p_get());

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
    from.m_p_ctr = &internal::NULL_REF_CTR;
    internal::NULL_REF_CTR.hold();
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
    from.m_p_ctr = &internal::NULL_REF_CTR;
    internal::NULL_REF_CTR.hold();
    return ret;
}
template <class To, class From>
FirmPtr<To> const_pointer_cast(const FirmPtr<From> &from) {
    return FirmPtr<To>(*from.m_p_ctr, const_cast<To *>(from.m_p_object));
}
template <class To, class From>
FirmPtr<To> const_pointer_cast(FirmPtr<From> &&from) {
    auto ret = FirmPtr<To>(*from.m_p_ctr, const_cast<To *>(from.m_p_object));
    from.m_p_ctr = &internal::NULL_REF_CTR;
    internal::NULL_REF_CTR.hold();
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
    from.m_p_ctr = &internal::NULL_REF_CTR;
    internal::NULL_REF_CTR.hold();
    return ret;
}

/**
 * @brief Shared implementation of std::optional for the ref-counted pointer
 * types (LazyPtr / FirmPtr / PinPtr). std::optional<...Ptr<...>> is used as a
 * nullable pointer.
 *
 * None of the pointer types can hold a nullptr as a valid value, so a null
 * counter pointer is used as the "no value" sentinel instead of a separate
 * bool flag: an engaged optional is bit-for-bit identical to the pointer it
 * holds, and an empty one only differs by storing nullptr in the counter slot.
 *
 * The contained pointer, when engaged, lives directly as the active member of
 * an anonymous union (@ref m_value), the other member (@ref CtrHead) exposing
 * the shared leading counter word.
 *
 * @note Relies on every PtrT being a standard-layout struct with its
 * `PolymorphicReferenceCounter* m_p_ctr` as the first member (true for all
 * three pointer types). PtrT and CtrHead therefore share a common initial
 * sequence, so the counter word can be read through CtrHead whichever member
 * is active; engaged <=> that word is non-null.
 */
template <class PtrT> class OptionalPtrBase {
    template <class O> friend class OptionalPtrBase;

    using RefCtr = PolymorphicReferenceCounter;

    // Empty and engaged states share storage through a union. CtrHead exposes
    // the `RefCtr* m_p_ctr` word that every PtrT carries as its first member
    struct alignas(PtrT) CtrHead {
        RefCtr *m_p_ctr;
    };
    union {
        CtrHead m_head;
        PtrT m_value;
    };

    static_assert(offsetof(CtrHead, m_p_ctr) == offsetof(PtrT, m_p_ctr) &&
                      alignof(CtrHead) == alignof(PtrT),
                  "The pointer type doesn't have the expected layout");

    // --- storage helpers ---

    // Activates m_head with the null sentinel. Assumes no member is currently
    // active (raw storage, or right after the contained pointer was destroyed).
    void set_empty() noexcept { m_head.m_p_ctr = nullptr; }

    // Constructs the contained pointer in place; assumes currently empty.
    template <class... Args> void construct(Args &&...args) {
        try {
            ::new (static_cast<void *>(&m_value))
                PtrT(std::forward<Args>(args)...);
        } catch (...) {
            set_empty();
            throw;
        }
    }

    // Assigns a value, reusing the contained pointer's own assignment (which
    // performs the proper release/hold) when already engaged, else constructs.
    template <class V> void assign(V &&value) {
        if (has_value())
            m_value = std::forward<V>(value);
        else
            construct(std::forward<V>(value));
    }

  public:
    using value_type = PtrT;

    // Constructors

    OptionalPtrBase() noexcept : m_head{nullptr} {}
    OptionalPtrBase(std::nullopt_t) noexcept : m_head{nullptr} {}

    OptionalPtrBase(const OptionalPtrBase &other) {
        if (other.has_value())
            construct(other.m_value);
        else
            set_empty();
    }
    OptionalPtrBase(OptionalPtrBase &&other) noexcept(
        std::is_nothrow_move_constructible_v<PtrT>) {
        if (other.has_value())
            construct(std::move(other.m_value));
        else
            set_empty();
    }

    // Converting from a compatible optional (e.g. derived -> base pointer)
    template <class P2>
    OptionalPtrBase(const OptionalPtrBase<P2> &other)
        requires std::is_constructible_v<PtrT, const P2 &>
    {
        if (other.has_value())
            construct(other.m_value);
        else
            set_empty();
    }
    template <class P2>
    OptionalPtrBase(OptionalPtrBase<P2> &&other)
        requires std::is_constructible_v<PtrT, P2 &&>
    {
        if (other.has_value())
            construct(std::move(other.m_value));
        else
            set_empty();
    }

    // In-place construction of the contained pointer
    template <class... Args>
    explicit OptionalPtrBase(std::in_place_t, Args &&...args) {
        construct(std::forward<Args>(args)...);
    }

    // From a pointer value (or anything a PtrT can be built from)
    template <class U = PtrT>
    OptionalPtrBase(U &&value)
        requires(std::is_constructible_v<PtrT, U> &&
                 !std::is_same_v<std::remove_cvref_t<U>, OptionalPtrBase> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t>)
    {
        construct(std::forward<U>(value));
    }

    // Destructor

    ~OptionalPtrBase() { reset(); }

    // Assignment

    OptionalPtrBase &operator=(std::nullopt_t) {
        reset();
        return *this;
    }

    OptionalPtrBase &operator=(const OptionalPtrBase &other) {
        if (other.has_value())
            assign(other.m_value);
        else
            reset();
        return *this;
    }
    OptionalPtrBase &operator=(OptionalPtrBase &&other) noexcept(
        std::is_nothrow_move_assignable_v<PtrT> &&
        std::is_nothrow_move_constructible_v<PtrT>) {
        if (other.has_value())
            assign(std::move(other.m_value));
        else
            reset();
        return *this;
    }

    template <class P2>
    OptionalPtrBase &operator=(const OptionalPtrBase<P2> &other)
        requires std::is_constructible_v<PtrT, const P2 &>
    {
        if (other.has_value())
            assign(other.m_value);
        else
            reset();
        return *this;
    }
    template <class P2>
    OptionalPtrBase &operator=(OptionalPtrBase<P2> &&other)
        requires std::is_constructible_v<PtrT, P2 &&>
    {
        if (other.has_value())
            assign(std::move(other.m_value));
        else
            reset();
        return *this;
    }

    template <class U = PtrT>
    OptionalPtrBase &operator=(U &&value)
        requires(std::is_constructible_v<PtrT, U> &&
                 !std::is_same_v<std::remove_cvref_t<U>, OptionalPtrBase>)
    {
        assign(std::forward<U>(value));
        return *this;
    }

    // Observer

    const PtrT *operator->() const noexcept { return &m_value; }
    PtrT *operator->() noexcept { return &m_value; }

    const PtrT &operator*() const & noexcept { return m_value; }
    PtrT &operator*() & noexcept { return m_value; }
    PtrT &&operator*() && noexcept { return std::move(m_value); }
    const PtrT &&operator*() const && noexcept { return std::move(m_value); }

    explicit operator bool() const noexcept { return has_value(); }
    bool has_value() const noexcept { return m_head.m_p_ctr != nullptr; }

    PtrT &value() & {
        if (!has_value())
            throw std::bad_optional_access();
        return m_value;
    }
    const PtrT &value() const & {
        if (!has_value())
            throw std::bad_optional_access();
        return m_value;
    }
    PtrT &&value() && {
        if (!has_value())
            throw std::bad_optional_access();
        return std::move(m_value);
    }
    const PtrT &&value() const && {
        if (!has_value())
            throw std::bad_optional_access();
        return std::move(m_value);
    }

    template <class U> PtrT value_or(U &&default_value) const & {
        return has_value() ? m_value
                           : static_cast<PtrT>(std::forward<U>(default_value));
    }
    template <class U> PtrT value_or(U &&default_value) && {
        return has_value() ? std::move(m_value)
                           : static_cast<PtrT>(std::forward<U>(default_value));
    }

    // Monadic operations

    template <class F> auto and_then(F &&f) & {
        using R = std::remove_cvref_t<std::invoke_result_t<F, PtrT &>>;
        return has_value() ? std::invoke(std::forward<F>(f), m_value) : R{};
    }
    template <class F> auto and_then(F &&f) const & {
        using R = std::remove_cvref_t<std::invoke_result_t<F, const PtrT &>>;
        return has_value() ? std::invoke(std::forward<F>(f), m_value) : R{};
    }
    template <class F> auto and_then(F &&f) && {
        using R = std::remove_cvref_t<std::invoke_result_t<F, PtrT &&>>;
        return has_value() ? std::invoke(std::forward<F>(f), std::move(m_value))
                           : R{};
    }

    template <class F> auto transform(F &&f) & {
        using U = std::remove_cv_t<std::invoke_result_t<F, PtrT &>>;
        return has_value()
                   ? std::optional<U>(std::invoke(std::forward<F>(f), m_value))
                   : std::optional<U>{};
    }
    template <class F> auto transform(F &&f) const & {
        using U = std::remove_cv_t<std::invoke_result_t<F, const PtrT &>>;
        return has_value()
                   ? std::optional<U>(std::invoke(std::forward<F>(f), m_value))
                   : std::optional<U>{};
    }
    template <class F> auto transform(F &&f) && {
        using U = std::remove_cv_t<std::invoke_result_t<F, PtrT &&>>;
        return has_value() ? std::optional<U>(std::invoke(std::forward<F>(f),
                                                          std::move(m_value)))
                           : std::optional<U>{};
    }

    template <class F> std::optional<PtrT> or_else(F &&f) const & {
        if (has_value())
            return std::optional<PtrT>(m_value);
        return std::invoke(std::forward<F>(f));
    }
    template <class F> std::optional<PtrT> or_else(F &&f) && {
        if (has_value())
            return std::optional<PtrT>(std::move(m_value));
        return std::invoke(std::forward<F>(f));
    }

    // Modifiers

    void reset() noexcept {
        if (has_value()) {
            m_value.~PtrT();
            set_empty();
        }
    }

    template <class... Args> PtrT &emplace(Args &&...args) {
        reset();
        construct(std::forward<Args>(args)...);
        return m_value;
    }

    void swap(OptionalPtrBase &other) noexcept(
        std::is_nothrow_move_constructible_v<PtrT> &&
        std::is_nothrow_swappable_v<PtrT>) {
        if (has_value() && other.has_value()) {
            // PtrT has no member swap; use a move dance
            PtrT tmp(std::move(m_value));
            m_value = std::move(other.m_value);
            other.m_value = std::move(tmp);
        } else if (has_value()) {
            other.construct(std::move(m_value));
            reset();
        } else if (other.has_value()) {
            construct(std::move(other.m_value));
            other.reset();
        }
    }
};

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

// Nullable-pointer optionals. The full interface is inherited from
// OptionalPtrBase. Only the special members (not inheritable by 'using') are
// re-declared so they delegate to the base.

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