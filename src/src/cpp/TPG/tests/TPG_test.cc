#define CATCH_CONFIG_MAIN

#include <Instruction.h>
#include <RegisterMachine.h>
#include <TPG.h>
#include <any>
#include <catch2/catch_test_macros.hpp>
#include <random>
#include <unordered_map>
#include <vector>

class TestRegisterMachine : public RegisterMachine {
  public:
   static RegisterMachine* createMockRegisterMachine(int size) {
      std::unordered_map<std::string, std::any> params;
      std::unordered_map<std::string, int> state;
      std::mt19937 rng;
      std::vector<bool> ops;

      // Initialize parameters and state with mock values
      params["stateful"] = 1;
      params["observation_buff_size"] = 10;
      params["max_initial_prog_size"] = size;
      params["p_instructions_mu_const"] = 0.1;
      params["n_memories"] = 5;
      params["memory_size"] = 10;

      state["t_current"] = 0;
      state["program_count"] = 0;

      ops.resize(instruction::NUM_OP,
                 true);  // Assuming all operations are legal

      // Create a new RegisterMachine instance
      return new RegisterMachine(-1, params, state, rng, ops);
   }
};

TEST_CASE("RegisterMachineCrossover Basic Properties", "[crossover]") {
   TPG tpg;

   SECTION("Crossover preserves minimum program length") {
      RegisterMachine* p1 = TestRegisterMachine::createMockRegisterMachine(5);
      RegisterMachine* p2 = TestRegisterMachine::createMockRegisterMachine(7);
      RegisterMachine *c1, *c2;

      tpg.RegisterMachineCrossover(p1, p2, &c1, &c2);

      REQUIRE(c1->instructions_.size() >= 1);
      REQUIRE(c2->instructions_.size() >= 1);

      // Clean up
      delete p1;
      delete p2;
      delete c1;
      delete c2;
   }
}