#include "dynasma/core_concepts.hpp"
#include "dynasma/managers/basic.hpp"
#include "dynasma/managers/naive.hpp"

#include <iostream>

class TestAsset : public dynasma::PolymorphicBase {
  public:
    TestAsset(std::string name) {
        std::cout << "--- TestAsset " << name << " constructed!" << std::endl;
    }
    ~TestAsset() { std::cout << "--- TestAsset destructed!" << std::endl; }

    std::size_t memory_cost() const { return sizeof(TestAsset); }
};

struct TestSeed {
    using Asset = TestAsset;
    std::variant<std::string> kernel;

    std::size_t load_cost() const { return 1; }
    std::string name() const { return std::get<std::string>(kernel); }
};

// A function taking template template parameter Manager that tests it
template <template <typename, typename> typename Manager> void testManager() {
    Manager<TestSeed, std::allocator<TestAsset>> manager;

    dynasma::LazyPtr<TestAsset> lazyPtr1;
    dynasma::LazyPtr<TestAsset> lazyPtr2;

    std::cout << "Entering block...\n";
    {
        std::cout << "    Entered block!\n";

        std::cout << "    Registering asset 1...\n";
        lazyPtr1 = manager.register_asset_k("<My asset 1>");
        std::cout << "    Asset registered!\n";

        std::cout << "    Taking firm reference...\n";
        auto firmPtr = lazyPtr1.getLoaded();
        std::cout << "    Firm reference taken!\n";

        std::cout << "    Asset pointer: " << &*firmPtr << "\n";

        std::cout << "    Exiting block...\n";
    }
    std::cout << "Exited block!\n";

    std::cout << "Entering block...\n";
    {
        std::cout << "    Entered block!\n";

        std::cout << "    Registering asset 2...\n";
        lazyPtr2 = manager.register_asset_k("<My asset 2>");
        std::cout << "    Asset registered!\n";

        std::cout << "    Taking firm reference...\n";
        auto firmPtr = lazyPtr2.getLoaded();
        std::cout << "    Firm reference taken!\n";

        std::cout << "    Asset pointer: " << &*firmPtr << "\n";

        std::cout << "    Exiting block...\n";
    }
    std::cout << "Exited block!\n";

    std::cout << "Trying to clean 0 bytes..." << std::endl;
    manager.clean(0);
    std::cout << "Cleaned!\n";

    std::cout << "Trying to clean sizeof(TestAsset) bytes..." << std::endl;
    manager.clean(sizeof(TestAsset));
    std::cout << "Cleaned!\n";

    std::cout << "Trying to clean all bytes..." << std::endl;
    manager.clean(10e9);
    std::cout << "Cleaned!\n";
}

int main() {
    std::cout << "==== TESTING NaiveManager ==== " << std::endl;
    testManager<dynasma::NaiveManager>();

    std::cout << "==== TESTING BasicManager ==== " << std::endl;
    testManager<dynasma::BasicManager>();

    return 0;
}