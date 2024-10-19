#!/bin/bash

# Default command line args
mode=0 #Train:0, Replay:1, Debug:2
numMPIProc=2
seed=42
replay_task=0
replay_gen=0
tm_id=0

while getopts m:n:r:s:T:t: flag
do
   case "${flag}" in
      m) mode=${OPTARG};;
      n) numMPIProc=${OPTARG};;
      s) seed=${OPTARG};;
      T) tm_id=${OPTARG};;
      t) replay_gen=${OPTARG};;
      r) replay_task=${OPTARG};;
   esac
done

# Evolve
if [ $mode -eq 0 ]; then
   echo "Starting run $seedTPG..."
   mpirun --oversubscribe -np $numMPIProc \
     $TPG/build/release/cpp/experiments/TPGExperimentMPI -s $seed \
     1> tpg.$seed.$$.std 2> tpg.$seed.$$.err &
fi

# Replay
if [ $mode -eq 1 ]; then
   # Training phase
   phase=2
   if ls replay/frames/* 1> /dev/null 2>&1; then rm replay/frames/*; fi
   if ls rplay/graphs/* 1> /dev/null 2>&1; then rm replay/graphs/*; fi
   if ls rplay/graphs/* 1> /dev/null 2>&1; then rm replay/graphs/*; fi
   
  #  # Get fitness of best team
  #  bestScore=$(grep setElTmsST  tpg.${seed}.*.std | \
  #    grep " fm 0 " | \
  #    grep " phs $phase " | \
  #    awk -F"mnOut" '{print $2}' | \
  #    awk -F "p${phase}t${replay_task}a0 " '{print $2}' | \
  #    awk '{print $1}' | \
  #    sort -n | \
  #    uniq | \
  #    tail -n 1)

  #  # Get generation of best team
  #  t_pickup=$(grep setElTmsST tpg.${seed}.*.std | \
  #    grep " fm 0 " | \
  #    grep "p${phase}t${replay_task}a0 ${bestScore} " | \
  #    grep " phs $phase " | \
  #    head -n 1 | \
  #    awk -F" t " '{print $2}' | \
  #    awk '{print $1}')
   
  #  # Get id of best team
  #  tm=$(grep "setElTmsST" tpg.${seed}.*.std | \
  #    grep " fm 0 " | \
  #    grep "p${phase}t${replay_task}a0 ${bestScore} " | \
  #    grep " phs $phase " | \
  #    grep " t $t_pickup " | \
  #    head -n 1 | \
  #    awk -F"id" '{print $2}' | \
  #    awk '{print $1}')

  
  
   # Get fitness of best team
   bestScore=$(grep setElTmsMTA  tpg.${seed}.*.std | \
     grep " fm 0 " | \
     grep " phs $phase " | \
     awk -F"mnOut" '{print $2}' | \
     awk -F "p${phase}t${replay_task}a0 " '{print $2}' | \
     awk '{print $1}' | \
     sort -n | \
     uniq | \
     tail -n 1)

   # Get generation of best team
   t_pickup=$(grep setElTmsMTA tpg.${seed}.*.std | \
     grep " fm 0 " | \
     grep "p${phase}t${replay_task}a0 ${bestScore} " | \
     grep " phs $phase " | \
     head -n 1 | \
     awk -F" t " '{print $2}' | \
     awk '{print $1}')
   
   # Get id of best team
   tm_id=$(grep "setElTmsMTA" tpg.${seed}.*.std | \
     grep " fm 0 " | \
     grep "p${phase}t${replay_task}a0 ${bestScore} " | \
     grep " phs $phase " | \
     grep " t $t_pickup " | \
     head -n 1 | \
     awk -F"id" '{print $2}' | \
     awk '{print $1}')


  #  # Get generation of best team
  #  t_pickup=$(grep setElTmsMTA tpg.${seed}.*.std | \
  #    grep " id $tm_id " | \
  #    grep " phs $phase " | \
  #    head -n 1 | \
  #    awk -F" t " '{print $2}' | \
  #    awk '{print $1}')
   
  #  # Get fitness of best team
  #  bestScore=$(grep setElTmsMTA  tpg.${seed}.*.std | \
  #    grep " id $tm_id " | \
  #    grep " phs $phase " | \
  #    awk -F "p${phase}t${replay_task}a0 " '{print $2}' | \
  #    awk '{print $1}' | \
  #    sort -n | \
  #    uniq | \
  #    tail -n 1)


   echo "Fitness:$bestScore Generation:$t_pickup Team:$tm_id"
   
   mpirun --oversubscribe -np 1 \
     $TPG/build/release/cpp/experiments/TPGExperimentMPI -a -R $tm_id -r $replay_task -C $phase \
     -p $t_pickup -s $seed -g $seed \
     1> tpg.$seed.replay.std 2> tpg.$seed.replay.err &
 
  #  echo "COMMAND: mpirun --oversubscribe -np 2 xterm -hold -e gdb -ex run --args $TPG/build/release/cpp/experiments/TPGExperimentMPI -a -R $tm -C $phase -p $t_pickup -s $seed -g $seed 1> tpg.$seed.replay.std 2> tpg.$seed.replay.err &"
  # #  replay with debugger
  #  mpirun --oversubscribe -np 1 xterm -hold -e gdb -ex run --args \
  #    $TPG/build/release/cpp/experiments/TPGExperimentMPI -R $tm -r $replay_task -C $phase \
  #    -p $t_pickup -s $seed -g $seed \
  #    1> tpg.$seed.replay.std 2> tpg.$seed.replay.err &
fi

# Debug
if [ $mode -eq 2 ]; then
   mpirun --oversubscribe -np $numMPIProc xterm -hold -e gdb -ex run \
     --args $TPG/build/release/cpp/experiments/TPGExperimentMPI -s $seed \
     1> tpg.$seed.$$.std 2> tpg.$seed.$$.err &
fi


# below this line is just sketches to be cleaned ###############################



# Check for memoy leaks
if [ $mode -eq 3 ]; then
##view profile with: google-pprof ../build/release/cpp/experiments/tpgExpBlocks_MPI ./tpg.out_27134   
##google-pprof --gv --focus=genTeams ../build/release/cpp/experiments/TPGExperimentMPI tpg.out_441771
#LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libprofiler.so CPUPROFILE=tpg.out \
#mpirun --oversubscribe -np $numMPIProc ../build/release/cpp/experiments/TPGExperimentMPI -w -F $numFitMode -D $dim -p -S -T $tMax -s $seedTPG -g $seedEnv -a $activeTasks -d $taskSwitchMod -f $fitMode 1> tpg.$seedTPG.$$.std 2> tpg.$seedTPG.$$.err &

#valgrind
mpirun --oversubscribe -np $numMPIProc valgrind --leak-check=yes --show-reachable=yes --log-file=vg.%p --suppressions=/usr/share/openmpi/openmpi-valgrind.supp \
../build/release/cpp/experiments/TPGExperimentMPI -s $seed 1> tpg.$seed.$$.std 2> tpg.$seed.$$.err &
fi

# #if [ $mode -eq 4 ]; then
# ##pickup from checkpoint file
# #t=$(grep -iRl end checkpoints/cp.*.-1.$seedTPG.0.rslt | cut -d '.' -f 2 | sort -n | tail -n 2 | head -n 1)
# ##sed  -i "1,/genTime\ t\ $t/!d" tpg.$seedTPG.std
# #pid=$(ls tpg.$seedTPG.*.std | cut -d '.' -f 4 | tail -n 1)
# #echo "pid $pid"
# #echo "Starting run seedTPG $seedTPG t $t"
# #LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libprofiler.so CPUPROFILE=tpg.out \
# #mpirun --oversubscribe -np $numMPIProc ../build/release/cpp/experiments/TPGExperimentMPI -s $seedTPG 1>> tpg.$seedTPG.$pid.std 2>> tpg.$seedTPG.$pid.err &
# ##mpirun --oversubscribe -np $numMPIProc xterm -hold -e gdb -ex run --args ../build/release/cpp/experiments/TPGExperimentMPI -C 0 -t $t -w -F $numFitMode -D $dim -p -S -T $tMax -s $seedTPG -g $seedEnv -d $taskSwitchMod -a $activeTasks 1>> tpg.$seedTPG.$pid.std 2>> tpg.$seedTPG.$pid.err &
# #fi
