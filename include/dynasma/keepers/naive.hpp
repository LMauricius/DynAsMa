#pragma once
#ifndef INCLUDED_DYNASMA_KEEPER_SIMPLE_H
#define INCLUDED_DYNASMA_KEEPER_SIMPLE_H

#include "dynasma/core_concepts.hpp"
#include "dynasma/keepers/abstract.hpp"
#include "dynasma/pointer.hpp"
#include "dynasma/util/definitions.hpp"
#include "dynasma/util/dynamic_typing.hpp"
#include "dynasma/util/helpful_concepts.hpp"
#include "dynasma/util/ref_management.hpp"

#include <concepts>
#include <list>
#include <variant>

namespace dynasma {

/**
 * @brief The naive asset keeper. Loads immediately, unloads when all references
 * to the asset are dropped
 * @tparam Seed A SeedLike type describing everything we need to know
 * about the Asset
 * @tparam Alloc The AllocatorLike type whose instance will be used to construct
 * instances of the Seed::Asset
 */
template <SeedLike Seed, SeededAllocatorLike<Seed> Alloc>
class NaiveKeeper : public virtual AbstractKeeper<Seed> {
  public:
    using ConstructedAsset = typename Alloc::value_type;
    using ExposedAsset = typename Seed::Asset;

  private:
    // reference counting response implementation
    class ProxyRefCtr : public TypeErasedReferenceCounter<ExposedAsset> {
        NaiveKeeper &m_manager;

      protected:
        void handle_usable_impl() override {}
        void handle_unloadable_impl() override {}
        void handle_forgettable_impl() override { delete this; }

      public:
        ProxyRefCtr(const Seed &seed, NaiveKeeper &manager)
            : m_manager(manager) {
            ConstructedAsset *p_asset = m_manager.m_allocator.allocate(1);
            this->p_obj = p_asset;
            std::visit(
                [p_asset](const auto &arg) {
                    new (p_asset) ConstructedAsset(arg);
                },
                seed.kernel);
        }
        ~ProxyRefCtr() {
            ConstructedAsset &asset_casted =
                *dynamic_cast<ConstructedAsset *>(this->p_obj);
            this->p_obj->~PolymorphicBase();
            m_manager.m_allocator.deallocate(&asset_casted, 1);
        }
    };

    [[DYNASMA_NO_UNIQUE_ADDRESS]] Alloc m_allocator;

  public:
    NaiveKeeper(const NaiveKeeper &) = delete;
    NaiveKeeper(NaiveKeeper &&) = delete;
    NaiveKeeper &operator=(const NaiveKeeper &) = delete;
    NaiveKeeper &operator=(NaiveKeeper &&) = delete;

    NaiveKeeper()
        requires std::default_initializable<Alloc>
        : m_allocator(){};
    NaiveKeeper(const Alloc &a) : m_allocator(a) {}
    NaiveKeeper(Alloc &&a) : m_allocator(std::move(a)) {}
    ~NaiveKeeper() = default;

    LazyPtr<ExposedAsset> new_asset(const Seed &seed) override {
        return LazyPtr<ExposedAsset>(*(new ProxyRefCtr(seed, *this)));
    }
    std::size_t clean(std::size_t bytenum) override
    {
        // do nothing; cleans itself automatically
        return 0;
    }
};
} // namespace dynasma

#endif // INCLUDED_DYNASMA_KEEPER_SIMPLE_H