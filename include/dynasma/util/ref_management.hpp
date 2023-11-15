#pragma once
#ifndef INCLUDED_DYNASMA_REF_MAN_H
#define INCLUDED_DYNASMA_REF_MAN_H

#include "dynasma/core_concepts.hpp"

namespace dynasma {
template <AssetLike Asset> class ReferenceManager {
  public:
    virtual ~ReferenceManager() = 0;

    virtual Asset &hold() = 0;
    virtual void release() = 0;

    virtual void weak_hold() = 0;
    virtual void weak_release() = 0;
};
} // namespace dynasma

#endif // INCLUDED_DYNASMA_REF_MAN_H