# DynAsMa
A modern C++ **dyn**amic **as**set **ma**nagement library.

# Good stuff about it
- Header only
- Simple to use templates
- C++20 concepts for safe and clear code
- Completely modular and extensible
- Simple to setup and use, (almost) fully documented code
- Assertion fails for misusing it (see **Warnings** section)

# Features
- Lifetime tracking
- Caching
- Asset managers and advanced Cachers
- Custom allocator support
- Low overhead polymorphism
- Built-in managers with immediate or on-demand memory cleanup

# Examples
The examples can be found in the `examples/test*` folders.
See [test1](https://github.com/LMauricius/DynAsMa/blob/main/examples/test1/main.cpp) for basic feature comparison.

# Versioning
The project is feature ready, but WIP and shouldn't be considered ABI stable.
When using it in a project, always stay at a specific release.

# What else can it do?
- It can track lifetimes of pointers to assets and allocate+load/deallocate them when needed
- Use LazyPtr to reference assets that don't have to be loaded (yet), FirmPtr to load them and get access to their contents.
- Upcast pointers, implement AbstractManager and AbstractCacher of an asset as a creator of asset's derived instances
- Register parameters for an asset's construction, pass around LazyPtrs until loading the asset later
- Cache parameters and reuse the asset with the same parameters

# Terminology
- Asset: an object we keep track of
- Seed: A class that defines all info we need about an Asset type. Contains parameters for Asset construction.
    - Seed::kernel: Parameters for Asset construction. A variant of possible constructor argument types.
- LazyPtr: a reference to an Asset that doesn't have to be loaded yet
- FirmPtr: a reference to an Asset that must be loaded and can be accessed through the FirmPtr
- Manager: Source of Asset pointers with a loading/unloading policy. Gives a unique Asset per each seed registration.
- Cacher: Source of const Asset pointers with a loading/unloading policy. Recycles Assets per equal seeds.
- Keeper: Source of Asset pointers with an unloading policy. Creates a new Asset immediately per each seed given.
- Standalone pointer: Pointer to an immediately constructed object independant from any managers.

# Warnings
Important: Destroying all asset instances instances before their manager's destruction is mandatory! The best way to do that is to manually call `clean(<max number>)` method of the manager-like instances before *their* destruction. Of course, it is neccessary to ensure all pointers to a resource were destroyed before that. In debug mode, **the library fails an assertion otherwise**.

Why such a strict requirement?

It would be possible to only require not having any pointers to managed resources before their managers' destruction, and cleaning cached instances automagically. However, this design was proven problematic in cases where different asset types from different managers reference one another since it's hard to keep the order of destruction that cleans everything properly.

Since all managers, cachers and keepers inherit from the `AbstractPool` class,
it's best to call clean in a loop:
```cpp
std::size_t totalCleaned = 0;
do {
    for (AbstractPool& pool : allPools) {
        totalCleaned += pool.cleanAll();
    }
} while (totalCleaned > 0);
```

This approach should clean everything, unless cyclic references exist.
In that case, fix your design.


# Installing
Currently CMake install doesn't copy headers, 
so just copy the `include/dynasma` folder to your favourite include location.