#include "training_runner.h"
#include "metrics/timing/timing_metrics.h"
#include "core/event_dispatcher.h"

TrainingRunner::TrainingRunner(TPG& tpg, std::vector<TaskEnv*>& tasks, boost::mpi::communicator& world, std::vector<int>& taskIndices)
    : tpg_(tpg), 
      tasks_(tasks), 
      world_(world),
      taskIndices_(taskIndices)
      {}

void TrainingRunner::initialization() {
  if (tpg_.GetParam<int>("start_from_checkpoint")) {
    tpg_.ReadCheckpoint(tpg_.GetParam<int>("checkpoint_in_t"),
                         tpg_.GetParam<int>("checkpoint_in_phase"),
                         false, "");
  } else {
    tpg_.InitTeams();
  }
}

void TrainingRunner::trainingLoop() {
    // time logging
    auto startGen = chrono::system_clock::now();
    chrono::duration<double> endGen = chrono::system_clock::now() - startGen;
    auto startGenTeams = chrono::system_clock::now();
    chrono::duration<double> endGenTeams =
        chrono::system_clock::now() - startGenTeams;
    auto startSetEliteTeams = chrono::system_clock::now();
    chrono::duration<double> endSetEliteTeams;
    auto startSelTeams = chrono::system_clock::now();
    chrono::duration<double> endSelTeams;
    auto startEval = chrono::system_clock::now();
    chrono::duration<double> endEval;
    auto startChkp = chrono::system_clock::now();
    chrono::duration<double> endChkp;
    auto startMODES = chrono::system_clock::now();
    chrono::duration<double> endMODES;
    auto startReport = chrono::system_clock::now();
    chrono::duration<double> endReport;

    while (tpg_.GetState("t_current") <= tpg_.GetParam<int>("n_generations")) {
        tpg_.phylo_graph_.clear();  // TODO(skelly): add switch for phylo

        // Replacement /////////////////////////////////////////////////
        if (tpg_.GetState("t_current") > tpg_.GetState("t_start")) {
            startGenTeams = chrono::system_clock::now();
            tpg_.GenerateNewTeams();
            endGenTeams = chrono::system_clock::now() - startGenTeams;
        }

        // Evaluation //////////////////////////////////////////////////
        startEval = chrono::system_clock::now();
        tpg_.MarkEffectiveCode();
        if (tpg_.GetState("t_current") > tpg_.GetState("t_start") &&
            tpg_.HaveParam("n_sampled_tasks_for_eval")) {
            // Split tasks into evaluated and estimated
            vector<int> evalTasks, estTasks;
            SplitSet(taskIndices_, evalTasks, estTasks,
                    tpg_.GetParam<int>("n_sampled_tasks_for_eval"),
                    tpg_.rngs_[TPG_SEED]);

            // Evaluate tasks
            evaluate_main(tpg_, world_, tasks_, evalTasks);

            // Estimate remaining tasks with phylogeny
            estimate_main(tpg_, tasks_, estTasks);
        } else {
            // If first generation, evaluate on all tasks
            evaluate_main(tpg_, world_, tasks_, taskIndices_);
        }
        endEval = chrono::system_clock::now() - startEval;

        // Selection ///////////////////////////////////////////////////
        startSetEliteTeams = chrono::system_clock::now();
        tpg_.SetEliteTeams(tasks_);
        endSetEliteTeams = chrono::system_clock::now() - startSetEliteTeams;
        startSelTeams = chrono::system_clock::now();
        tpg_.SelectTeams();
        endSelTeams = chrono::system_clock::now() - startSelTeams;

        // Accounting and reporting ////////////////////////////////////
        startReport = chrono::system_clock::now();
        if (tpg_.GetParam<int>("test_mod") != 0 &&
            tpg_.GetState("t_current") % tpg_.GetParam<int>("test_mod") ==
                0) {
            // validation
            tpg_.state_["phase"] = _VALIDATION_PHASE;
            evaluate_main(tpg_, world_, tasks_, taskIndices_);
            tpg_.SetEliteTeams(tasks_);

            // test
            tpg_.state_["phase"] = _TEST_PHASE;
            evaluate_main(tpg_, world_, tasks_, taskIndices_);
            tpg_.SetEliteTeams(tasks_);

            tpg_.state_["phase"] = _TRAIN_PHASE;
        }
        endReport = chrono::system_clock::now() - startReport;

        /* MODES
            * *************************************************************/
        startMODES = chrono::system_clock::now();
        if (tpg_.GetState("t_current") == tpg_.GetState("t_start") ||
            tpg_.GetState("t_current") % MODES_T == 0)
            tpg_.updateMODESFilters(true);
        endMODES = chrono::system_clock::now() - startMODES;

        /* checkpoint
            * ********************************************************/
        startChkp = chrono::system_clock::now();
        if (tpg_.GetParam<int>("write_train_checkpoints") > 0 &&
            tpg_.GetState("t_current") %
                    tpg_.GetParam<int>("write_train_checkpoints") ==
                0) {
            // Checkpoint the entire population.
            tpg_.WriteCheckpoint(false);
        }
        if (tpg_.GetParam<int>("write_phylogeny")) {
            tpg_.printPhyloGraphDot(tpg_.GetBestTeam());
        }
        endChkp = chrono::system_clock::now() - startChkp;
        endGen = chrono::system_clock::now() - startGen;

        /* print generation timing
            * *******************************************/
        double lost = endGen.count() -
                        (endEval.count() + endGenTeams.count() +
                        endSetEliteTeams.count() + endSelTeams.count() +
                        endChkp.count() + endReport.count());

        // if (tpg_.GetParam<int>("track_experiments") &&
        //     tpg_.GetState("t_current") % tpg_.GetParam<int>("track_mod") ==
        //         0) {
        //     std::string gen = to_string(tpg_.GetState("t_current"));
        //     apiClient->LogMetric("sec", std::to_string(endGen.count()), "",
        //                         gen);
        //     apiClient->LogMetric("evl", std::to_string(endEval.count()), "",
        //                         gen);
        //     apiClient->LogMetric("gTms", std::to_string(endGenTeams.count()),
        //                         "", gen);
        //     apiClient->LogMetric(
        //         "elTms", std::to_string(endSetEliteTeams.count()), "", gen);
        //     apiClient->LogMetric("sTms", std::to_string(endSelTeams.count()),
        //                         "", gen);
        //     apiClient->LogMetric("chkp", std::to_string(endChkp.count()), "",
        //                         gen);
        //     apiClient->LogMetric("rprt", std::to_string(endReport.count()),
        //                         "", gen);
        //     apiClient->LogMetric("MDS", std::to_string(endMODES.count()), "",
        //                         gen);
        //     apiClient->LogMetric("lost", std::to_string(lost), "", gen);
        // }
        TimingMetricsBuilder builder;
        builder.with_generation(tpg_.GetState("t_current"))
            .with_total_generation_time(endGen.count())
            .with_evaluation_time(endEval.count())
            .with_generate_teams_time(endGenTeams.count())
            .with_set_elite_teams_time(endSetEliteTeams.count())
            .with_select_teams_time(endSelTeams.count())
            .with_report_time(endReport.count())
            .with_modes_time(endMODES.count())
            .with_lost_time(lost);

        TimingMetrics metrics = builder.build();
        EventDispatcher<TimingMetrics>::instance().notify(EventType::TMS, metrics);

        startGen = chrono::system_clock::now();
        if (tpg_.GetState("t_current") % PRINT_MOD == 0)
            tpg_.printOss();   
        tpg_.SanityCheck();
        tpg_.state_["t_current"]++;
    }
}

// void TrainingRunner::logGenerationMetrics() {
//   // Log and record metrics as needed.
// }

void TrainingRunner::run() {
  initialization();
  trainingLoop();
}
