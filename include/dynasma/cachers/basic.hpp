#pragma once
#ifndef INCLUDED_DYNASMA_MAN_BASIC_H
#define INCLUDED_DYNASMA_MAN_BASIC_H

#include "dynasma/cachers/abstract.hpp"
#include "dynasma/core_concepts.hpp"
#include "dynasma/pointer.hpp"
#include "dynasma/util/helpful_concepts.hpp"
#include "dynasma/util/ref_management.hpp"

#include <concepts>
#include <list>
#include <map>
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
template <SortableSeedLike Seed, AllocatorLike<typename Seed::Asset> Alloc>
class BasicCacher : public virtual AbstractCacher<Seed> {
  public:
    using Asset = typename Seed::Asset;

  private:
    // reference counting response implementation
    class ProxyRefCtr : public TypeErasedReferenceCounter<Asset> {
        BasicCacher &m_manager;
        std::list<ProxyRefCtr>::iterator m_it;
        std::map<Seed, ProxyRefCtr *const>::iterator m_map_it;

      protected:
        void handle_usable_impl() override {
            if (!this->is_loaded()) {
                const Seed &seed = m_map_it->first;

                // create new
                Asset *p_asset = m_manager.m_allocator.allocate(1);
                this->p_obj = p_asset;
                std::visit(
                    [p_asset](const auto &arg) { new (p_asset) Asset(arg); },
                    seed.kernel);

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
            if (!this->is_loaded()) {
                forget();
            }
            // else we keep it cached for when we remember it
        }

        /**
         * @note This must only be called when the asset is not loaded
         */
        void forget() {
            m_manager.m_searchable_registry.erase(m_map_it); // removes the seed
            m_manager.m_unloaded_registry.erase(m_it);       // deletes this
        }

      public:
        ProxyRefCtr(BasicCacher &manager) : m_it(), m_manager(manager) {}

        /**
         * Unloads the asset and moves it from the cached registry to the
         * unloaded registry.
         *
         * @throws None
         */
        void unload() {
            // unload
            Asset &asset_casted = *dynamic_cast<Asset *>(this->p_obj);
            this->p_obj->~PolymorphicBase();
            m_manager.m_allocator.deallocate(&asset_casted, 1);
            this->p_obj = nullptr;

            // move from cached to unloaded
            m_manager.m_unloaded_registry.splice(
                m_manager.m_unloaded_registry.end(),
                m_manager.m_cached_registry, m_it);

            if (this->is_forgettable()) {
                forget();
            }
        }

        void setSelfRegistryPos(
            std::list<ProxyRefCtr> *p_cur_list,
            std::list<ProxyRefCtr>::iterator it,
            std::map<Seed, ProxyRefCtr *const>::iterator map_it) {
            m_it = it;
            m_map_it = map_it;
        }
    };

    [[no_unique_address, msvc::no_unique_address]] Alloc m_allocator;

    // 3 seed registries
    std::list<ProxyRefCtr> m_unloaded_registry;
    std::list<ProxyRefCtr> m_cached_registry;
    std::list<ProxyRefCtr> m_used_registry;
    std::map<Seed, ProxyRefCtr *const> m_searchable_registry;

  public:
    BasicCacher(const BasicCacher &) = delete;
    BasicCacher(BasicCacher &&) = delete;
    BasicCacher &operator=(const BasicCacher &) = delete;
    BasicCacher &operator=(BasicCacher &&) = delete;

    BasicCacher()
        requires std::default_initializable<Alloc>
        : m_allocator(){};
    BasicCacher(const Alloc &a) : m_allocator(a) {}
    BasicCacher(Alloc &&a) : m_allocator(std::move(a)) {}
    ~BasicCacher() = default;

    using AbstractCacher<Seed>::retrieve_asset;

    WeakPtr<Asset> retrieve_asset(Seed &&seed) override {
        // check if the seed has already been registered
        auto lb = m_searchable_registry.lower_bound(seed);

        if (lb != m_searchable_registry.end() &&
            !(m_searchable_registry.key_comp()(seed, lb->first))) {
            // key already exists
            return WeakPtr<Asset>(*(lb->second));
        } else {
            // the key does not exist in the map
            // add it to the map

            // create the counter
            m_unloaded_registry.emplace_front(*this);
            ProxyRefCtr &newCtr = m_unloaded_registry.front();

            // register it into the searchable registry
            auto it = m_searchable_registry.insert(
                lb, {seed, &newCtr}); // Use lb as a hint to insert,
                                      // so it can avoid another lookup

            // make it aware of its place
            newCtr.setSelfRegistryPos(&m_unloaded_registry,
                                      m_unloaded_registry.begin(), it);

            return WeakPtr<Asset>(newCtr);
        }
    }
    void clean(std::size_t bytenum) override {
        /*
        Unloads the oldest unloadable assets first
        */
        std::size_t bFreed = 0;
        while (bFreed < bytenum && !m_cached_registry.empty()) {
            bFreed += dynamic_cast<Asset &>(*m_cached_registry.front().p_get())
                          .memory_cost();
            m_cached_registry.front().unload();
        }
    }
};

} // namespace dynasma

#endif // INCLUDED_DYNASMA_MAN_BASIC_H