#pragma once
#include "dynasma/pointer.hpp"
#ifndef INCLUDED_DYNASMA_CONSTRUCTION_H
#define INCLUDED_DYNASMA_CONSTRUCTION_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/util/ref_management.hpp"

#include <memory>

namespace dynasma {

template <class T, class Ctr, class... ArgTs>
void constructObject(T *p, Ctr &ctr, ArgTs &&...args)
    requires(!RawConvertibleToPtr<T>)
{
    new (p) T(std::forward<ArgTs>(args)...);
}

template <class T, class Ctr, class... ArgTs>
void constructObject(T *p, Ctr &ctr, ArgTs &&...args)
    requires RawConvertibleToPtr<T>
{
    new (p) T(&ctr, std::forward<ArgTs>(args)...);
}

template <class T> void destroyObject(T *p) { p->~T(); }

} // namespace dynasma

#endif // INCLUDED_DYNASMA_CONSTRUCTION_H