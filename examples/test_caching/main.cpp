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

    dynasma::LazyPtr<TestAsset> lazyPtr_a, lazyPtr_b;

    {
        auto firmPtr1 = cacher.retrieve_asset(seed1).getLoaded();
        auto firmPtr2 = cacher.retrieve_asset(seed2).getLoaded();
        auto firmPtr3 = cacher.retrieve_asset(seed1).getLoaded();

        std::cout << "firmPtr1 === firmPtr2: "
                  << (&*firmPtr1 == &*firmPtr2) << std::endl;

        std::cout << "firmPtr1 === firmPtr3: "
                  << (&*firmPtr1 == &*firmPtr3) << std::endl;

        lazyPtr_a = firmPtr1;
    }

    {
        auto firmPtr1 = cacher.retrieve_asset(seed1).getLoaded();
        auto firmPtr2 = cacher.retrieve_asset(seed2).getLoaded();
        auto firmPtr3 = cacher.retrieve_asset(seed1).getLoaded();

        std::cout << "firmPtr1 === firmPtr2: "
                  << (&*firmPtr1 == &*firmPtr2) << std::endl;

        std::cout << "firmPtr1 === firmPtr3: "
                  << (&*firmPtr1 == &*firmPtr3) << std::endl;

        lazyPtr_b = firmPtr1;
    }

    auto firmPtr_a = lazyPtr_a.getLoaded();
    auto firmPtr_b = lazyPtr_b.getLoaded();

    std::cout << "firmPtr_a === firmPtr_b: "
              << (&*firmPtr_a == &*firmPtr_b) << std::endl;

    return 0;
}