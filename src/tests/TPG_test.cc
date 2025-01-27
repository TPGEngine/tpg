#define CATCH_CONFIG_MAIN

#include <instruction.h>
#include <RegisterMachine.h>
#include <TPG.h>
#include <catch2/catch_test_macros.hpp>
#include <random>
#include <unordered_map>
#include <vector>

// Helper function to create default parameters
std::unordered_map<string, std::any> createDefaultParams() {
   std::unordered_map<string, std::any> params;
   params["stateful"] = 1;                         
   params["observation_buff_size"] = 1;          
   params["max_initial_prog_size"] = 10;         
   params["p_memory_mu_const"] = 0.5;           
   params["n_memories"] = 8;                     
   params["memory_size"] = 8;                  
   params["continuous_output"] = 1;          
   params["n_discrete_action"] = 1;   
   return params;

}

// Helper function to create default state
std::unordered_map<string, int> createDefaultState() {
   std::unordered_map<string, int> state;
   state["t_current"] = 0;
   state["program_count"] = 0;
   state["team_count"] = 0;
   return state;
}

TEST_CASE("RegisterMachine Crossover Test", "[TPG]") {

   SECTION("TEST1") {
      CHECK(1 < 2);

      TPG tpg;
      
      // tpg.params_["id"] = -1;  // remove later
      // tpg.state_["world_rank"] = 1;
      
      std::unordered_map<string, std::any> params = createDefaultParams();
      std::unordered_map<string, int> state = createDefaultState();
      tpg.params_ = params;
      tpg.state_ = state;

      std::vector<bool> legal_ops(instruction::NUM_OP, true);
      std::mt19937 rng(40);
      INFO("Creating Register machines");
      // Create parent machines with known sizes
      RegisterMachine* parent1 =
          new RegisterMachine(-1, params, state, rng, legal_ops);
      RegisterMachine* parent2 =
          new RegisterMachine(-1, params, state, rng, legal_ops);

      // Pointers to hold children
      RegisterMachine* child1 = nullptr;
      RegisterMachine* child2 = nullptr;

      tpg.RegisterMachineCrossover(parent1, parent2, &child1, &child2);

      REQUIRE(child1 != nullptr);
      REQUIRE(child2 != nullptr);

      // Check that children have reasonable sizes
      CHECK(child1->instructions_.size() > 0);
      CHECK(child2->instructions_.size() > 0);
      CHECK(child1->instructions_.size() <=
            std::max(parent1->instructions_.size(),
                     parent2->instructions_.size()));

      CHECK(child2->instructions_.size() <=
            std::max(parent1->instructions_.size(),
                     parent2->instructions_.size()));

      // Check that children have valid actions
      CHECK((child1->action_ == parent1->action_));
      CHECK((child2->action_ == parent1->action_));

      // Cleanup
      delete parent1;
      delete parent2;
      delete child1;
      delete child2;
   }

   SECTION("TEST2") {
      // CHECK(1 < 4);
      // CHECK(2 < 4);
   }
}
