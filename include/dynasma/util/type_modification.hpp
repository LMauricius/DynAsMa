#pragma once
#ifndef INCLUDED_DYNASMA_TYPE_MOD_H
#define INCLUDED_DYNASMA_TYPE_MOD_H

#include <type_traits>

namespace dynasma {

template <typename From, typename To> struct CopyCV {
  private:
    using U1 = std::conditional_t<std::is_const<From>::value,
                                  std::add_const_t<To>, To>;
    using U2 = std::conditional_t<std::is_volatile<From>::value,
                                  std::add_volatile_t<U1>, U1>;

  public:
    using type = U2;
};

} // namespace dynasma

#endif // INCLUDED_DYNASMA_TYPE_MOD_H