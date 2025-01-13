#define CATCH_CONFIG_MAIN
#include "TPG/TPG.h"
#include <catch2/catch_test_macros.hpp>

class TestRegisterMachine : public RegisterMachine {
  public:
   TestRegisterMachine(int size)
       : RegisterMachine(-1, params_, state_, rng, ops_) {
      for (int i = 0; i < size; i++) {
         instructions_.push_back(new Instruction(i));
      }
   }
};

TEST_CASE("RegisterMachineCrossover Basic Properties", "[crossover]") {
   TPG tpg;

   SECTION("Crossover preserves minimum program length") {
      TestRegisterMachine p1(5);
      TestRegisterMachine p2(7);
      RegisterMachine *c1, *c2;

      tpg.RegisterMachineCrossover(&p1, &p2, &c1, &c2);

      REQUIRE(c1->instructions_.size() >= 1);
      REQUIRE(c2->instructions_.size() >= 1);
   }
}