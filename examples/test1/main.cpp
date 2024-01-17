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

    const TestSeed seed1{"<My asset 1>"};
    const TestSeed seed2{"<My asset 2>"};

    dynasma::WeakPtr<TestAsset> weakPtr1;
    dynasma::WeakPtr<TestAsset> weakPtr2;

    std::cout << "Entering block...\n";
    {
        std::cout << "    Entered block!\n";

        std::cout << "    Registering asset 1...\n";
        weakPtr1 = manager.register_asset(seed1);
        std::cout << "    Asset registered!\n";

        std::cout << "    Taking strong reference...\n";
        auto strongPtr = weakPtr1.getLoaded();
        std::cout << "    Strong reference taken!\n";

        std::cout << "    Asset pointer: " << &*strongPtr << "\n";

        std::cout << "    Exiting block...\n";
    }
    std::cout << "Exited block!\n";

    std::cout << "Entering block...\n";
    {
        std::cout << "    Entered block!\n";

        std::cout << "    Registering asset 2...\n";
        weakPtr2 = manager.register_asset(seed2);
        std::cout << "    Asset registered!\n";

        std::cout << "    Taking strong reference...\n";
        auto strongPtr = weakPtr2.getLoaded();
        std::cout << "    Strong reference taken!\n";

        std::cout << "    Asset pointer: " << &*strongPtr << "\n";

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