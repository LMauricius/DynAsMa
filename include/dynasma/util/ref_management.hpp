#pragma once
#ifndef INCLUDED_DYNASMA_REF_MAN_H
#define INCLUDED_DYNASMA_REF_MAN_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/util/type_modification.hpp"

namespace dynasma {
template <class T> class ReferenceCounter {
    std::size_t m_firmcount;
    std::size_t m_lazycount;

  protected:
    T *p_obj;

    /**
     * @brief Ensures that the asset is loaded and its pointer is stored in
     * p_asset
     * @note Called only on the switch from unloadable to usable
     */
    virtual void handle_usable_impl() = 0;
    /**
     * @brief Allows unloading the asset
     * @note Called only on the switch from usable to unloadable
     */
    virtual void handle_unloadable_impl() = 0;
    /**
     * @brief Allows `this` to be deleted
     * @note Called only on the switch from unloadable to forgettable
     * @note this instance should not be referenced after this call
     * @note this instance can be deleted by this function, or after calling it
     */
    virtual void handle_forgettable_impl() = 0;

  public:
    ReferenceCounter() : m_firmcount(0), m_lazycount(0), p_obj(nullptr){};
    virtual ~ReferenceCounter(){};

    /**
     * @brief Raises the firm reference count
     * @note If the asset is not loaded, it will be loaded
     * @returns reference to the loaded asset
     */
    T &hold() {
        if (is_unloadable()) {
            m_firmcount++;
            handle_usable_impl();
        } else {
            m_firmcount++;
        }
        return *p_obj;
    }
    /**
     * @brief Reduces the firm reference count
     * @note If the count reaches 0, the asset can be unloaded
     */
    void release() {
        m_firmcount--;
        if (is_unloadable()) {
            handle_unloadable_impl();
            if (m_lazycount == 0) {
                handle_forgettable_impl();
            }
        }
    }
    /**
     * @returns a pointer to the loaded asset or nullptr if the asset is
     * not loaded
     */
    T *p_get() { return p_obj; }

    /**
     * @brief Increases the lazy reference count
     */
    void lazy_hold() { m_lazycount++; }
    /**
     * @brief Reduces the lazy reference count
     * @note If the count reaches 0, this instance can be deleted
     */
    void lazy_release() {
        m_lazycount--;
        if (is_forgettable()) {
            handle_forgettable_impl();
        }
    }

    /**
     * Check if the counter object is usable.
     * @return true if the object is usable, false otherwise
     */
    bool is_usable() const { return m_firmcount > 0; }
    /**
     * Check if the counter is unloadable. Also positive if it is forgettable
     * @return true if the object is unloadable, false otherwise
     */
    bool is_unloadable() const { return m_firmcount == 0; }
    /**
     * Check if the counter is forgettable based on the reference counts.
     * @return true if the object is forgettable, false otherwise
     */
    bool is_forgettable() const {
        return m_firmcount == 0 && m_lazycount == 0;
    }

    /**
     * @returns whether the asset is loaded
     */
    bool is_loaded() const { return p_obj != nullptr; }

    /**
     * @returns whether the asset is loaded, but unloadable
     */
    bool is_cached() const { return is_unloadable() && is_loaded(); }

    /**
     * @enum CounterState
     * @brief The mutually exclusive status of the counter
     */
    enum class CounterState { Usable, Unloadable, Forgettable };

    /**
     * @return CounterState
     */
    CounterState get_counter_state() const {
        if (is_forgettable()) {
            return CounterState::Forgettable;
        } else if (is_unloadable()) {
            return CounterState::Unloadable;
        } else {
            return CounterState::Usable;
        }
    }

    /**
     * @enum ObjectState
     * @brief The mutually exclusive status of the object for which we count
     * references
     */
    enum class ObjectState { Used, Cached, Unloaded };

    /**
     * @return ObjectState
     */
    ObjectState get_object_state() const {
        if (is_loaded()) {
            if (is_usable()) {
                return ObjectState::Used;
            } else {
                return ObjectState::Cached;
            }
        } else {
            return ObjectState::Unloaded;
        }
    }
};
} // namespace dynasma

#endif // INCLUDED_DYNASMA_REF_MAN_H