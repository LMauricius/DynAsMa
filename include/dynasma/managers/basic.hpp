#pragma once
#ifndef INCLUDED_DYNASMA_MAN_BASIC_H
#define INCLUDED_DYNASMA_MAN_BASIC_H

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
 * @brief A basic asset manager. Allocates assets when they are needed and
 * deletes them when cleanup is requested, oldest first
 * @tparam Seed A SeedLike type describing everything we need to know about the
 * Asset
 * @tparam Alloc The AllocatorLike type whose instance will be used to construct
 * instances of the Seed::Asset
 */
template <SeedLike Seed, AllocatorLike Alloc>
class BasicManager : public AbstractManager<Seed> {
    class ProxyRefCtr : public ReferenceCounter<typename Seed::Asset> {
        Seed m_seed;
        BasicManager &m_manager;
        std::list<ProxyRefCtr>::iterator m_it;

      protected:
        void ensure_loaded_impl() override {
            if (this->p_asset == nullptr) {
                // create new
                this->p_asset = m_manager.m_allocator.allocate(1);
                std::visit(
                    [this](const auto &arg) {
                        new (this->p_asset) typename Seed::Asset(arg);
                    },
                    this->m_seed.kernel);

                // move from unloaded to used
                this->m_manager.m_used_registry.splice(
                    this->m_manager.m_used_registry.end(),
                    this->m_manager.m_unloaded_registry, m_it);
            } else {
                // move from cached to used
                this->m_manager.m_used_registry.splice(
                    this->m_manager.m_used_registry.end(),
                    this->m_manager.m_cached_registry, m_it);
            }
        }
        void allow_unload_impl() override {
            // move from used to cached
            this->m_manager.m_cached_registry.splice(
                this->m_manager.m_cached_registry.end(),
                this->m_manager.m_used_registry, m_it);
        }
        void forget_impl() override {
            if (this->p_asset != nullptr) {
                this->unload();
            }
            m_manager.m_unloaded_registry.erase(m_it); // deletes this
        }

      public:
        ProxyRefCtr(Seed &&seed, BasicManager &manager)
            : m_seed(seed), m_it(), m_manager(manager) {}

        /**
         * Unloads the asset and moves it from the cached registry to the
         * unloaded registry.
         *
         * @throws None
         */
        void unload() {
            // unload
            this->p_asset->Seed::Asset::~Asset();
            m_manager.m_allocator.deallocate(this->p_asset, 1);
            this->p_asset = nullptr;

            // move from cached to unloaded
            m_manager.m_unloaded_registry.splice(
                m_manager.m_unloaded_registry.end(),
                m_manager.m_cached_registry, m_it);
        }

        void setSelfRegistryPos(std::list<ProxyRefCtr> *p_cur_list,
                                std::list<ProxyRefCtr>::iterator it) {
            m_it = it;
        }
    };

    [[no_unique_address, msvc::no_unique_address]] Alloc m_allocator;

    // 3 seed registries
    std::list<ProxyRefCtr> m_unloaded_registry;
    std::list<ProxyRefCtr> m_cached_registry;
    std::list<ProxyRefCtr> m_used_registry;

  public:
    BasicManager(const BasicManager &) = delete;
    BasicManager(BasicManager &&) = delete;
    BasicManager &operator=(const BasicManager &) = delete;
    BasicManager &operator=(BasicManager &&) = delete;

    BasicManager()
        requires std::default_initializable<Alloc>
        : m_allocator(){};
    BasicManager(const Alloc &a) : m_allocator(a) {}
    BasicManager(Alloc &&a) : m_allocator(std::move(a)) {}
    ~BasicManager() = default;

    using AbstractManager<Seed>::register_asset;

    WeakPtr<typename Seed::Asset> register_asset(Seed &&seed) override {
        m_unloaded_registry.emplace_front(std::move(seed), *this);
        m_unloaded_registry.front().setSelfRegistryPos(
            &m_unloaded_registry, m_unloaded_registry.begin());
        return WeakPtr<typename Seed::Asset>(m_unloaded_registry.front());
    }
    void clean(std::size_t bytenum) override {
        /*
        Unloads the oldest unloadable assets first
        */
        std::size_t bFreed = 0;
        while (bFreed < bytenum && !m_cached_registry.empty()) {
            bFreed += m_cached_registry.front().p_get()->memory_cost();
            m_cached_registry.front().unload();
        }
    }
};

} // namespace dynasma

#endif // INCLUDED_DYNASMA_MAN_BASIC_H