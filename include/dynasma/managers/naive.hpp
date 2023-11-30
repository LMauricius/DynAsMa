#pragma once
#ifndef INCLUDED_DYNASMA_MAN_SIMPLE_H
#define INCLUDED_DYNASMA_MAN_SIMPLE_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/managers/abstract.hpp"
#include "dynasma/pointer.hpp"
#include "dynasma/util/helpful_concepts.hpp"
#include "dynasma/util/ref_management.hpp"

#include <concepts>
#include <list>
#include <variant>

namespace dynasma {

/**
 * @brief The naive asset manager. Does the job, but has no advanced memory
 * management or caching features
 * @tparam Seed A SeedLike type describing everything we need to know about the
 * Asset
 * @tparam Alloc The AllocatorLike type whose instance will be used to construct
 * instances of the Seed::Asset
 */
template <SeedLike Seed, AllocatorLike Alloc>
class NaiveManager : public AbstractManager<Seed::Asset> {
    class ProxyReferenceManager : public ReferenceManager<Seed::Asset> {
        Seed m_seed;
        NaiveManager &m_manager;
        std::list<ProxyReferenceManager>::iterator m_it;

      protected:
        Seed::Asset &build_impl() override {
            Seed::Asset *p = Alloc::allocate(1);
            std::visit([p](const auto &arg) { new (p) Seed::Asset(arg); },
                       m_seed.kernel);
            return p;
        }
        void destroy_impl() override {
            delete (p_get())Seed::Asset;
            Alloc::deallocate(p_get(), 1);
        }
        void forget_impl() override {
            m_manager.m_seed_registry.erase(m_it); // deletes this
        }

      public:
        ProxyReferenceManager(Seed &&seed, NaiveManager &manager)
            : m_seed(seed), m_it(), m_manager(manager) {}

        void setSelfIterator(std::list<ProxyReferenceManager>::iterator it) {
            m_it = it;
        }
    };

    [[no_unique_address, msvc::no_unique_address]] Alloc m_allocator;
    std::list<ProxyReferenceManager> m_seed_registry;

  public:
    NaiveManager(const NaiveManager &) = delete;
    NaiveManager(NaiveManager &&) = delete;
    operator=(const NaiveManager &) = delete;
    operator=(NaiveManager &&) = delete;

    NaiveManager()
        requires std::default_constructible<Alloc>
        : m_alloc(){} = delete;
    NaiveManager(const Alloc &a) : m_allocator(a) {}
    NaiveManager(Alloc &&a) : m_allocator(std::move(a)) {}
    ~NaiveManager() = default;

    WeakPtr<Seed::Asset> register_asset(Seed &&seed) override {
        m_seed_registry.emplace_back(std::move(seed), *this);
    }
    void clean(std::size_t bytenum) override {
        // do nothing; cleans itself automatically
    }
};
} // namespace dynasma

#endif // INCLUDED_DYNASMA_MAN_SIMPLE_H