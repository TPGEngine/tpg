#define CATCH_CONFIG_MAIN

#include <RegisterMachine.h>
#include <TPG.h>
#include <instruction.h>
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
   TPG tpg;

   std::unordered_map<string, std::any> params = createDefaultParams();
   std::unordered_map<string, int> state = createDefaultState();
   tpg.params_ = params;
   tpg.state_ = state;

   std::vector<bool> legal_ops(instruction::NUM_OP, true);
   tpg._ops = legal_ops;
   std::mt19937 rng(40);

   SECTION("Test basic crossover functionality") {
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
 

      // Check that children have valid actions
      CHECK((child1->action_ == parent1->action_));
      CHECK((child2->action_ == parent1->action_));

      // Verify children's instructions come from both parents
      bool has_different_instructions = false;
      for (size_t i = 0; i < child1->instructions_.size(); i++) {
         if (i < child2->instructions_.size()) {
            if (child1->instructions_[i] != child2->instructions_[i]) {
               has_different_instructions = true;
               break;
            }
         }
      }

      CHECK(has_different_instructions);

      // Cleanup
      delete parent1;
      delete parent2;
      delete child1;
      delete child2;
   }

   SECTION("Test chunk splitting and recombination") {
      tpg.Seed(TPG_SEED, 42);

      // Create parent machines with known instruction sequences
      RegisterMachine* parent1 =
          new RegisterMachine(-1, params, state, rng, legal_ops);
      RegisterMachine* parent2 =
          new RegisterMachine(-2, params, state, rng, legal_ops);

      // Add some test instructions to parents
      for (int i = 0; i < 6; i++) {
         parent1->instructions_.push_back(new instruction(params, rng));
         parent2->instructions_.push_back(new instruction(params, rng));
      }

      RegisterMachine* child1 = nullptr;
      RegisterMachine* child2 = nullptr;

      tpg.RegisterMachineCrossover(parent1, parent2, &child1, &child2);

      SECTION("Verify chunk creation") {
         // Check children were created
         REQUIRE(child1 != nullptr);
         REQUIRE(child2 != nullptr);

         // Check instruction counts
         CHECK(child1->instructions_.size() > 0);
         CHECK(child2->instructions_.size() > 0);
         CHECK(child1->instructions_.size() <=
               parent1->instructions_.size() + parent2->instructions_.size());
         CHECK(child2->instructions_.size() <=
               parent1->instructions_.size() + parent2->instructions_.size());

         // Verify children have different instruction patterns
         bool has_different_instructions = false;
         for (size_t i = 0; i < child1->instructions_.size(); i++) {
            if (i < child2->instructions_.size()) {
               if (child1->instructions_[i] != child2->instructions_[i]) {
                  has_different_instructions = true;
                  break;
               }
            }
         }
         CHECK(has_different_instructions);
      }

      // Cleanup
      delete parent1;
      delete parent2;
      delete child1;
      delete child2;
   }


   SECTION("REST"){
      // Initialize TPG and RegisterMachines
      RegisterMachine* p1 =
          new RegisterMachine(-1, params, state, rng, legal_ops);
      RegisterMachine* p2 =
          new RegisterMachine(-1, params, state, rng, legal_ops);

      // Add some instructions to p1 and p2
      for (int i = 0; i < 10; ++i) {
         p1->instructions_.push_back(new instruction(params, rng));
         p2->instructions_.push_back(new instruction(params, rng));
      }

      RegisterMachine* c1 = nullptr;
      RegisterMachine* c2 = nullptr;
      tpg.RegisterMachineCrossover(p1, p2, &c1, &c2);
      int p1Size = static_cast<int>(p1->instructions_.size());
      int p2Size = static_cast<int>(p2->instructions_.size());

      int c1Size = static_cast<int>(c1->instructions_.size());
      int c2Size = static_cast<int>(c2->instructions_.size());


      // Values subject to change
      int dcmax = 25;
      int lsmax = 30;
      int dsmax = 20;  
      int lmin = 5;   
      int lmax = 30;  

      SECTION(
          "Step 1: Randomly select crossover points within distance dcmax") {

         // Ensure crossover points are within the allowed distance, // Example: dcmax = 25

         // distance | i1 − i2 |≤ min(l(gp1) − 1, dcmax) 
         INFO(p1Size);
         INFO(p2Size);

         INFO(c1Size);
         INFO(c2Size);
         REQUIRE(std::abs(c1Size - c2Size) <= dcmax);
      }

      SECTION("Step 2: Select segment lengths within lsmax") {
         

         // Ensure segment lengths are within the allowed maximum
         REQUIRE(c1Size <= lsmax);
         REQUIRE(c2Size <= lsmax);
      }

      SECTION("Step 3: Reselect segment length if difference exceeds dsmax") {
         

         // Ensure the difference in segment lengths is within the allowed maximum
         REQUIRE(std::abs(c1Size-c2Size) <= dsmax);
      }

      SECTION("Step 4: Ensure l(s1) <= l(s2)") {
         

         // Ensure the first segment is not longer than the second
         REQUIRE(c1Size <= c2Size);
      }

      SECTION(
          "Step 5: Adjust segment lengths if resulting programs are out of "
          "bounds") {
         

         // Ensure the resulting programs are within the allowed length bounds

         REQUIRE(c1Size >= lmin);
         REQUIRE(c1Size <= lmax);
         REQUIRE(c2Size >= lmin);
         REQUIRE(c2Size <= lmax);
      }

  



      // Clean up
      delete p1;
      delete p2;
      delete c1;
      delete c2;
   }
}
