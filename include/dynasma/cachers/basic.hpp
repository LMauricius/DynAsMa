#pragma once
#ifndef INCLUDED_DYNASMA_CACHER_BASIC_H
#define INCLUDED_DYNASMA_CACHER_BASIC_H

#include "dynasma/cachers/abstract.hpp"
#include "dynasma/core_concepts.hpp"
#include "dynasma/pointer.hpp"
#include "dynasma/util/construction.hpp"
#include "dynasma/util/definitions.hpp"
#include "dynasma/util/helpful_concepts.hpp"
#include "dynasma/util/ref_management.hpp"

#include <cassert>
#include <concepts>
#include <list>
#include <map>
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
template <CacheableSeedLike Seed, SeededAllocatorLike<Seed> Alloc>
class BasicCacher : public virtual AbstractCacher<Seed> {
  public:
    using ConstructedAsset = typename Alloc::value_type;
    using ExposedAsset = typename Seed::Asset;

  private:
    // reference counting response implementation
    class ProxyRefCtr : public TypeErasedReferenceCounter<ExposedAsset> {
        BasicCacher &m_manager;
        std::list<ProxyRefCtr>::iterator m_it;
        std::map<Seed, ProxyRefCtr *const>::iterator m_map_it;

      protected:
        void handle_usable_impl() override {
            if (!this->is_loaded()) {
                const Seed &seed = m_map_it->first;

                // create new
                ConstructedAsset *p_asset = m_manager.m_allocator.allocate(1);
                this->p_obj = p_asset;

                // move from unloaded to used
                this->m_manager.m_used_registry.splice(
                    this->m_manager.m_used_registry.end(),
                    this->m_manager.m_unloaded_registry, m_it);

                // construct (may throw)
                std::visit(
                    [p_asset, this](const auto &arg) {
                        constructObject(p_asset, *this, arg);
                    },
                    seed.kernel);
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
            ConstructedAsset &asset_casted =
                *dynamic_cast<ConstructedAsset *>(this->p_obj);
            destroyObject(this->p_obj);
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

    [[DYNASMA_NO_UNIQUE_ADDRESS]] Alloc m_allocator;

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
    ~BasicCacher()
    {
        assert(m_unloaded_registry.size() == 0 && m_cached_registry.size() == 0 &&
               m_used_registry.size() == 0);
    }

    using AbstractCacher<Seed>::retrieve_asset;

    LazyPtr<ExposedAsset> retrieve_asset(Seed &&seed) override {
        // check if the seed has already been registered
        auto lb = m_searchable_registry.lower_bound(seed);

        if (lb != m_searchable_registry.end() &&
            !(m_searchable_registry.key_comp()(seed, lb->first))) {
            // key already exists
            return LazyPtr<ExposedAsset>(*(lb->second));
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

            return LazyPtr<ExposedAsset>(newCtr);
        }
    }
    std::size_t clean(std::size_t bytenum) override
    {
        /*
        Unloads the oldest unloadable assets first
        */
        std::size_t bFreed = 0;
        while (bFreed < bytenum && !m_cached_registry.empty()) {
            bFreed += dynamic_cast<ConstructedAsset &>(
                          *m_cached_registry.front().p_get())
                          .memory_cost();
            m_cached_registry.front().unload();
        }

        return bFreed;
    }
};

} // namespace dynasma

#endif // INCLUDED_DYNASMA_CACHER_BASIC_H