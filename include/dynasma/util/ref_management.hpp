#pragma once
#ifndef INCLUDED_DYNASMA_REF_MAN_H
#define INCLUDED_DYNASMA_REF_MAN_H

#include "dynasma/core_concepts.hpp"

namespace dynasma {
template <AssetLike Asset> class ReferenceCounter {
    std::size_t m_strongcount;
    std::size_t m_weakcount;

  protected:
    Asset *p_asset;

    virtual void ensure_loaded_impl() = 0;
    virtual void allow_unload_impl() = 0;
    virtual void forget_impl() = 0;

  public:
    ReferenceCounter(){};
    virtual ~ReferenceCounter()
        : m_strongcount(0), m_weakcount(0), p_asset(nullptr){};

    Asset &hold() {
        if (m_strongcount == 0) {
            p_asset = ensure_loaded_impl();
        }
        m_strongcount++;
        return p_asset;
    }
    void release() {
        m_strongcount--;
        if (m_strongcount == 0) {
            allow_unload_impl(p_asset);
        }
    }
    Asset *p_get() { return p_asset; }

    void weak_hold() { m_weakcount++; }
    void weak_release() {
        m_weakcount--;
        if (m_weakcount == 0 && m_strongcount == 0) {
            forget_impl();
        }
    }
};
} // namespace dynasma

#endif // INCLUDED_DYNASMA_REF_MAN_H