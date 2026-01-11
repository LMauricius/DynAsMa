#pragma once
#include "dynasma/core_concepts.hpp"
#ifndef INCLUDED_DYNASMA_STANDALONE_H
#define INCLUDED_DYNASMA_STANDALONE_H

#include "dynasma/pointer.hpp"

namespace dynasma {

namespace internal {

template <StandardPolymorphic T>
class StandaloneRefCtr : public TypeErasedReferenceCounter<T> {
  protected:
    void handle_usable_impl() override {}
    void handle_unloadable_impl() override {}
    void handle_forgettable_impl() override { delete this; }

    T m_obj;

  public:
    template <class... Params>
    StandaloneRefCtr(Params... params)
        requires(!RawConvertibleToPtr<T>)
        : m_obj(std::forward<Params>(params)...) {
        this->p_obj = &(this->m_obj);
    }

    template <class... Params>
    StandaloneRefCtr(Params... params)
        requires RawConvertibleToPtr<T>
        : m_obj(this, std::forward<Params>(params)...) {
        this->p_obj = &(this->m_obj);
    }

    ~StandaloneRefCtr() {}
};

} // namespace internal

template <StandardPolymorphic T, class... Params>
FirmPtr<T> makeStandalone(Params... params) {
    return FirmPtr<T>(
        *(new internal::StandaloneRefCtr<T>(std::forward<Params>(params)...)));
}

} // namespace dynasma

#endif // INCLUDED_DYNASMA_STANDALONE_H