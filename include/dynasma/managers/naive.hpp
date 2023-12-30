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
class NaiveManager : public AbstractManager<Seed> {
    class ProxyRefCtr : public ReferenceCounter<typename Seed::Asset> {
        Seed m_seed;
        NaiveManager &m_manager;
        std::list<ProxyRefCtr>::iterator m_it;

      protected:
        void ensure_loaded_impl() override {
            this->p_asset = m_manager.m_allocator.allocate(1);
            std::visit(
                [this](const auto &arg) {
                    new (this->p_asset) Seed::Asset(arg);
                },
                m_seed.kernel);
        }
        void allow_unload_impl() override {
            this->p_asset->Seed::Asset::~Asset();
            m_manager.m_allocator.deallocate(this->p_asset, 1);
            this->p_asset = nullptr;
        }
        void forget_impl() override {
            m_manager.m_seed_registry.erase(m_it); // deletes this
        }

      public:
        ProxyRefCtr(Seed &&seed, NaiveManager &manager)
            : m_seed(seed), m_it(), m_manager(manager) {}

        void setSelfRegistryPos(std::list<ProxyRefCtr>::iterator it) {
            m_it = it;
        }
    };

    [[no_unique_address, msvc::no_unique_address]] Alloc m_allocator;
    std::list<ProxyRefCtr> m_seed_registry;

  public:
    NaiveManager(const NaiveManager &) = delete;
    NaiveManager(NaiveManager &&) = delete;
    NaiveManager &operator=(const NaiveManager &) = delete;
    NaiveManager &operator=(NaiveManager &&) = delete;

    NaiveManager()
        requires std::default_initializable<Alloc>
        : m_allocator(){};
    NaiveManager(const Alloc &a) : m_allocator(a) {}
    NaiveManager(Alloc &&a) : m_allocator(std::move(a)) {}
    ~NaiveManager() = default;

    WeakPtr<typename Seed::Asset> register_asset(Seed &&seed) override {
        m_seed_registry.emplace_front(std::move(seed), *this);
        m_seed_registry.front().setSelfRegistryPos(m_seed_registry.begin());
        return WeakPtr<typename Seed::Asset>(m_seed_registry.front());
    }
    void clean(std::size_t bytenum) override {
        // do nothing; cleans itself automatically
    }
};
} // namespace dynasma

#endif // INCLUDED_DYNASMA_MAN_SIMPLE_H