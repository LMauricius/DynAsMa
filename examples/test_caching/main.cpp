#include "dynasma/cachers/basic.hpp"
#include "dynasma/core_concepts.hpp"

#include <iostream>

class TestAsset : public dynasma::PolymorphicBase {
    std::string m_name;

  public:
    TestAsset(std::string name) : m_name(name) {
        std::cout << "--- TestAsset " << name << " constructed!" << std::endl;
    }
    ~TestAsset() { std::cout << "--- TestAsset destructed!" << std::endl; }

    std::string name() const { return m_name; }

    std::size_t memory_cost() const {
        return sizeof(TestAsset) + m_name.capacity(); // estimate!!
    }
};

struct TestSeed {
    using Asset = TestAsset;
    std::variant<std::string> kernel;

    std::size_t load_cost() const { return 1; }
    std::string name() const { return std::get<std::string>(kernel); }

    bool operator<(const TestSeed &other) const {
        return name() < other.name();
    }
};

int main() {
    dynasma::BasicCacher<TestSeed, std::allocator<TestAsset>> cacher;

    TestSeed seed1{"<My asset 1>"}, seed2{"<My asset 2>"};

    dynasma::WeakPtr<TestAsset> weakPtr_a, weakPtr_b;

    {
        auto strongPtr1 = cacher.retrieve_asset(seed1).getLoaded();
        auto strongPtr2 = cacher.retrieve_asset(seed2).getLoaded();
        auto strongPtr3 = cacher.retrieve_asset(seed1).getLoaded();

        std::cout << "strongPtr1 === strongPtr2: "
                  << (&*strongPtr1 == &*strongPtr2) << std::endl;

        std::cout << "strongPtr1 === strongPtr3: "
                  << (&*strongPtr1 == &*strongPtr3) << std::endl;

        weakPtr_a = strongPtr1;
    }

    {
        auto strongPtr1 = cacher.retrieve_asset(seed1).getLoaded();
        auto strongPtr2 = cacher.retrieve_asset(seed2).getLoaded();
        auto strongPtr3 = cacher.retrieve_asset(seed1).getLoaded();

        std::cout << "strongPtr1 === strongPtr2: "
                  << (&*strongPtr1 == &*strongPtr2) << std::endl;

        std::cout << "strongPtr1 === strongPtr3: "
                  << (&*strongPtr1 == &*strongPtr3) << std::endl;

        weakPtr_b = strongPtr1;
    }

    auto strongPtr_a = weakPtr_a.getLoaded();
    auto strongPtr_b = weakPtr_b.getLoaded();

    std::cout << "strongPtr_a === strongPtr_b: "
              << (&*strongPtr_a == &*strongPtr_b) << std::endl;

    return 0;
}