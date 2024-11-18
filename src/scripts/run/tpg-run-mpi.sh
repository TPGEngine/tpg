#!/bin/bash

# Default command line args
mode=0 #Train:0, Replay:1, Debug:2
num_mpi_proc=2
seed_tpg=42
seed_aux=42
task_to_replay=0
replay_gen=0
tm_id=0

while getopts g:m:n:r:s:T:t: flag
do
   case "${flag}" in
      g) seed_aux=${OPTARG};;
      m) mode=${OPTARG};;
      n) num_mpi_proc=${OPTARG};;
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
     $TPG/build/release/cpp/experiments/TPGExperimentMPI \
     seed_tpg=${seed_tpg} \
     1> tpg.$seed_tpg.$$.std \
     2> tpg.$seed_tpg.$$.err &
fi

# Replay mode ##################################################################
if [ $mode -eq 1 ]; then
   # Training phase
   phase=0
   if ls replay/frames/* 1> /dev/null 2>&1; then rm replay/frames/*; fi
   if ls rplay/graphs/* 1> /dev/null 2>&1; then rm replay/graphs/*; fi
   if ls rplay/graphs/* 1> /dev/null 2>&1; then rm replay/graphs/*; fi
  
  
   # Get fitness of best team
   best_fitness=$(grep setElTmsMTA  tpg.${seed_tpg}.*.std | \
     grep " fm 0 " | \
     grep " phs $phase " | \
     awk -F"mnOut" '{print $2}' | \
     awk -F "p${phase}t${task_to_replay}a0 " '{print $2}' | \
     awk '{print $1}' | \
     sort -n | \
     uniq | \
     tail -n 1)

   # Get generation of best team
   checkpoint_in_t=$(grep setElTmsMTA tpg.${seed_tpg}.*.std | \
     grep " fm 0 " | \
     grep "p${phase}t${task_to_replay}a0 ${best_fitness} " | \
     grep " phs $phase " | \
     head -n 1 | \
     awk -F" t " '{print $2}' | \
     awk '{print $1}')
   
   # Get id of best team
   tm_id=$(grep "setElTmsMTA" tpg.${seed_tpg}.*.std | \
     grep " fm 0 " | \
     grep "p${phase}t${task_to_replay}a0 ${best_fitness} " | \
     grep " phs $phase " | \
     grep " t $checkpoint_in_t " | \
     head -n 1 | \
     awk -F"id" '{print $2}' | \
     awk '{print $1}')

   echo "Fitness:$best_fitness Generation:$checkpoint_in_t Team:$tm_id"
   
   #  for dbg mpirun --oversubscribe -np 1 xterm -hold -e gdb -ex run --args \
   mpirun --oversubscribe \
     -np 1 $TPG/build/release/cpp/experiments/TPGExperimentMPI \
     replay=1 animate=1 id_to_replay=$tm_id task_to_replay=$task_to_replay \
     checkpoint_in_phase=$phase checkpoint_in_t=$checkpoint_in_t \
     seed_tpg=$seed_tpg seed_aux=$seed_aux \
     1> tpg.$seed_tpg.$seed_aux.replay.std \
     2> tpg.$seed_tpg.$seed_aux.replay.err &
fi

# Debug mode ###################################################################
if [ $mode -eq 2 ]; then
   mpirun --oversubscribe -np $num_mpi_proc xterm -hold -e gdb -ex run \
     --args $TPG/build/release/cpp/experiments/TPGExperimentMPI \
     seed_tpg=${seed_tpg} \
     1> tpg.$seed_tpg.$$.std \
     2> tpg.$seed_tpg.$$.err &
fi

# Valgrind #####################################################################
if [ $mode -eq 3 ]; then
  mpirun --oversubscribe -np $num_mpi_proc valgrind --leak-check=yes \ 
    --show-reachable=yes --log-file=vg.%p \ 
    --suppressions=/usr/share/openmpi/openmpi-valgrind.supp \ 
    $TPG/build/release/cpp/experiments/TPGExperimentMPI seed_tpg=${seed_tpg} \ 
    1> tpg.$seed_tpg.$$.std \ 
    2> tpg.$seed_tpg.$$.err &
fi

# below this line is just sketches to be cleaned ###############################

# # Check for memoy leaks
# if [ $mode -eq 3 ]; then
# ##view profile with: google-pprof ../build/release/cpp/experiments/tpgExpBlocks_MPI ./tpg.out_27134   
# ##google-pprof --gv --focus=genTeams ../build/release/cpp/experiments/TPGExperimentMPI tpg.out_441771
# #LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libprofiler.so CPUPROFILE=tpg.out \
# #mpirun --oversubscribe -np $num_mpi_proc ../build/release/cpp/experiments/TPGExperimentMPI -w -F $numFitMode -D $dim -p -S -T $tMax -s $seedTPG -g $seedEnv -a $activeTasks -d $taskSwitchMod -f $fitMode 1> tpg.$seedTPG.$$.std 2> tpg.$seedTPG.$$.err &

# #valgrind
# mpirun --oversubscribe -np $num_mpi_proc valgrind --leak-check=yes --show-reachable=yes --log-file=vg.%p --suppressions=/usr/share/openmpi/openmpi-valgrind.supp \
# ../build/release/cpp/experiments/TPGExperimentMPI -s $seed 1> tpg.$seed.$$.std 2> tpg.$seed.$$.err &
# fi

# #if [ $mode -eq 4 ]; then
# ##pickup from checkpoint file
# #t=$(grep -iRl end checkpoints/cp.*.-1.$seedTPG.0.rslt | cut -d '.' -f 2 | sort -n | tail -n 2 | head -n 1)
# ##sed  -i "1,/genTime\ t\ $t/!d" tpg.$seedTPG.std
# #pid=$(ls tpg.$seedTPG.*.std | cut -d '.' -f 4 | tail -n 1)
# #echo "pid $pid"
# #echo "Starting run seedTPG $seedTPG t $t"
# #LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libprofiler.so CPUPROFILE=tpg.out \
# #mpirun --oversubscribe -np $num_mpi_proc ../build/release/cpp/experiments/TPGExperimentMPI -s $seedTPG 1>> tpg.$seedTPG.$pid.std 2>> tpg.$seedTPG.$pid.err &
# ##mpirun --oversubscribe -np $num_mpi_proc xterm -hold -e gdb -ex run --args ../build/release/cpp/experiments/TPGExperimentMPI -C 0 -t $t -w -F $numFitMode -D $dim -p -S -T $tMax -s $seedTPG -g $seedEnv -d $taskSwitchMod -a $activeTasks 1>> tpg.$seedTPG.$pid.std 2>> tpg.$seedTPG.$pid.err &
# #fi
