#pragma once
#ifndef INCLUDED_DYNASMA_DYNAMIC_TYPING_H
#define INCLUDED_DYNASMA_DYNAMIC_TYPING_H

namespace dynasma {

class PolymorphicBase {
  public:
    virtual ~PolymorphicBase() = default;
};

template <class T> class ReferenceCounter;

/**
 * Base class that enables casting raw pointers to instances of derived classes
 * to FirmPtr and LazyPtr
 */
class PtrAwareBase {
    template <class T> friend class FirmPtr;
    template <class T> friend class LazyPtr;

    using RefCounter = ReferenceCounter<PolymorphicBase>;

    RefCounter *const m_p_counter;

  public:
    PtrAwareBase(RefCounter *p_counter) : m_p_counter(p_counter) {}
    PtrAwareBase() = delete;
    PtrAwareBase(const PtrAwareBase &other) = default;
    PtrAwareBase(PtrAwareBase &&other) = default;
    PtrAwareBase &operator=(const PtrAwareBase &other) = delete;
    PtrAwareBase &operator=(PtrAwareBase &&other) = delete;
};

} // namespace dynasma

#endif // INCLUDED_DYNASMA_DYNAMIC_TYPING_H