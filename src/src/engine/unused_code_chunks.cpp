/****************************************************************************/
// // linear crossover with two parent programs (i.e. two teams with one program
// // each)
// if (crossover && GetParam<double>("p_atomic") == 1.0 && pm1->size() == 1 &&
//     pm2->size() == 1 &&
//     real_dist_(_rngs[TPG_SEED]) <
//         GetParam<double>("p_instructions_xover")) {
//   phylo_graph_[(*cm)->id_].ancestorIds.insert(pm2->id_);
//   (*cm)->addAncestorId(pm2->id_);

//   linearCrossover = true;
//   vector<program *> pm1Programs;
//   pm1->members(pm1Programs);
//   vector<program *> pm2Programs;
//   pm1->members(pm2Programs);
//   linearM *c1;  // = NULL;
//   linearM *c2;  // = NULL;
//   programCrossover(dynamic_cast<linearM *>(pm1Programs[0]),
//                    dynamic_cast<linearM *>(pm2Programs[0]), &c1, &c2,
//                    _rngs[TPG_SEED]);

//   cm2 = new team(GetState("t_current"), state_["team_count"]++);
//   numNewTeams++;

//   phylo_graph_[(cm2)->id_].ancestorIds.insert(pm1->id_);
//   (cm2)->addAncestorId(pm1->id_);
//   phylo_graph_[(cm2)->id_].ancestorIds.insert(pm2->id_);
//   (cm2)->addAncestorId(pm2->id_);

//   if (real_dist_(_rngs[TPG_SEED]) < 0.5) {
//     (*cm)->addProgram(c1);
//     (cm2)->addProgram(c2);
//   } else {
//     (*cm)->addProgram(c2);
//     (cm2)->addProgram(c1);
//   }

//   addTeam(cm2);
//   _Mroot.insert(cm2);
//   phylo_graph_.insert(pair<long, phyloRecord>(cm2->id_, phyloRecord()));
//   phylo_graph_[cm2->id_].gtime = GetState("t_current");
//   phylo_graph_[cm2->id_].root = true;
// }
/****************************************************************************/

/****************************************************************************/
// // team crossover
// if (crossover && (pm1->size() > 1 || pm2->size() > 1)) {
//   phylo_graph_[(*cm)->id_].ancestorIds.insert(pm2->id_);
//   (*cm)->addAncestorId(pm2->id_);

//   pm2->getMembersRef(p2programs);
//   auto p2liter = p2programs->begin();
//   while (p1liter != p1programs->end() || p2liter != p2programs->end()) {
//     if (p1liter != p1programs->end() &&
//         (int)(*cm)->size() < GetParam<int>("max_team_size") &&
//         (((*p1liter)->action_ < 0 && (*cm)->numAtomic_ < 1) ||
//          find(p2programs->begin(), p2programs->end(), *p1liter) !=
//              p2programs->end()))
//       (*cm)->addProgram(*p1liter);
//     else if ((int)(*cm)->size() < GetParam<int>("max_team_size") &&
//              p1liter != p1programs->end() &&
//              real_dist_(_rngs[TPG_SEED]) < 0.5)
//       (*cm)->addProgram(*p1liter);
//     if ((int)(*cm)->size() < GetParam<int>("max_team_size") &&
//         p2liter != p2programs->end() &&
//         real_dist_(_rngs[TPG_SEED]) < 0.5)
//       (*cm)->addProgram(*p2liter);
//     if (p1liter != p1programs->end()) p1liter++;
//     if (p2liter != p2programs->end()) p2liter++;
//   }
//   if ((*cm)->numAtomic_ < 1)
//     die(__FILE__, __FUNCTION__, __LINE__,
//         "Crossover must leave the fail-safe atomic program!");
// } else
//   for (p1liter = p1programs->begin(); p1liter != p1programs->end();
//   p1liter++)
//     (*cm)->addProgram(*p1liter);
/****************************************************************************/

// if (instructions_.size() != instructions_effective_.size()) {
//   cerr << endl << "dbg Mark instructions_s " << instructions_.size() << " bidE_s "
//        << instructions_effective_.size() << endl;

//   cerr << "instructions_ exec" << endl;
//   for (auto i : instructions_) {
//     i->exec(true);
//     cerr << endl;
//   }
//   cerr << "exec done" << endl;

//   cerr << "bidE_ exec" << endl;
//   for (auto i : instructions_effective_) {
//     i->exec(true);
//     cerr << endl;
//   }
//   cerr << "exec done" << endl;
// }

/******************************************************************************/
double linearM::run(state *obs, int timeStep, int graphDepth, mt19937 &rng) {
  (void)rng;
  bool dbg = false;
  // ofstream dbg_file("prog-"+std::to_string(id_) + ".txt");
  // if (dbg) dbg_file << "id: " << id_ << " run:" << endl;

  // reset memory
  if (!stateful_) CopySharedConstToWorking();

  private_memory_[memoryEigen::kScalarType_]->working_memory_[0].setZero();
  private_memory_[memoryEigen::kScalarType_]->working_memory_[1].setZero();

  for (auto istr : instructions_effective_) {
    // read inputs
    size_t idx = 0;
    for (size_t in = 0; in < 2; in++) {
      if (istr->inType(in) != memoryEigen::kNAType) {
        if (istr->IsInput(in)) {
          // if (dbg) dbg_file << "in" << in << " fRef ";
          // in this case inMem(in) will be inputMemory_ and we use index 0
          if (istr->inType(in) == memoryEigen::kScalarType_)
            istr->inMem(in)->working_memory_[idx](0, 0) =
                obs->stateValueAtIndex(istr->inIdx(in));
          else if (istr->inType(in) == memoryEigen::kVectorType_)
            for (size_t f = istr->inIdx(in), row = 0;
                 row < istr->inMem(in)->memoryRows(); row++)
              istr->inMem(in)->working_memory_[idx](row, 0) =
                  obs->stateValueAtIndex(
                      f++ % num_input_);  //(*feature)[f++ % num_input_];
          else if (istr->inType(in) == memoryEigen::kMatrixType_)
            for (size_t f = istr->inIdx(in), row = 0;
                 row < istr->inMem(in)->memoryRows(); row++)
              for (size_t col = 0; col < istr->inMem(in)->memoryCols(); col++)
                istr->inMem(in)->working_memory_[idx](row, col) =
                    obs->stateValueAtIndex(f++ % num_input_);
          istr->inIdxE(in, idx);  // reset inIdxE to zero for input ref
        } else {                  // this input is a memory ref
          // if (dbg) dbg_file << "in" << in << " mRef ";
          // track read time for temporal memory
          istr->inMem(in)->getReadTimeE()(istr->inIdx(in), 0) =
              timeStep + (graphDepth / MAX_GRAPH_DEPTH);
        }
      }
    }
    // if (dbg) dbg_file << endl;
    // track write times for temporal memory
    istr->out_->getWriteTimeE()(istr->outIdx_, 0) =
        timeStep + (graphDepth / MAX_GRAPH_DEPTH);

    istr->exec(dbg);
  }
  // if (dbg) {
  //   dbg_file << "id: " << id_ << " outs ";
  //   dbg_file << private_memory_[memoryEigen::kScalarType_]
  //               ->working_memory_[0](0, 0);
  //   dbg_file << " ";
  //   dbg_file << private_memory_[memoryEigen::kScalarType_]
  //               ->working_memory_[1](0, 0);
  //   dbg_file << endl;
  // }
  // dbg_file.close();
  return private_memory_[memoryEigen::kScalarType_]->working_memory_[0](0, 0);
}

// /******************************************************************************
//  * Markus F. Brameier and Wolfgang Banzhaf. 2010.
//  * Linear Genetic Programming (1st. ed.). Springer Publishing Company, Inc.
//  * Algorithm 3.1 (detection of structural introns)
//  */
// void linearM::MarkIntrons(std::unordered_map<std::string, std::any> &params)
// {
//   fill(op_counts_.begin(), op_counts_.end(), 0);  // count occurance of each
//   op

//   // keep track of which memories are effective with a map
//   map<int, vector<bool> > Reff;  // maps [memory type][index]->true/false

//   for (int mem_t = 0; mem_t < memoryEigen::kNumMemoryType_; mem_t++)
//     Reff[mem_t] =
//         vector<bool>(std::any_cast<int>(params["n_memories"]), false);

//   Reff[memoryEigen::kScalarType_][0] = true;  // mark bid output memory

//   if (std::any_cast<int>(params["continuous_output"]))
//     Reff[memoryEigen::kScalarType_][1] = true;  // mark continuous output
//     memory

//   features_.clear();
//   instructions_effective_.clear();

//   // From last to first instruction.
//   // vector<instruction *>::reverse_iterator riter;
//   for (auto riter = instructions_.rbegin(); riter != instructions_.rend(); riter++) {
//     if (!skipIntrons_ || Reff[(*riter)->outType()][(*riter)->outIdx_]) {
//       instructions_effective_.insert(instructions_effective_.begin(), *riter);
//       op_counts_[(*riter)->op_]++;
//       // output TODO(spkelly) this is always true now
//       (*riter)->out_ = private_memory_[(*riter)->outType()];
//       // inputs
//       for (int in = 0; in < 2; in++) {
//         // if this input is actually used for this op
//         if ((*riter)->inType(in) != memoryEigen::kNAType) {
//           if ((*riter)->IsInput(in)) {  // this input is a feature ref
//             (*riter)->inMem(in,
//             inputMemoryPointers_[in][(*riter)->inType(in)]);
//             // mark features (accounting, should have no effect behaviour)
//             if ((*riter)->inType(in) == memoryEigen::kScalarType_) {
//               features_.insert((*riter)->inIdx(in));
//             } else if ((*riter)->inType(in) == memoryEigen::kVectorType_) {
//               for (size_t f = (*riter)->inIdx(in), row = 0;
//                    row < (*riter)->inMem(in)->memoryRows(); row++) {
//                 features_.insert(f++ % num_input_);  // toroidal
//               }
//             } else if ((*riter)->inType(in) == memoryEigen::kMatrixType_) {
//               for (size_t f = (*riter)->inIdx(in), row = 0;
//                    row < (*riter)->inMem(in)->memoryRows(); row++) {
//                 for (size_t col = 0; col < (*riter)->inMem(in)->memoryCols();
//                      col++) {
//                   features_.insert(f++ % num_input_);  // toroidal
//                 }
//               }
//             }
//           } else {  // this input is a memory ref
//             (*riter)->inMem(in,
//             private_memory_[(*riter)->inType(in)]);
//             Reff[(*riter)->inType(in)][(*riter)->inIdx(in)] = true;
//           }
//         }
//       }
//     }
//   }
// }