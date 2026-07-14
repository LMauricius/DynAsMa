#pragma once
#ifndef INCLUDED_DYNASMA_CAST_H
#define INCLUDED_DYNASMA_CAST_H

namespace dynasma {

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

} // namespace internal

} // namespace dynasma

#endif // INCLUDED_DYNASMA_CAST_H