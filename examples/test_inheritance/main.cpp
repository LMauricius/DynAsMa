#include "dynasma/core_concepts.hpp"
#include "dynasma/managers/basic.hpp"

#include <iostream>

class TestAssetBase : public dynasma::PolymorphicBase {
    std::string m_name;

  public:
    TestAssetBase(std::string name) : m_name(name) {
        std::cout << "--- TestAsset " << name << " constructed!" << std::endl;
    }
    ~TestAssetBase() { std::cout << "--- TestAsset destructed!" << std::endl; }

    std::string name() const { return m_name; }

    virtual std::size_t memory_cost() const = 0;
};

class TestAssetDerived : public TestAssetBase {
    std::string m_name;

  public:
    TestAssetDerived(std::string name) : TestAssetBase(name + " derived") {}

    std::size_t memory_cost() const {
        return sizeof(TestAssetDerived) + m_name.capacity(); // estimate!!
    }
};

struct TestSeed {
    using Asset = TestAssetBase;
    std::variant<std::string> kernel;

    std::size_t load_cost() const { return 1; }
    std::string name() const { return std::get<std::string>(kernel); }

    bool operator<(const TestSeed &other) const {
        return name() < other.name();
    }
};

int main() {
    dynasma::BasicManager<TestSeed, std::allocator<TestAssetDerived>> mgr;

    TestSeed seed{"<My asset 1>"};

    auto weakPtr = mgr.register_asset(seed);
    auto strongPtr = weakPtr.getLoaded();

    return 0;
}
