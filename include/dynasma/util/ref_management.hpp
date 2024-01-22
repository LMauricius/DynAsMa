#pragma once
#ifndef INCLUDED_DYNASMA_REF_MAN_H
#define INCLUDED_DYNASMA_REF_MAN_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/util/type_modification.hpp"

namespace dynasma {
template <class T> class ReferenceCounter {
    std::size_t m_strongcount;
    std::size_t m_weakcount;

  protected:
    T *p_obj;

    /**
     * @brief Ensures that the asset is loaded and its pointer is stored in
     * p_asset
     * @note Called only while the asset is allowed to be unloaded
     */
    virtual void ensure_loaded_impl() = 0;
    /**
     * @brief Allows unloading the asset
     * @note Called while the asset is required to be loaded
     */
    virtual void allow_unload_impl() = 0;
    /**
     * @brief Allows `this` to be deleted
     * @note Called only while the asset is allowed to be unloaded
     * @note this instance should not be referenced after this call
     */
    virtual void forget_impl() = 0;

  public:
    ReferenceCounter() : m_strongcount(0), m_weakcount(0), p_obj(nullptr){};
    virtual ~ReferenceCounter(){};

    /**
     * @brief Raises the strong reference count
     * @note If the asset is not loaded, it will be loaded
     * @returns reference to the loaded asset
     */
    T &hold() {
        if (m_strongcount == 0) {
            ensure_loaded_impl();
        }
        m_strongcount++;
        return *p_obj;
    }
    /**
     * @brief Reduces the strong reference count
     * @note If the count reaches 0, the asset can be unloaded
     */
    void release() {
        m_strongcount--;
        if (m_strongcount == 0) {
            allow_unload_impl();
            if (m_weakcount == 0) {
                forget_impl();
            }
        }
    }
    /**
     * @returns a pointer to the loaded asset or nullptr if the asset is
     * not loaded
     */
    T *p_get() { return p_obj; }

    /**
     * @brief Increases the weak reference count
     */
    void weak_hold() { m_weakcount++; }
    /**
     * @brief Reduces the weak reference count
     * @note If the count reaches 0, this instance can be deleted
     */
    void weak_release() {
        m_weakcount--;
        if (m_weakcount == 0 && m_strongcount == 0) {
            forget_impl();
        }
    }

    /**
     * Check if the counter object is usable.
     * @return true if the object is usable, false otherwise
     */
    bool is_usable() const { return m_strongcount > 0; }
    /**
     * Check if the counter is unloadable. Also positive if it is forgetable
     * @return true if the object is unloadable, false otherwise
     */
    bool is_unloadable() const { return m_strongcount == 0; }
    /**
     * Check if the counter is forgettable based on the reference counts.
     * @return true if the object is forgettable, false otherwise
     */
    bool is_forgetable() const {
        return m_strongcount == 0 && m_weakcount == 0;
    }

    /**
     * @enum Status
     * @brief The mutually exclusive status of the object for which we count
     * references
     */
    enum class Status { SurelyLoaded, Unloadable, Forgetable };
    /**
     * Returns the mutually exclusive status of the object.
     * @return the status of the object
     */
    Status get_status() const {
        if (is_forgetable()) {
            return Status::Forgetable;
        } else if (is_unloadable()) {
            return Status::Unloadable;
        } else {
            return Status::Usable;
        }
    }
};
} // namespace dynasma

#endif // INCLUDED_DYNASMA_REF_MAN_H