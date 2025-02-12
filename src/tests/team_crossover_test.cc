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
   params["max_team_size"] = 5;
   params["pmx_p"] = 0.5;
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

TEST_CASE("Team Crossover Tests", "[TPG]") {
   TPG tpg;
   std::unordered_map<string, std::any> params = createDefaultParams();
   std::unordered_map<string, int> state = createDefaultState();
   tpg.params_ = params;
   tpg.state_ = state;
   std::vector<bool> legal_ops(instruction::NUM_OP, true);
   tpg._ops = legal_ops;
   std::mt19937 rng(42);

   SECTION("Single Program Teams - Linear Crossover") {
      // Create two parent teams with single programs
      team* parent1 = new team(0, state["team_count"]++);
      team* parent2 = new team(0, state["team_count"]++);

      // Add programs to parents
      RegisterMachine* prog1 =
          new RegisterMachine(-1, params, state, rng, legal_ops);
      RegisterMachine* prog2 =
          new RegisterMachine(-1, params, state, rng, legal_ops);

      // Add some instructions
      for (int i = 0; i < 5; i++) {
         prog1->instructions_.push_back(new instruction(params, rng));
         prog2->instructions_.push_back(new instruction(params, rng));
      }

      parent1->AddProgram(prog1);
      parent2->AddProgram(prog2);

      team* child = tpg.TeamCrossover(parent1, parent2);

      REQUIRE(child != nullptr);
      CHECK(child->size() == 1);
      CHECK(child->n_atomic_ >= 0);

      delete parent1;
      delete parent2;
      delete child;
   }

   SECTION("Multi-Program Teams - Team Crossover") {
      team* parent1 = new team(0, state["team_count"]++);
      team* parent2 = new team(0, state["team_count"]++);

      // Add multiple programs to parents
      for (int i = 0; i < 3; i++) {
         RegisterMachine* prog1 =
             new RegisterMachine(-1, params, state, rng, legal_ops);
         RegisterMachine* prog2 =
             new RegisterMachine(-1, params, state, rng, legal_ops);
         parent1->AddProgram(prog1);
         parent2->AddProgram(prog2);
      }

      team* child = tpg.TeamCrossover(parent1, parent2);

      REQUIRE(child != nullptr);
      CHECK(child->size() <= std::any_cast<int>(params["max_team_size"]));
      CHECK(child->n_atomic_ >= 1);

      delete parent1;
      delete parent2;
      delete child;
   }

   SECTION("Atomic Program Preservation") {
      team* parent1 = new team(0, state["team_count"]++);
      team* parent2 = new team(0, state["team_count"]++);

      // Add one atomic and one non-atomic program to each parent
      RegisterMachine* atomic1 =
          new RegisterMachine(-1, params, state, rng, legal_ops);
      RegisterMachine* nonatomic1 =
          new RegisterMachine(1, params, state, rng, legal_ops);
      RegisterMachine* atomic2 =
          new RegisterMachine(-2, params, state, rng, legal_ops);
      RegisterMachine* nonatomic2 =
          new RegisterMachine(2, params, state, rng, legal_ops);

      parent1->AddProgram(atomic1);
      parent1->AddProgram(nonatomic1);
      parent2->AddProgram(atomic2);
      parent2->AddProgram(nonatomic2);

      team* child = tpg.TeamCrossover(parent1, parent2);

      REQUIRE(child != nullptr);
      CHECK(child->n_atomic_ >= 1);

      delete parent1;
      delete parent2;
      delete child;
   }

   SECTION("Team Size Limits") {
      team* parent1 = new team(0, state["team_count"]++);
      team* parent2 = new team(0, state["team_count"]++);

      // Add maximum number of programs to parents
      for (int i = 0; i < std::any_cast<int>(params["max_team_size"]); i++) {
         RegisterMachine* prog1 =
             new RegisterMachine(-1, params, state, rng, legal_ops);
         RegisterMachine* prog2 =
             new RegisterMachine(-1, params, state, rng, legal_ops);
         parent1->AddProgram(prog1);
         parent2->AddProgram(prog2);
      }

      team* child = tpg.TeamCrossover(parent1, parent2);

      REQUIRE(child != nullptr);
      CHECK(child->size() <= std::any_cast<int>(params["max_team_size"]));
      CHECK(child->n_atomic_ >= 1);

      delete parent1;
      delete parent2;
      delete child;
   }
}