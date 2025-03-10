#!/bin/bash

mkdir -p logs

# Default command line args
mode=0 #Train:0, Replay:1, Debug:2
animate=1
num_mpi_proc=2
seed_tpg=42
seed_aux=42
task_to_replay=0
replay_gen=0
tm_id=0
checkpoint_in_phase=0
min_fitness=-1000000

# TODO(skelly): change to full name parameters
while getopts a:c:g:m:n:p:r:s:T:t: flag
do
   case "${flag}" in
      a) animate=${OPTARG};;
      c) checkpoint_in_phase=${OPTARG};;
      g) seed_aux=${OPTARG};;
      m) mode=${OPTARG};;
      n) num_mpi_proc=${OPTARG};;
      p) parameters_file="${OPTARG}";;
      s) seed_tpg=${OPTARG};;
      T) tm_id=${OPTARG};;
      t) replay_gen=${OPTARG};;
      r) task_to_replay=${OPTARG};;
   esac
done

# Evolve mode ##################################################################
if [ $mode -eq 0 ]; then
   echo "Starting run $seed_tpg..."
   mpirun --oversubscribe -np $num_mpi_proc \
     $TPG/build/release/experiments/TPGExperimentMPI \
     parameters_file=${parameters_file} \
     seed_tpg=${seed_tpg} pid=$$ \
     1> logs/tpg.$seed_tpg.$$.std \
     2> logs/tpg.$seed_tpg.$$.err &
fi

# Replay mode ##################################################################
if [ $mode -eq 1 ]; then
   min_fitness=-15
   # Training phase
   phase=0
   if ls replay/frames/* 1> /dev/null 2>&1; then rm replay/frames/*; fi
   if ls replay/graphs/* 1> /dev/null 2>&1; then rm replay/graphs/*; fi
   if ls replay/graphs/* 1> /dev/null 2>&1; then rm replay/graphs/*; fi

  # Get fitness of best team
   best_fitness=$(grep setElTmsMTA  logs/tpg.${seed_tpg}.*.std | \
     grep " fm 0 " | \
     grep " phs $phase " | \
     awk -F "p${phase}t${task_to_replay}a0 " '{print $2}' | \
     awk '{print $1}' | \
     tail -n 1)
 
  # Get generation of best team
   best_fitness_t=$(grep setElTmsMTA  logs/tpg.${seed_tpg}.*.std | \
     grep " fm 0 " | \
     grep " phs $phase " | \
     awk -F " t " '{print $2}' | \
     awk '{print $1}' | \
     tail -n 1) 

   # Get phase and generation of checkpoint file
   checkpoint_in_phase=$(grep -iRl end \
    checkpoints/cp.*.${seed_tpg}.*.rslt | \
    cut -d '.' -f 4 | sort -n | tail -n 1)
   checkpoint_in_t=$(grep -iRl end \
    checkpoints/cp.*.${seed_tpg}.${checkpoint_in_phase}.rslt | \
    cut -d '.' -f 2 | sort -n | tail -n 1) 

   # Get id of best team
   tm_id=$(grep "setElTmsMTA" logs/tpg.${seed_tpg}.*.std | \
     grep " fm 0 " | \
     grep "p${phase}t${task_to_replay}a0 ${best_fitness} " | \
     grep " phs $phase " | \
     grep " t $best_fitness_t " | \
     head -n 1 | \
     awk -F"id" '{print $2}' | \
     awk '{print $1}')
   
   echo "Fitness:$best_fitness Generation:$checkpoint_in_t Team:$tm_id"
   
  #  mpirun --oversubscribe -np 1 xterm -hold -e gdb -ex run --args \
   mpirun --oversubscribe -np 1 \
    $TPG/build/release/experiments/TPGExperimentMPI \
    parameters_file=${parameters_file} \
    seed_tpg=${seed_tpg} seed_aux=${seed_aux} \
    start_from_checkpoint=1 \
    checkpoint_in_phase=${checkpoint_in_phase} \
    checkpoint_in_t=${checkpoint_in_t} \
    replay=1 animate=${animate} id_to_replay=${tm_id} \
    task_to_replay=${task_to_replay} \
    1> logs/tpg.${seed_tpg}.${seed_aux}.replay.std \
    2> logs/tpg.${seed_tpg}.${seed_aux}.replay.err &

fi
# fi

# Debug mode ###################################################################
if [ $mode -eq 2 ]; then
   mpirun --oversubscribe -np $num_mpi_proc xterm -hold -e gdb -ex run \
     --args $TPG/build/release/experiments/TPGExperimentMPI \
     parameters_file=${parameters_file} \
     seed_tpg=${seed_tpg} n_root=10 n_root_gen=10 \
     1> logs/tpg.$seed_tpg.$$.std \
     2> logs/tpg.$seed_tpg.$$.err &
fi

# Valgrind #####################################################################
# Note that TPG must be compiled with debugging enabled, use "scons --dbg"
if [ $mode -eq 3 ]; then
  mpirun --oversubscribe -np 2 \
  valgrind --leak-check=yes --show-reachable=yes \
  --log-file=vg.%p --suppressions=/usr/share/openmpi/openmpi-valgrind.supp \
  $TPG/build/release/experiments/TPGExperimentMPI \
  parameters_file=${parameters_file} \
  seed_tpg=${seed_tpg} n_root=10 n_root_gen=10 n_generations=3 \
  mj_max_timestep=1 mj_n_eval_train=1 \
  1> logs/tpg.$seed_tpg.$$.std \
  2> logs/tpg.$seed_tpg.$$.err &

fi

# Pickup from checkpoint #######################################################
if [ $mode -eq 4 ]; then
  checkpoint_in_phase=0
  checkpoint_in_t=$(grep -iRl end \
  checkpoints/cp.*.${seed_tpg}.${checkpoint_in_phase}.rslt | \
  cut -d '.' -f 2 | sort -n | tail -n 1)
  pid=$(ls tpg.${seed_tpg}.*.std | cut -d '.' -f 3 | tail -n 1)
  echo "Starting run ${seed_tpg} t ${checkpoint_in_t} pid $pid"
  mpirun --oversubscribe -np $num_mpi_proc \
    $TPG/build/release/experiments/TPGExperimentMPI \
    parameters_file=${parameters_file} \
    seed_tpg=${seed_tpg} \
    start_from_checkpoint=1 \
    checkpoint_in_phase=${checkpoint_in_phase} \
    checkpoint_in_t=${checkpoint_in_t} \
    1>> logs/tpg.${seed_tpg}.${pid}.std \
    2>> logs/tpg.${seed_tpg}.${pid}.err &
fi

# below this line is just sketches to be cleaned ###############################
################################################################################

# # Check for memoy leaks
# if [ $mode -eq 3 ]; then
# ##view profile with: google-pprof ../build/release/experiments/tpgExpBlocks_MPI ./tpg.out_27134   
# ##google-pprof --gv --focus=genTeams ../build/release/experiments/TPGExperimentMPI tpg.out_441771
# #LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libprofiler.so CPUPROFILE=tpg.out \
# #mpirun --oversubscribe -np $num_mpi_proc ../build/release/experiments/TPGExperimentMPI -w -F $numFitMode -D $dim -p -S -T $tMax -s $seedTPG -g $seedEnv -a $activeTasks -d $taskSwitchMod -f $fitMode 1> tpg.$seedTPG.$$.std 2> tpg.$seedTPG.$$.err &

# #valgrind
# mpirun --oversubscribe -np $num_mpi_proc valgrind --leak-check=yes --show-reachable=yes --log-file=vg.%p --suppressions=/usr/share/openmpi/openmpi-valgrind.supp \
# ../build/release/experiments/TPGExperimentMPI -s $seed 1> tpg.$seed.$$.std 2> tpg.$seed.$$.err &
# fi

# #if [ $mode -eq 4 ]; then
# ##pickup from checkpoint file
# #t=$(grep -iRl end checkpoints/cp.*.-1.$seedTPG.0.rslt | cut -d '.' -f 2 | sort -n | tail -n 2 | head -n 1)
# ##sed  -i "1,/genTime\ t\ $t/!d" tpg.$seedTPG.std
# #pid=$(ls tpg.$seedTPG.*.std | cut -d '.' -f 4 | tail -n 1)
# #echo "pid $pid"
# #echo "Starting run seedTPG $seedTPG t $t"
# #LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libprofiler.so CPUPROFILE=tpg.out \
# #mpirun --oversubscribe -np $num_mpi_proc ../build/release/experiments/TPGExperimentMPI -s $seedTPG 1>> tpg.$seedTPG.$pid.std 2>> tpg.$seedTPG.$pid.err &
# ##mpirun --oversubscribe -np $num_mpi_proc xterm -hold -e gdb -ex run --args ../build/release/experiments/TPGExperimentMPI -C 0 -t $t -w -F $numFitMode -D $dim -p -S -T $tMax -s $seedTPG -g $seedEnv -d $taskSwitchMod -a $activeTasks 1>> tpg.$seedTPG.$pid.std 2>> tpg.$seedTPG.$pid.err &
# #fi
