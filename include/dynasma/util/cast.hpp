#pragma once
#ifndef INCLUDED_DYNASMA_CAST_H
#define INCLUDED_DYNASMA_CAST_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/util/helpful_concepts.hpp"

#include <concepts>

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

namespace internal {

/**
 * Performs a dynamic cast without checking whether the object is of that type.
 * Can be faster than normal dynamic_cast, but passing a wrong object causes UB.
 * Never returns nullptr
 */
template <typename TPtr, typename F>
inline TPtr assume_dynamic_cast(F *p_from) {
    /*
    Assume the casting is possible, i.e. we have a valid pointer
    Can be dereferenced -> is not nullptr
    This could be used to optimize for non-virtual inheritances because.
    With the naive dynamic_cast, nullptr is returned when casting is not
    possible, so the type needs to be checked in addition to offsetting the ptr
    */
    return &*dynamic_cast<TPtr>(p_from);
}

/**
 * Converts O* to T* for a PointerCastable relation: a no-op when O and T are
 * the same type (ignoring cv), otherwise routed through assume_dynamic_cast.
 * Never returns nullptr.
 */
template <typename T, typename O> inline T *assume_convert(O *p_from) {
    if constexpr (PointerNoCastNeeded<O, T>)
        return p_from;
    else
        return assume_dynamic_cast<T *>(p_from);
}

} // namespace internal

} // namespace dynasma

#endif // INCLUDED_DYNASMA_CAST_H