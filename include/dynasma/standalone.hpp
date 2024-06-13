#pragma once
#ifndef INCLUDED_DYNASMA_STANDALONE_H
#define INCLUDED_DYNASMA_STANDALONE_H

#include "dynasma/pointer.hpp"

namespace dynasma {

namespace internal {

template <AssetLike T>
class StandaloneRefCtr : public TypeErasedReferenceCounter<T> {
  protected:
    void handle_usable_impl() override {}
    void handle_unloadable_impl() override {}
    void handle_forgettable_impl() override { delete this; }

  public:
    template <class... Params> StandaloneRefCtr(Params... params) {
        this->p_obj = new T(params...);
    }
    ~StandaloneRefCtr() { delete this->p_obj; }
};

} // namespace internal

template <AssetLike T, class... Params>
FirmPtr<T> makeStandalone(Params... params) {
    return FirmPtr<T>(
        *(new internal::StandaloneRefCtr<T>(std::forward<Params>(params)...)));
}

} // namespace dynasma

#endif // INCLUDED_DYNASMA_STANDALONE_H