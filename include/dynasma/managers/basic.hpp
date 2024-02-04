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
 * @tparam Seed A ReloadableSeedLike type describing everything we need to know
 * about the Asset
 * @tparam Alloc The AllocatorLike type whose instance will be used to construct
 * instances of the Seed::Asset
 */
template <ReloadableSeedLike Seed, SeededAllocatorLike<Seed> Alloc>
class BasicManager : public virtual AbstractManager<Seed> {
  public:
    using ConstructedAsset = typename Alloc::value_type;
    using ExposedAsset = typename Seed::Asset;

  private:
    // reference counting response implementation
    class ProxyRefCtr : public TypeErasedReferenceCounter<ExposedAsset> {
        Seed m_seed;
        BasicManager &m_manager;
        std::list<ProxyRefCtr>::iterator m_it;

      protected:
        void handle_usable_impl() override {
            if (!this->is_loaded()) {
                // create new
                ConstructedAsset *p_asset = m_manager.m_allocator.allocate(1);
                this->p_obj = p_asset;
                std::visit(
                    [p_asset](const auto &arg) {
                        new (p_asset) ConstructedAsset(arg);
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
        void handle_unloadable_impl() override {
            // move from used to cached
            this->m_manager.m_cached_registry.splice(
                this->m_manager.m_cached_registry.end(),
                this->m_manager.m_used_registry, m_it);
        }
        void handle_forgettable_impl() override {
            if (this->is_loaded()) {
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
            ConstructedAsset &asset_casted =
                *dynamic_cast<ConstructedAsset *>(this->p_obj);
            this->p_obj->~PolymorphicBase();
            m_manager.m_allocator.deallocate(&asset_casted, 1);
            this->p_obj = nullptr;

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

    LazyPtr<ExposedAsset> register_asset(Seed &&seed) override {
        m_unloaded_registry.emplace_front(std::move(seed), *this);
        m_unloaded_registry.front().setSelfRegistryPos(
            &m_unloaded_registry, m_unloaded_registry.begin());
        return LazyPtr<ExposedAsset>(m_unloaded_registry.front());
    }
    void clean(std::size_t bytenum) override {
        /*
        Unloads the oldest unloadable assets first
        */
        std::size_t bFreed = 0;
        while (bFreed < bytenum && !m_cached_registry.empty()) {
            bFreed +=
                dynamic_cast<ExposedAsset &>(*m_cached_registry.front().p_get())
                    .memory_cost();
            m_cached_registry.front().unload();
        }
    }
};

} // namespace dynasma

#endif // INCLUDED_DYNASMA_MAN_BASIC_H