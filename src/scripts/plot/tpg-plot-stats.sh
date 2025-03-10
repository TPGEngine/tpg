#!/bin/bash

#defaults
numAux=4
fitMode=0
maxt=0
modes=0
phs=0
winSize=1
wd=$(echo $PWD | rev | cut -d '/' -f 1 | rev)

while getopts a:f:g:m:p:T:w: flag
do
   case "${flag}" in
      a) numAux=${OPTARG};;
      f) fitMode=${OPTARG};;
      m) modes=${OPTARG};;
      p) phs=${OPTARG};;
      T) maxt=${OPTARG};;
      w) winSize=${OPTARG};;
   esac
done

mkdir -p plots

numTask=$(grep "n_task" logs/tpg.*.std | cut -d ' ' -f 2 | head -n 1)

c=1
#if ls *p${phs}*csv 1> /dev/null 2>&1; then rm *p${phs}.csv; fi
if ls *.csv 1> /dev/null 2>&1; then rm *.csv; fi
maxT=$maxt
if [ $maxt -eq 0 ] 
then
   maxT=$(grep "gTime t " logs/tpg*.std | awk -F" t " '{print $2}' | awk '{print $1}' | sort -n | tail -n 1 | tr -d '\n')


elif [ $maxt -eq -1 ]
then
   files=$(ls logs/tpg*.std | grep -v replay)
   mt=""
   for f in $files; do
      mt="$(grep "gTime t " $f | awk -F" t " '{print $2}' | awk '{print $1}' | sort -n | tail -n 1 | tr -d '\n') $mt"
   done
   maxT=$(echo $mt | tr ' ' '\n' | sort -n | head -n 1)
fi

files=$(ls logs/tpg*.std | sort -n -t '.' -k 3 | grep -v replay)

for f in $files
do
   echo "Processing $f ..."

   #run time stats
   for aux in `seq 0 $(echo "$numAux-1" | bc)`; do
      for task in `seq 0 $(echo "$numTask-1" | bc)`; do
         echo $(tac $f | sed '/restart/q' | tac | grep setElTmsST | grep " fm ${fitMode} " | grep " ss $task " | grep " phs $phs " | head -n $maxT | awk -F"mnOut " '{print $2}' | \
            awk -F"p${phs}t${task}a${aux} " '{print $2}' | awk '{print $1}' | tr '\n' ' ')  >> aux_${aux}_ST_${task}_p${phs}.csv;
                     if [ $fitMode -eq 0 ] && [ $numTask -gt 1 ]; then
                        echo $(tac $f | sed '/restart/q' | tac | grep setElTmsMTA | grep " fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F"mnOut " '{print $2}' | \
                           awk -F"p${phs}t${task}a${aux} " '{print $2}' | awk '{print $1}' | tr '\n' ' ')  >> aux_${aux}_MTA_${task}_p${phs}.csv;
                     fi 
                  done
               done

               echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA " | grep " phs $phs " | head -n $maxT | awk -F"minThr" '{print $2}' | \
                  awk '{print $1}' | tr '\n' ' ')  >> tpg-auxDouble_MTA-minThresh.csv

   #wall time
   echo $(tac $f | sed '/restart/q' | tac | grep "gTime " | head -n $maxT | awk -F" sec " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-sec.csv
   echo $(tac $f | sed '/restart/q' | tac | grep "gTime " | head -n $maxT | awk -F" evl " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-eval.csv
   #echo $(tac $f | sed '/restart/q' | tac | grep "gTime " | head -n $maxT | awk -F" gTms " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-genTeams.csv
   #echo $(tac $f | sed '/restart/q' | tac | grep "gTime " | head -n $maxT | awk -F" elTms " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-setEliteTeams.csv
   #echo $(tac $f | sed '/restart/q' | tac | grep "gTime " | head -n $maxT | awk -F" sTms " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-selTeams.csv
   #echo $(tac $f | sed '/restart/q' | tac | grep "gTime " | head -n $maxT | awk -F" rprt " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-rprt.csv
   #
   #   #cumulative state
   #   echo $(tac $f | sed '/restart/q' | tac | grep "cpAFts " | head -n $maxT | awk -F"cpAFts " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-cpAFts.csv
   #
   #echo $(grep "tToEvl " $f | head -n $maxT | awk -F" tToEvl " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-genTimeCurve-tToEvl.csv

  for task in `seq 0 $(echo "$numTask-1" | bc)`; do
     echo $(tac $f | sed '/restart/q' | tac | grep setElTmsST | grep " fm ${fitMode} " | grep "ss $task " | grep " phs $phs " | head -n $maxT | awk -F " nP " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-pCount-st-${task}.csv
     echo $(tac $f | sed '/restart/q' | tac | grep setElTmsST | grep "fm ${fitMode} " | grep "ss $task " | grep " phs $phs " | head -n $maxT | awk -F " nT " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-tCount-st-${task}.csv
     echo $(tac $f | sed '/restart/q' | tac | grep setElTmsST | grep "fm ${fitMode} " | grep "ss $task " | grep " phs $phs " | head -n $maxT | awk -F "age" '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-age-st-${task}.csv
     #echo $(tac $f | sed '/restart/q' | tac | grep setElTmsST | grep "fm ${fitMode} " | grep "ss $task " | grep " phs $phs " | head -n $maxT | awk -F "fit" '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-fit-st-${task}.csv
  done

   # echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " nP " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-pCount-mt.csv
   # echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " nT " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-tCount-mt.csv
   echo $(tac $f | sed '/restart/q' | tac | grep setElTmsMTA | grep " fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F "age" '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-age-mt.csv   

   echo $(tac $f | sed '/restart/q' | tac | grep setElTmsMTA | grep " fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " mnProgIns " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-meanPIns.csv
   echo $(tac $f | sed '/restart/q' | tac | grep setElTmsMTA | grep " fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " mnEProgIns " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-meanEPIns.csv

#echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " pF " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-policyFeatures.csv
#echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " pF " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-policyFeatures.csv

#echo $(tac $f | sed '/restart/q' | tac | grep "setElTmsMTA fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " lTypePr " '{print $2}' | awk '{print $1}') >> tpg-propType0.csv
#echo $(tac $f | sed '/restart/q' | tac | grep setElTmsMTA | grep " fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " mnTmSzR " '{print $2}' | awk '{print $1}') >> tpg-tmSizeRoot.csv
#echo $(tac $f | sed '/restart/q' | tac | grep setElTmsMTA | grep " fm ${fitMode} " | grep " phs $phs " | head -n $maxT | awk -F " mnTmSzS " '{print $2}' | awk '{print $1}') >> tpg-tmSizeSub.csv

# echo $(tac $f | sed '/restart/q' | tac | grep genTms | head -n $maxT | awk -F" Msz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-gt-Msize.csv
# echo $(tac $f | sed '/restart/q' | tac | grep genTms | head -n $maxT | awk -F" Lsz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-gt-Lsize.csv
# echo $(tac $f | sed '/restart/q' | tac | grep genTms | head -n $maxT | awk -F" mSz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-gt-MemSize.csv
# # echo $(tac $f | sed '/restart/q' | tac | grep genTms | head -n $maxT | awk -F" rSz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-gt-Rsize.csv
# echo $(tac $f | sed '/restart/q' | tac | grep genTms | head -n $maxT | awk -F" eLSz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-gt-eLSz.csv

echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" Msz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-st-Msize.csv
echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" Lsz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-st-Lsize.csv
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" mSz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-st-MemSize.csv
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" rSz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-st-Rsize.csv
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" mrSz " '{print $2}' | awk '{print $1}' | tr '\n' ' ') >> tpg-st-mRsize.csv
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" nOldDelPr " '{print $2}' | awk '{print $1}') >> tpg-osr.csv
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" sRTC " '{print $2}' | awk '{print $1}') >> tpg-sRTC.csv
#echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" sGsz " '{print $2}' | awk '{print $1}') >> tpg-sGsz.csv
echo $(tac $f | sed '/restart/q' | tac | grep selTms | head -n $maxT | awk -F" nDel " '{print $2}' | awk '{print $1}') >> tpg-nDel.csv

   #MODES
   if [ $modes -gt 0 ]; then
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"change" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-change.csv
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"novelty" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-novelty.csv
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"complexityRTC" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-complexityRTC.csv
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"complexityTeams" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-complexityTeams.csv
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"complexityPrograms" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-complexityPrograms.csv
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"complexityInstructions" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-complexityInstructions.csv
      echo $(tac $f | sed '/restart/q' | tac | grep TPG::MODES | awk -F"ecology" '{print $2}' | awk '{print $1}') >> ${wd}-MODES-ecology.csv
   fi
done

echo $(tac $f | sed '/restart/q' | tac | grep archive | head -n $maxT | awk -F" sz " '{print $2}' | awk '{print $1}') >> tpg-archive-size.csv


# plot #########################################################

i=0
if ls *p${phs}.pdf 1> /dev/null 2>&1; then rm *p${phs}.pdf; fi

##combined train+test
#if [ $phs -eq 2 ]; then
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves-paired.R aux_0_ST_0_p0.csv aux_0_ST_0_p2.csv "Mean Fitness" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
#i=$((i+1))
#fi

# for aux in `seq 0 $(echo "$numAux-1" | bc)`; do
#    for task in `seq 0 $(echo "$numTask-1" | bc)`; do
#       i=$((i+1))
#       Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R aux_${aux}_ST_${task}_p${phs}.csv "Aux ${aux} (single-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
#    done
# done

# fitness of best individual for each task
aux=0
for task in `seq 0 $(echo "$numTask-1" | bc)`; do
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R aux_${aux}_ST_${task}_p${phs}.csv "Fitness (single-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0

done

# teams per prediction of best individual for each task
aux=1
for task in `seq 0 $(echo "$numTask-1" | bc)`; do
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R aux_${aux}_ST_${task}_p${phs}.csv "Teams/Prediction (single-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0

done

# instructions per prediction of best individual for each task
aux=2
for task in `seq 0 $(echo "$numTask-1" | bc)`; do
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R aux_${aux}_ST_${task}_p${phs}.csv "Instructions/Prediction (single-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0

done

# # ? per prediction of best individual for each task
# aux=3
# for task in `seq 0 $(echo "$numTask-1" | bc)`; do
#    i=$((i+1))
#    Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R aux_${aux}_ST_${task}_p${phs}.csv "MSE (single-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0

# done



if [ $fitMode -eq 0 ] && [ $numTask -gt 1 ]; then
   # fitness of best individual for each task
   aux=0
   for task in `seq 0 $(echo "$numTask-1" | bc)`; do
      i=$((i+1))
      Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R aux_${aux}_MTA_${task}_p${phs}.csv "Fitness (multi-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
   done
   
   # teams per prediction of best individual for each task
   aux=1
   for task in `seq 0 $(echo "$numTask-1" | bc)`; do
      i=$((i+1))
      Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R aux_${aux}_MTA_${task}_p${phs}.csv "Teams/Prediction (multi-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
   done

   # instructions per prediction of best individual for each task
   aux=2
   for task in `seq 0 $(echo "$numTask-1" | bc)`; do
      i=$((i+1))
      Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R aux_${aux}_MTA_${task}_p${phs}.csv "Instructions/Prediction (multi-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
   done

   # # ? per prediction of best individual for each task
   # aux=3
   # for task in `seq 0 $(echo "$numTask-1" | bc)`; do
   #    i=$((i+1))
   #    Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R aux_${aux}_MTA_${task}_p${phs}.csv "MSE (multi-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
   # done
fi

# if [ $fitMode -eq 0 ] && [ $numTask -gt 1 ]; then
#    for aux in `seq 0 $(echo "$numAux-1" | bc)`; do
#       for task in `seq 0 $(echo "$numTask-1" | bc)`; do
#          i=$((i+1))
#          Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R aux_${aux}_MTA_${task}_p${phs}.csv "Aux ${aux} (multi-task $task)" "$winSize" "$(printf "%03d" $i)_${task}-${aux}" 0
#       done
#    done
# fi

 for task in `seq 0 $(echo "$numTask-1" | bc)`; do
    i=$((i+1))
    Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-tCount-st-${task}.csv "Teams per Graph (best single-task ${task})" "$winSize" "$(printf "%03d" $i)-st-${task}" 0
 done
#  
 for task in `seq 0 $(echo "$numTask-1" | bc)`; do
    i=$((i+1))
    Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-pCount-st-${task}.csv "Programs per Graph (best single-task ${task})" "$winSize" "$(printf "%03d" $i)-st-${task}" 0
 done
 
 for task in `seq 0 $(echo "$numTask-1" | bc)`; do
    i=$((i+1))
    Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-age-st-${task}.csv "Age of Graph (best single-task ${task})" "$winSize" "$(printf "%03d" $i)-st-${task}" 0
 done

# for task in `seq 0 $(echo "$numTask-1" | bc)`; do
#     i=$((i+1))
#     Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-fit-st-${task}.csv "Fitness of Graph (best single-task ${task})" "$winSize" "$(printf "%03d" $i)-st-${task}" 0
# done

# i=$((i+1))
# Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-tCount-mt.csv "Teams per Graph (best multi-task ${task})" "$winSize" "$(printf "%03d" $i)" 0
# i=$((i+1))
# Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-pCount-mt.csv "Programs per Graph (best multi-task ${task})" "$winSize" "$(printf "%03d" $i)" 0
i=$((i+1))
Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-age-mt.csv "Age of Graph (best multi-task ${task})" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-auxDouble_MTA-minThresh.csv "minThreshold" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-sRTC.csv "Population-wide Instructions Executed" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-sGsz.csv "Population-wide Teams Executed" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-nDel.csv "# teams deleted" "$winSize" "$(printf "%03d" $i)" 0
i=$((i+1))
Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-sec.csv "Seconds Total" "$winSize" "$(printf "%03d" $i)" 0 
i=$((i+1))
Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-eval.csv "Seconds Evaluation " "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-genTeams.csv "Seconds Replacement" "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-setEliteTeams.csv "Seconds Set Elite Teams" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-selTeams.csv "Seconds Selection" "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-rprt.csv "Seconds Accounting & Reporting" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-genTimeCurve-tToEvl.csv "Teams to Evaluate" "$winSize" "$(printf "%03d" $i)" 0
#  #i=$((i+1))
#  #Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-propType0.csv "Proportion of Action-Value Programs (best graph)" "$winSize" "$(printf "%03d" $i)" 1 
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-osr.csv "Offspring Survival Rate (numOldDeleted/numDeleted)" "$winSize" "$(printf "%03d" $i)" 0 
#  #i=$((i+1))
#  #Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-avp.csv "Proportion Action-Value Programs in Population" "$winSize" "$(printf "%03d" $i)" 0

if [ $modes -gt 0 ]; then
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-change.csv "MODES - Change" 5 "$(printf "%03d" $i)" 0 
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-novelty.csv "MODES - Novelty" 5 "$(printf "%03d" $i)" 0 
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-complexityRTC.csv "MODES - Complexity RTC" 5 "$(printf "%03d" $i)" 0 
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-complexityTeams.csv "MODES - Complexity Teams" 5 "$(printf "%03d" $i)" 0 
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-complexityPrograms.csv "MODES - Complexity Programs" 5 "$(printf "%03d" $i)" 0
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-complexityInstructions.csv "MODES - Complexity Instruction" 5 "$(printf "%03d" $i)" 0
   i=$((i+1))
   Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R ${wd}-MODES-ecology.csv "MODES - Ecology" 5 "$(printf "%03d" $i)" 0 
fi

i=$((i+1))
Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-meanPIns.csv "Mean Instructions per Program (best graph)" "$winSize" "$(printf "%03d" $i)" 0 
i=$((i+1))
Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-meanEPIns.csv "Mean Effective Instructions per Program (best graph)" "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-policyFeatures.csv "Features (best graph)" "$winSize" "$(printf "%03d" $i)" 0
# i=$((i+1))
# Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-gt-Msize.csv "Team Population Size" "$winSize" "$(printf "%03d" $i)" 0 
# i=$((i+1))
# Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-gt-Lsize.csv "Program Population Size" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-gt-MemSize.csv "Memory Population Size" "$winSize" "$(printf "%03d" $i)" 0
# i=$((i+1))
# Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-gt-Rsize.csv "Root Population Size" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-gt-eLSz.csv "Elite Teams " "$winSize" "$(printf "%03d" $i)" 0
i=$((i+1))
Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-st-Msize.csv "Team population size" "$winSize" "$(printf "%03d" $i)" 0
i=$((i+1))
Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-st-Lsize.csv "Program population size" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-st-Rsize.csv "st Rsize" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-st-mRsize.csv "st mRoot population size" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-st-MemSize.csv "st Memory population size" "$winSize" "$(printf "%03d" $i)" 0
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-tmSizeRoot.csv "Mean Root Team Size" "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-tmSizeSub.csv "Mean Sub Team Size" "$winSize" "$(printf "%03d" $i)" 0 
#i=$((i+1))
#Rscript  $TPG/scripts/plot/plot-tpg-trainingCurves.R tpg-archive-size.csv "Archive Size " "$winSize" "$(printf "%03d" $i)" 0



pdfunite 0*.pdf plots/${wd}_p${phs}.pdf
rm 0*.pdf
rm *csv

